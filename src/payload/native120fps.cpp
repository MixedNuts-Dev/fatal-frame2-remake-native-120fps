// FATAL FRAME II: Crimson Butterfly REMAKE — Native 120FPS Option
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

namespace {

std::wstring g_modDir;
bool         g_log = true;

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
// 0x00D9E01F は MES_MENU の通し番号 257 に対応する文字列で、中身は
// 翻訳漏れのプレースホルダ "[[3537079]]"。この ID を参照している
// OPTION_MENU_SELECT_ECB の行（sel r36）はどの項目からも参照されておらず、
// ゲーム中のどこにも表示されない。そこを "120" に書き換えて流用する。
uint32_t g_labelId = 0x00D9E01F;

// 置き換え対象の文字列（UTF-16LE の "[[3537079]]" + 終端）
const uint8_t kOldLabel[] = {
    0x5B, 0x00, 0x5B, 0x00, 0x33, 0x00, 0x35, 0x00, 0x33, 0x00, 0x37, 0x00,
    0x30, 0x00, 0x37, 0x00, 0x39, 0x00, 0x5D, 0x00, 0x5D, 0x00, 0x00, 0x00,
};
// "120" + 終端。残りは 0 で埋めて元のスロット長に収める
const uint8_t kNewLabel[sizeof(kOldLabel)] = {
    0x31, 0x00, 0x32, 0x00, 0x30, 0x00, 0x00, 0x00,
};

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

bool PatchMenuHandler()
{
    uint8_t* base = nullptr;
    size_t   size = 0;
    if (!GetTextSection(base, size)) return false;

    uint8_t* found = nullptr;
    for (size_t i = 0; i + kSigLen <= size; ++i)
    {
        if (MatchSig(base + i))
        {
            if (found)
            {
                Log("[NG] シグネチャが複数一致。安全のため中止します");
                return false;
            }
            found = base + i;
        }
    }
    if (!found) return false;

    uint8_t* target = found + kPatchOff;
    if (memcmp(target, kPatch, sizeof(kPatch)) == 0)
    {
        Log("[--] メニューハンドラは適用済み");
        return true;
    }
    if (memcmp(target, kOrig, sizeof(kOrig)) != 0)
    {
        Log("[NG] 想定外のバイト列。中止します");
        return false;
    }
    if (!WriteMem(target, kPatch, sizeof(kPatch)))
    {
        Log("[NG] 書き込みに失敗しました");
        return false;
    }
    Log("[OK] メニューハンドラを解除 (RVA 0x%llX)",
        static_cast<unsigned long long>(target - reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr))));
    return true;
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

bool g_labelDone = false;   // ラベル文字列の差し替え済みフラグ
bool g_tableDone = false;   // 選択肢テーブルの拡張済みフラグ

// 任意のバイト列を SEH 保護付きで探す（ラベル文字列の差し替え用）
bool ScanBytes(uint8_t* p, size_t n, size_t start, const uint8_t* pat, size_t len,
               size_t& foundOff)
{
    __try
    {
        for (size_t i = start; i + len <= n; i += 16)
        {
            if (p[i] == pat[0] && memcmp(p + i, pat, len) == 0)
            {
                foundOff = i;
                return true;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

// 指定領域内でラベル文字列を "120" に差し替える。
// 文字列プールは 16 バイト境界に並ぶので、その刻みで探せば十分速い。
void PatchLabelInRegion(uint8_t* p, size_t n)
{
    if (g_labelDone) return;
    const uint64_t t0 = GetTickCount64();
    size_t at = 0;
    while (ScanBytes(p, n, at, kOldLabel, sizeof(kOldLabel), at))
    {
        if (WriteMem(p + at, kNewLabel, sizeof(kNewLabel)))
        {
            Log("[OK] ラベルを \"120\" に差し替え (0x%p, %llu ms)", p + at,
                static_cast<unsigned long long>(GetTickCount64() - t0));
            g_labelDone = true;
            return;
        }
        at += 16;
    }
    Log("[NG] ラベル文字列が見つかりません (%llu ms)",
        static_cast<unsigned long long>(GetTickCount64() - t0));
}

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

bool PatchChoiceTable()
{
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    auto addr = reinterpret_cast<uint8_t*>(si.lpMinimumApplicationAddress);
    auto maxA = reinterpret_cast<uint8_t*>(si.lpMaximumApplicationAddress);

    uint8_t pattern[8];
    memcpy(pattern, &kId30, 4);
    memcpy(pattern + 4, &kId60, 4);

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
        {
            auto p = static_cast<uint8_t*>(mbi.BaseAddress);
            const size_t n = mbi.RegionSize;

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
                    // メッセージプールは ECB と同じアロケーション内にあるので、
                    // この領域だけを対象にラベル文字列を差し替える。
                    // 全領域を舐めると時間がかかりすぎてパッチが間に合わない。
                    PatchLabelInRegion(p, n);
                }
                else
                {
                    Log("[NG] 選択肢テーブルの書き込みに失敗");
                }
                break;
            }
        }
        // どちらも終わったら走査を打ち切る。片方だけなら残りの領域も見る。
        if (g_tableDone) break;

        auto next = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        if (next <= addr) break;
        addr = next;
    }
    return g_tableDone;
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
    g_labelId  = GetPrivateProfileIntW(L"Patch", L"LabelId", 0x00F92FF0, ini.c_str());
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

    Log("=== Native 120FPS Option 起動 ===");

    // Steam DRM の復号とアーカイブのロードを待つ。
    //
    // 走査は必ず有限回で打ち切る。1 回のメモリ走査に数秒かかるため、
    // 回数だけでなく実時間でも上限を設ける。ゲームプレイ中に走査が
    // 動き続けると、解放中のメモリに触れて巻き添えでクラッシュする。
    constexpr int      kMaxTries    = 20;
    constexpr uint64_t kDeadlineMs  = 90 * 1000;
    const uint64_t     startMs      = GetTickCount64();

    bool codeDone = false, tableDone = false;
    int tries = 0;
    while (!(codeDone && tableDone) && tries < kMaxTries &&
           GetTickCount64() - startMs < kDeadlineMs)
    {
        ++tries;
        if (!codeDone)  codeDone  = PatchMenuHandler();
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
        Log("=== 完了 (%d 回目)。オプション画面の FPS が 3 択になります ===", tries);
    else
        Log("=== 打ち切り (%d 回 / %llu ms, コード:%s テーブル:%s) ===",
            tries, static_cast<unsigned long long>(GetTickCount64() - startMs),
            codeDone ? "OK" : "NG", tableDone ? "OK" : "NG");

    Log("走査を終了しました。以降ゲームには一切触れません。");
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
