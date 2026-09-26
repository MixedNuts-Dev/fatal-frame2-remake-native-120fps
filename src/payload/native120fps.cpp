// FATAL FRAME II: Crimson Butterfly REMAKE — Native 120FPS Option
// Created by MixedNuts - https://github.com/MixedNuts-Dev/fatal-frame2-remake-mods
// Licensed under the MIT License. See LICENSE for details.
//
// ゲーム本体は Steam DRM により .text が暗号化されているため、ファイルへの
// 静的パッチはできない。復号後のメモリに対して実行時にパッチを当てる。
//
// 行うことは 2 つ:
//   1) メニューの FPS 項目ハンドラが選択インデックス 0/1 しか受け付けない
//      ハードコードを解除する（8 バイト）
//   2) OPTION_MENU_SELECT_ECB の FPS 行を 2 択 → 3 択に拡張する
//
// ゲームのファイルは一切変更しない。

#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

namespace {

constexpr char kVersion[] = "1.0.1";

std::wstring g_modDir;
bool         g_log = true;
bool         g_diagnose = false;   // ini: Diagnose=1 のときだけ追加の診断を出す

// ログは UTF-8 で書くので、ワイド文字列は明示的に変換する
// （%ls に任せるとロケール依存で日本語のパスが化ける）
std::string Utf8(const wchar_t* w)
{
    if (!w || !*w) return std::string();
    const int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 1) return std::string();
    std::string s(static_cast<size_t>(n - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, -1, &s[0], n, nullptr, nullptr);
    return s;
}

void Log(const char* fmt, ...)
{
    if (!g_log) return;
    wchar_t path[MAX_PATH]{};
    swprintf_s(path, L"%snative120fps.log", g_modDir.c_str());

    const bool isNew = (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES);
    FILE* f = nullptr;
    if (_wfopen_s(&f, path, L"a") != 0 || !f) return;
    if (isNew) fwrite("\xEF\xBB\xBF", 1, 3, f);   // UTF-8 BOM（メモ帳で文字化けしないように）

    SYSTEMTIME st{};
    GetLocalTime(&st);
    fprintf(f, "[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}

// ---- シグネチャ ---------------------------------------------------------
//
// mov edx,0x3B726180      <- OPTION_MENU_ITEM_ECB の FPS 項目 ID。極めて特徴的
// mov rcx,rsi
// call ...
// test al,al
// je   ...
// xor  bl,bl
// call ...
// cmp  eax,1              <- ここから 8 バイトを書き換える (+0x18)
// jne  +3
// movzx ebx,al
constexpr uint8_t kSig[] = {
    0xBA, 0x80, 0x61, 0x72, 0x3B, 0x48, 0x8B, 0xCE,
    0xE8, 0x00, 0x00, 0x00, 0x00, 0x84, 0xC0, 0x74,
    0x00, 0x32, 0xDB, 0xE8, 0x00, 0x00, 0x00, 0x00,
    0x83, 0xF8, 0x01, 0x75, 0x03, 0x0F, 0xB6, 0xD8,
};
// 0 = ワイルドカード
constexpr uint8_t kMask[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 1, 1, 1,
    0, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
};
constexpr size_t kSigLen    = sizeof(kSig);
constexpr size_t kPatchOff  = 0x18;
const uint8_t kOrig[8]  = { 0x83, 0xF8, 0x01, 0x75, 0x03, 0x0F, 0xB6, 0xD8 };
// movzx ebx,al ; nop x5  → 選択インデックスをそのまま採用する
const uint8_t kPatch[8] = { 0x0F, 0xB6, 0xD8, 0x90, 0x90, 0x90, 0x90, 0x90 };

// OPTION_MENU_SELECT_ECB の FPS 行（sel r30）: 選択肢1="30" の ID に続いて "60" の ID
constexpr uint32_t kId30 = 0x00D24344;
constexpr uint32_t kId60 = 0x00CB7EE4;

// 3つ目の選択肢に使うラベル ID（ini で変更可）
//
// 0x00D9E01F は MES_MENU の通し番号 257 に対応する。この ID を参照している
// OPTION_MENU_SELECT_ECB の行（sel r36）はどの項目からも参照されておらず、
// ゲーム中のどこにも表示されないため、流用しても副作用がない。
// 実際に "120" と表示させるための文字列の差し替えは、ローダ側が
// archive_06.lnk の改変版を用意して行う。
uint32_t g_labelId = 0x00D9E01F;

// ---- ユーティリティ -----------------------------------------------------

bool WriteMem(void* addr, const void* data, size_t len)
{
    DWORD old = 0;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &old)) return false;
    memcpy(addr, data, len);
    DWORD tmp = 0;
    VirtualProtect(addr, len, old, &tmp);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return true;
}

bool MatchSig(const uint8_t* p)
{
    for (size_t i = 0; i < kSigLen; ++i)
        if (kMask[i] && p[i] != kSig[i]) return false;
    return true;
}

// 実行ファイルの .text 範囲を得る
bool GetTextSection(uint8_t*& base, size_t& size)
{
    auto mod = reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr));
    if (!mod) return false;
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(mod);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(mod + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return false;

    auto sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec)
    {
        if (memcmp(sec->Name, ".text", 5) == 0)
        {
            base = mod + sec->VirtualAddress;
            size = sec->Misc.VirtualSize;
            return true;
        }
    }
    return false;
}

// ---- パッチ1: メニューハンドラの 2 択ハードコード解除 --------------------

// モジュール先頭からの相対位置。ログに出すためだけに使う
unsigned long long Rva(const void* p)
{
    return static_cast<unsigned long long>(
        static_cast<const uint8_t*>(p) - reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr)));
}

// 失敗の理由を呼び出し側へ返す。
//
// このパッチだけが失敗してテーブル拡張が成功すると、「メニューに 120 は
// 出るが選ぶと 30FPS になる」という紛らわしい状態になる。1.0.0 では
// 未発見のときに何もログを出していなかったため、報告されたログから
// 原因を切り分けられなかった。
enum class HandlerResult {
    Pending,            // 初期値（まだ試していない）
    Ok,                 // 今回パッチした
    AlreadyPatched,     // 既に当たっている
    NoTextSection,      // .text が取れない
    NotFound,           // シグネチャが見つからない（復号前 / バージョン違い）
    Ambiguous,          // 複数一致したので中止した
    UnexpectedBytes,    // 当該位置のバイト列が想定外（他の Mod と衝突など）
    WriteFailed,        // 書き込みに失敗した
};

const char* HandlerResultName(HandlerResult r)
{
    switch (r)
    {
    case HandlerResult::Pending:         return "未試行";
    case HandlerResult::Ok:              return "OK";
    case HandlerResult::AlreadyPatched:  return "適用済み";
    case HandlerResult::NoTextSection:   return ".text不明";
    case HandlerResult::NotFound:        return "未発見";
    case HandlerResult::Ambiguous:       return "複数一致";
    case HandlerResult::UnexpectedBytes: return "バイト列不一致";
    case HandlerResult::WriteFailed:     return "書込失敗";
    }
    return "?";
}

// 32 バイトのシグネチャが当たらなかったときに、錨の 5 バイト
// (mov edx,0x3B726180) だけで探し直して周辺を 16 進で吐く。
//
// ゲームの更新で周辺のコードが変わった場合、この出力があればログだけで
// 新しいシグネチャを作れる。錨が 0 件なら、そもそも .text が復号されて
// いないか、別バージョンだと判断できる。
void DumpSignatureNeighborhood()
{
    uint8_t* base = nullptr;
    size_t   size = 0;
    if (!GetTextSection(base, size))
    {
        Log("[??] .text が取得できないため診断できません");
        return;
    }

    constexpr uint8_t kAnchor[5] = { 0xBA, 0x80, 0x61, 0x72, 0x3B };
    constexpr size_t  kDump      = 48;
    int hits = 0;
    for (size_t i = 0; i + sizeof(kAnchor) <= size; ++i)
    {
        if (memcmp(base + i, kAnchor, sizeof(kAnchor)) != 0) continue;
        ++hits;
        if (hits > 3) continue;   // 件数は数えるが、出力は 3 件までにする

        const size_t n = (i + kDump <= size) ? kDump : size - i;
        char hex[kDump * 3 + 1]{};
        for (size_t k = 0; k < n; ++k)
            sprintf_s(hex + k * 3, 4, "%02X ", base[i + k]);
        Log("[??] 錨 %d 件目 RVA 0x%llX: %s", hits, Rva(base + i), hex);
    }

    Log("[??] 錨 (mov edx,0x3B726180) の一致数: %d 件 / .text %llu bytes",
        hits, static_cast<unsigned long long>(size));
    if (hits == 0)
        Log("[??] 錨が 1 件も無い。Steam DRM の復号前か、ゲームのバージョンが"
            "異なる可能性があります");
    else
        Log("[??] 錨はあるが前後のコードが一致しない。ゲームの更新で"
            "シグネチャが変わった可能性があります");
}

HandlerResult PatchMenuHandler()
{
    uint8_t* base = nullptr;
    size_t   size = 0;
    if (!GetTextSection(base, size)) return HandlerResult::NoTextSection;

    uint8_t* found = nullptr;
    for (size_t i = 0; i + kSigLen <= size; ++i)
    {
        if (MatchSig(base + i))
        {
            if (found) return HandlerResult::Ambiguous;
            found = base + i;
        }
    }
    if (!found) return HandlerResult::NotFound;

    uint8_t* target = found + kPatchOff;
    if (memcmp(target, kPatch, sizeof(kPatch)) == 0) return HandlerResult::AlreadyPatched;
    if (memcmp(target, kOrig, sizeof(kOrig)) != 0)
    {
        char hex[sizeof(kOrig) * 3 + 1]{};
        for (size_t k = 0; k < sizeof(kOrig); ++k)
            sprintf_s(hex + k * 3, 4, "%02X ", target[k]);
        Log("[NG] 想定外のバイト列 (RVA 0x%llX): %s", Rva(target), hex);
        return HandlerResult::UnexpectedBytes;
    }
    if (!WriteMem(target, kPatch, sizeof(kPatch))) return HandlerResult::WriteFailed;

    Log("[OK] メニューハンドラを解除 (RVA 0x%llX)", Rva(target));
    return HandlerResult::Ok;
}

// ---- パッチ2: 選択肢を 3 つに拡張 ---------------------------------------

// ECB ブロック先頭からの相対位置:
//   0x20（ヘッダ） + 30行 * 56バイト + 8（col2 = 選択肢1）= 0x6B8
constexpr size_t kEcbHeaderBack = 0x6B8;

// 領域の読み取りは他スレッドの解放と競合しうるので SEH で保護する。
// 例外が起きたらその領域を諦めて次へ進む。
bool ScanRegion(uint8_t* p, size_t n, size_t start, const uint8_t*, size_t& foundOff)
{
    __try
    {
        // 4 バイト境界に並ぶので u32 として比較する。
        // まず 1 つ目の ID だけ見て、当たったときだけ 2 つ目を確認する。
        auto q = reinterpret_cast<const uint32_t*>(p);
        const size_t cnt = n / 4;
        for (size_t i = start / 4; i + 1 < cnt; ++i)
        {
            if (q[i] == kId30 && q[i + 1] == kId60)
            {
                foundOff = i * 4;
                return true;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

bool g_tableDone = false;   // 選択肢テーブルの拡張済みフラグ


// 偽ヒット（スタック上の一時コピーなど）を排除するため、
// ブロック先頭が 'ecb\0' であることを確認する。
bool LooksLikeEcb(uint8_t* slot1)
{
    __try
    {
        uint8_t* h = slot1 - kEcbHeaderBack;
        return h[0] == 'e' && h[1] == 'c' && h[2] == 'b' && h[3] == 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

// 候補領域をサイズの大きい順に見る。
//
// アーカイブは数GB規模の巨大な確保領域の中にあり、小さな領域が多数あるため、
// 素直に番地順で舐めると目的の領域に辿り着くまでが遅い。大きい方から見れば
// 通常は最初の領域で見つかる。
//
// 「各領域の先頭だけを見る」という絞り方も試したが、候補領域が多いため
// 1周に数秒かかるうえ、アーカイブが先頭付近にあるとは限らず逆効果だった。
bool PatchChoiceTable()
{
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    auto addr = reinterpret_cast<uint8_t*>(si.lpMinimumApplicationAddress);
    auto maxA = reinterpret_cast<uint8_t*>(si.lpMaximumApplicationAddress);

    uint8_t pattern[8];
    memcpy(pattern, &kId30, 4);
    memcpy(pattern + 4, &kId60, 4);

    // まず候補領域を集める
    std::vector<std::pair<uint8_t*, size_t>> cands;
    MEMORY_BASIC_INFORMATION mbi{};
    while (addr < maxA && VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi))
    {
        const DWORD prot = mbi.Protect & 0xFF;
        const bool readable = mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_GUARD) &&
                              (prot == PAGE_READONLY || prot == PAGE_READWRITE ||
                               prot == PAGE_WRITECOPY);
        // 走査対象を絞る。
        // アーカイブは MEM_PRIVATE / READWRITE の巨大な確保領域の中にある
        // （実測で約 4.1GB の単一領域だった）。上限を小さく取ると目的の領域ごと
        // 除外してしまうので、上限は十分に大きく取る。
        // イメージ領域とマップド領域は対象外のままにして、触る範囲は抑える。
        const bool candidate = readable && mbi.Type == MEM_PRIVATE &&
                               mbi.RegionSize >= 0x10000 &&
                               mbi.RegionSize <= (8ull << 30);
        if (candidate)
            cands.emplace_back(static_cast<uint8_t*>(mbi.BaseAddress), mbi.RegionSize);

        auto next0 = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        if (next0 <= addr) break;
        addr = next0;
    }

    // 大きい領域から先に見る
    std::sort(cands.begin(), cands.end(),
              [](const std::pair<uint8_t*, size_t>& a,
                 const std::pair<uint8_t*, size_t>& b) { return a.second > b.second; });

    for (const auto& c : cands)
    {
        {
            uint8_t* const p = c.first;
            const size_t n = c.second;

            size_t at = 0;
            while (ScanRegion(p, n, at, pattern, at))
            {
                uint8_t* slot1 = p + at;
                at += 4;
                if (!LooksLikeEcb(slot1)) continue;   // ECB でなければ無視

                auto count = reinterpret_cast<uint32_t*>(slot1 - 4);
                auto slot3 = reinterpret_cast<uint32_t*>(slot1 + 8);
                if (*count == 3 && *slot3 == g_labelId)
                {
                    Log("[--] 選択肢テーブルは適用済み (0x%p)", count);
                    g_tableDone = true;
                    break;
                }
                if (*count != 2) continue;

                const uint32_t three = 3;
                if (WriteMem(count, &three, 4) && WriteMem(slot3, &g_labelId, 4))
                {
                    Log("[OK] 選択肢を 3 つに拡張 (0x%p, ラベルID 0x%08X)", count, g_labelId);
                    g_tableDone = true;
                }
                else
                {
                    Log("[NG] 選択肢テーブルの書き込みに失敗");
                }
                break;
            }
        }
        if (g_tableDone) break;
    }
    return g_tableDone;
}

// ---- 環境情報 -----------------------------------------------------------
//
// 不具合報告のログから、ゲームのバージョン違いを切り分けるために出す。
// exe のサイズと更新日時があれば、こちらの検証環境と同じビルドかどうかが
// 判断できる。
void LogEnvironment()
{
    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    Log("exe: %s", Utf8(exe).c_str());

    WIN32_FILE_ATTRIBUTE_DATA fad{};
    if (GetFileAttributesExW(exe, GetFileExInfoStandard, &fad))
    {
        const unsigned long long bytes =
            (static_cast<unsigned long long>(fad.nFileSizeHigh) << 32) | fad.nFileSizeLow;
        FILETIME   lt{};
        SYSTEMTIME st{};
        FileTimeToLocalFileTime(&fad.ftLastWriteTime, &lt);
        FileTimeToSystemTime(&lt, &st);
        Log("exe サイズ: %llu bytes / 更新日時: %04d-%02d-%02d %02d:%02d",
            bytes, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    }

    uint8_t* base = nullptr;
    size_t   size = 0;
    if (GetTextSection(base, size))
        Log("モジュール: 0x%p / .text: 0x%p (%llu bytes)",
            GetModuleHandleW(nullptr), base, static_cast<unsigned long long>(size));
    else
        Log(".text: 取得できませんでした");
}

// ---- 任意: FPS インデックスの監視 ---------------------------------------
//
// 「120 を選んでも 30FPS になる」という報告の切り分け用。
// ini の Diagnose=1 のときだけ動く。
//
// 読むのは .data 上の 1 バイト（0=30 / 1=60 / 2=120）だけで、書き込みは
// 一切しない。この領域は実行ファイルのイメージ内にあり解放されないため、
// 読んでも安全だが、念のため SEH で保護する。
uintptr_t g_fpsIndexRva = 0x2C31808;

bool ReadByte(const uint8_t* p, uint8_t& out)
{
    __try { out = *p; return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

void WatchFpsIndex()
{
    auto mod = reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr));
    if (!mod || g_fpsIndexRva == 0) return;
    const uint8_t* p = mod + g_fpsIndexRva;

    uint8_t cur = 0;
    if (!ReadByte(p, cur))
    {
        Log("[??] FPS インデックス (RVA 0x%llX) を読めませんでした",
            static_cast<unsigned long long>(g_fpsIndexRva));
        return;
    }
    Log("[??] Diagnose=1: FPS インデックスの監視を開始 (RVA 0x%llX, 現在値 %u"
        " / 0=30 1=60 2=120)", static_cast<unsigned long long>(g_fpsIndexRva), cur);

    // 15 分まで、5 秒おきに見る。値が変わったときだけ記録する
    constexpr int kIntervalMs = 5000;
    constexpr int kMaxSec     = 15 * 60;
    for (int t = 0; t < kMaxSec; t += kIntervalMs / 1000)
    {
        Sleep(kIntervalMs);
        uint8_t v = 0;
        if (!ReadByte(p, v)) break;
        if (v == cur) continue;
        Log("[??] FPS インデックスが %u -> %u に変化", cur, v);
        cur = v;
    }
    Log("[??] FPS インデックスの監視を終了しました (最終値 %u)", cur);
}

// ---- 設定読み込み -------------------------------------------------------

void LoadConfig(HMODULE self)
{
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(self, path, MAX_PATH);
    std::wstring dir(path);
    dir.resize(dir.find_last_of(L'\\') + 1);
    g_modDir = dir;

    const std::wstring ini = dir + L"native120fps.ini";
    g_log      = GetPrivateProfileIntW(L"General", L"Log", 1, ini.c_str()) != 0;
    g_diagnose = GetPrivateProfileIntW(L"General", L"Diagnose", 0, ini.c_str()) != 0;
    g_labelId  = GetPrivateProfileIntW(L"Patch", L"LabelId", 0x00F92FF0, ini.c_str());
    g_fpsIndexRva = static_cast<uintptr_t>(
        GetPrivateProfileIntW(L"Patch", L"FpsIndexRva", 0x2C31808, ini.c_str()));
    if (GetPrivateProfileIntW(L"General", L"Enabled", 1, ini.c_str()) == 0)
    {
        Log("[--] Enabled=0 のため何もしません");
        g_labelId = 0;   // 無効の印
    }
}

DWORD WINAPI Worker(LPVOID param)
{
    LoadConfig(static_cast<HMODULE>(param));
    if (g_labelId == 0) return 0;

    Log("=== Native 120FPS Option %s / Created by MixedNuts ===", kVersion);
    LogEnvironment();

    // Steam DRM の復号とアーカイブのロードを待つ。
    //
    // 走査は必ず有限回で打ち切る。1 回のメモリ走査に数秒かかるため、
    // 回数だけでなく実時間でも上限を設ける。ゲームプレイ中に走査が
    // 動き続けると、解放中のメモリに触れて巻き添えでクラッシュする。
    constexpr int      kMaxTries    = 20;
    constexpr uint64_t kDeadlineMs  = 90 * 1000;
    const uint64_t     startMs      = GetTickCount64();

    bool codeDone = false, tableDone = false;
    HandlerResult last = HandlerResult::Pending;
    int tries = 0;
    while (!(codeDone && tableDone) && tries < kMaxTries &&
           GetTickCount64() - startMs < kDeadlineMs)
    {
        ++tries;
        if (!codeDone)
        {
            const HandlerResult r = PatchMenuHandler();
            if (r == HandlerResult::Ok)
            {
                codeDone = true;
            }
            else if (r == HandlerResult::AlreadyPatched)
            {
                Log("[--] メニューハンドラは適用済み");
                codeDone = true;
            }
            else if (r != last)
            {
                // 同じ理由を毎回書くとログが埋まるので、変わったときだけ記録する
                Log("[..] メニューハンドラ未適用: %s (%d 回目)", HandlerResultName(r), tries);
            }
            last = r;
        }
        if (!tableDone)
        {
            const uint64_t t0 = GetTickCount64();
            tableDone = PatchChoiceTable();
            Log("[..] %d 回目の走査: %llu ms (%s)", tries,
                static_cast<unsigned long long>(GetTickCount64() - t0),
                tableDone ? "発見" : "未発見");
        }
        if (codeDone && tableDone) break;
        Sleep(1000);
    }

    if (codeDone && tableDone)
    {
        Log("=== 完了 (%d 回目)。オプション画面の FPS が 3 択になります ===", tries);
    }
    else
    {
        Log("=== 打ち切り (%d 回 / %llu ms, コード:%s テーブル:%s) ===",
            tries, static_cast<unsigned long long>(GetTickCount64() - startMs),
            codeDone ? "OK" : HandlerResultName(last), tableDone ? "OK" : "NG");

        // 2 つのパッチは独立しているので、片方だけ失敗した状態を明示する。
        // 特にコードパッチだけ失敗した場合は「120 を選べるのに 30FPS になる」
        // という紛らわしい症状になるため、ログに書いておく。
        if (!codeDone && tableDone)
        {
            Log("[!!] コードパッチが当たっていません。この状態でメニューの 3 つ目を"
                "選ぶと 30FPS になります（選択肢だけが増えた状態）");
            DumpSignatureNeighborhood();
        }
        else if (!codeDone)
        {
            Log("[!!] コードパッチが当たっていません");
            DumpSignatureNeighborhood();
        }
        if (!tableDone)
            Log("[!!] 選択肢テーブルが見つかりませんでした。オプションは 2 択のままです");
    }

    if (g_diagnose)
    {
        WatchFpsIndex();
    }
    else
    {
        Log("走査を終了しました。以降ゲームには一切触れません。");
    }
    return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        if (HANDLE t = CreateThread(nullptr, 0, Worker, hModule, 0, nullptr))
            CloseHandle(t);
    }
    return TRUE;
}
