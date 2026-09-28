// FATAL FRAME II: Crimson Butterfly REMAKE — Native 120FPS Option
// Created by MixedNuts - https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps
// Licensed under the MIT License. See LICENSE for details.
//
// ゲーム本体は Steam DRM により .text が暗号化されているため、ファイルへの
// 静的パッチはできない。復号後のメモリに対して実行時にパッチを当てる。
//
// ここで行うのは 1 つだけ:
//   メニューの FPS 項目ハンドラが選択インデックス 0/1 しか受け付けない
//   ハードコードを解除する（8 バイト）
//
// 選択肢を 3 つに増やす処理と、3 つ目のラベルを "120" にする処理は、
// ローダ側が archive_01.lnk / archive_06.lnk の改変版を用意して行う。
// 1.0.2 までは選択肢テーブルをメモリ上で探していたが、ゲームが確保する
// 4〜8GB の領域を走査する必要があり、適用まで最悪 17 秒かかっていた。
//
// ゲームのファイルは一切変更しない。
//
// ログは英語で書く。配布先の利用者は大半が英語話者で、日本語のログでは
// 自分の状況を判断できず、こちらへ丸投げするしかなくなる（1.0.1 で実際に
// 起きた）。コメントは日本語のままにする。

#include <windows.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <mixednuts/code.hpp>
#include <mixednuts/env.hpp>
#include <mixednuts/file.hpp>
#include <mixednuts/ini.hpp>
#include <mixednuts/log.hpp>
#include <mixednuts/path.hpp>

namespace {

using mixednuts::Log;
using mixednuts::Utf8;
using mixednuts::code::Hex;
using mixednuts::code::Rva;

constexpr char kVersion[] = "1.1.0";

bool g_enabled  = true;
bool g_diagnose = false;   // ini: Diagnose=1 のときだけ追加の診断を出す

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
constexpr size_t kSigLen   = sizeof(kSig);
constexpr size_t kPatchOff = 0x18;

// 錨だけを切り出したもの（mov edx,0x3B726180）
constexpr uint8_t kAnchor[5] = { 0xBA, 0x80, 0x61, 0x72, 0x3B };

// 錨の判定は .text 全域（実測 33MB）を 1 バイトずつ見るので、
// ここが遅いと起動直後に間に合わなくなる。1 バイト比較で振り落としてから
// 残りを u32 で一度に比べる。memcmp を毎バイト呼ぶと桁違いに遅い。
constexpr uint8_t  kAnchorOp  = 0xBA;          // mov edx,imm32
constexpr uint32_t kAnchorImm = 0x3B726180u;   // OPTION_MENU_ITEM_ECB の FPS 項目 ID

// 錨の直後から、この範囲内で対象のバイト列を探す。
// 本来の距離は 0x18 なので、命令が数個挿入されても届く幅を取る。
constexpr size_t kRelaxedWindow = 0x60;

const uint8_t kOrig[8]  = { 0x83, 0xF8, 0x01, 0x75, 0x03, 0x0F, 0xB6, 0xD8 };
// movzx ebx,al ; nop x5  → 選択インデックスをそのまま採用する
const uint8_t kPatch[8] = { 0x0F, 0xB6, 0xD8, 0x90, 0x90, 0x90, 0x90, 0x90 };

// ---- ユーティリティ -----------------------------------------------------

inline bool IsAnchor(const uint8_t* p)
{
    return p[0] == kAnchorOp && mixednuts::Rd<uint32_t>(p + 1) == kAnchorImm;
}

bool MatchSig(const uint8_t* p)
{
    return mixednuts::code::Match(p, {kSig, kMask, kSigLen});
}

// json の `"key":"value"` または `"key":value` を素朴に取り出す。
// 照会したいのは数個のスカラ値だけなので、パーサは持ち込まない。
std::string JsonValue(const std::string& s, const char* key)
{
    const std::string k = std::string("\"") + key + "\"";
    size_t at = s.find(k);
    if (at == std::string::npos) return std::string();
    at = s.find(':', at + k.size());
    if (at == std::string::npos) return std::string();
    ++at;
    while (at < s.size() && (s[at] == ' ' || s[at] == '\t')) ++at;
    if (at >= s.size()) return std::string();
    if (s[at] == '"')
    {
        const size_t end = s.find('"', at + 1);
        if (end == std::string::npos) return std::string();
        return s.substr(at + 1, end - at - 1);
    }
    const size_t end = s.find_first_of(",}", at);
    return s.substr(at, (end == std::string::npos ? s.size() : end) - at);
}

// ---- 環境情報 -----------------------------------------------------------
//
// 共通の環境情報（env.hpp）に加えて、保存された FPS 設定まで出す。

// ゲームが保存しているグラフィック設定。
// fps がここで何になっているかは、この Mod の不具合報告で最も知りたい値。
// 読み取り専用属性が残っていると、ゲームは設定を保存できない。
void LogGraphicsOption()
{
    wchar_t local[MAX_PATH]{};
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH) == 0)
    {
        Log("graphics_option.json: LOCALAPPDATA is not set");
        return;
    }
    std::wstring p(local);
    p += L"\\KoeiTecmo\\FatalFrameII\\Savedata\\graphics_option.json";

    const DWORD attr = GetFileAttributesW(p.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        Log("graphics_option.json: not found");
        return;
    }
    const bool readOnly = (attr & FILE_ATTRIBUTE_READONLY) != 0;

    const std::string s = mixednuts::file::ReadText(p, 256 * 1024);
    if (s.empty())
    {
        Log("graphics_option.json: unreadable (read-only: %s)", readOnly ? "YES" : "no");
        return;
    }
    const std::string fps   = JsonValue(s, "fps");
    const std::string vsync = JsonValue(s, "vsync");
    const std::string mode  = JsonValue(s, "display_mode");
    Log("graphics_option.json: fps=%s vsync=%s display_mode=%s",
        fps.empty() ? "?" : fps.c_str(),
        vsync.empty() ? "?" : vsync.c_str(),
        mode.empty() ? "?" : mode.c_str());
    if (readOnly)
        Log("[!!] graphics_option.json is READ-ONLY. The game cannot save your"
            " choice. Clear the read-only attribute on that file.");
}

// 実際のリフレッシュレート（env.hpp の Display 行）が 120Hz 以上になっていなければ
// 120FPS は出ない。
void LogEnvironment()
{
    mixednuts::env::LogAll();
    LogGraphicsOption();
}

// ---- メニューハンドラの 2 択ハードコード解除 ----------------------------

// 失敗の理由を呼び出し側へ返す。
//
// 1.0.0 では未発見のときに何もログを出していなかったため、報告された
// ログから原因を切り分けられなかった。
enum class HandlerResult {
    Pending,            // 初期値（まだ試していない）
    Ok,                 // 今回パッチした
    AlreadyPatched,     // 既に当たっている
    NoTextSection,      // .text が取れない
    NoAnchor,           // 錨すら無い（復号前 / 全く別のバージョン）
    NotFound,           // 錨はあるが対象のバイト列が無い
    Ambiguous,          // 候補が複数あるので中止した
    WriteFailed,        // 書き込みに失敗した
};

const char* HandlerResultName(HandlerResult r)
{
    switch (r)
    {
    case HandlerResult::Pending:        return "not tried yet";
    case HandlerResult::Ok:             return "OK";
    case HandlerResult::AlreadyPatched: return "already patched";
    case HandlerResult::NoTextSection:  return "no .text";
    case HandlerResult::NoAnchor:       return "anchor not found";
    case HandlerResult::NotFound:       return "target not found";
    case HandlerResult::Ambiguous:      return "ambiguous";
    case HandlerResult::WriteFailed:    return "write failed";
    }
    return "?";
}

// 見つけたパッチ候補
struct Site {
    uint8_t* anchor  = nullptr;
    uint8_t* target  = nullptr;
    size_t   off     = 0;       // 錨から対象までの距離
    bool     exact   = false;   // 32 バイトのシグネチャも一致したか
    bool     patched = false;   // 既に当たっているか
};

// 錨を起点に候補を集める。
//
// 1.0.1 までは 32 バイトのシグネチャ完全一致を必須にしていたため、
// ゲームの更新やリージョン差で周辺の命令が変わるだけで当たらなくなった。
// 錨（FPS 項目 ID の即値）は極めて特徴的なので、こちらを主とし、
// その近傍で対象の 8 バイトを探す。候補が複数あるときは中止する。
HandlerResult CollectSites(std::vector<Site>& out, int& anchorCount)
{
    out.clear();
    anchorCount = 0;

    uint8_t* base = nullptr;
    size_t   size = 0;
    if (!mixednuts::code::TextSection(base, size)) return HandlerResult::NoTextSection;

    for (size_t i = 0; i + sizeof(kAnchor) <= size; ++i)
    {
        if (!IsAnchor(base + i)) continue;
        ++anchorCount;

        const size_t from = i + sizeof(kAnchor);
        size_t to = i + kRelaxedWindow;
        if (to + sizeof(kOrig) > size) to = size - sizeof(kOrig);

        for (size_t j = from; j <= to; ++j)
        {
            // 窓は 0x60 バイトしかなく、錨に当たったときだけ回るので
            // ここは素直に比べてよい。先頭バイトで振り落としておく。
            const uint8_t b = base[j];
            if (b != kOrig[0] && b != kPatch[0]) continue;
            const bool isOrig  = memcmp(base + j, kOrig,  sizeof(kOrig))  == 0;
            const bool isPatch = memcmp(base + j, kPatch, sizeof(kPatch)) == 0;
            if (!isOrig && !isPatch) continue;

            Site s;
            s.anchor  = base + i;
            s.target  = base + j;
            s.off     = j - i;
            s.patched = isPatch;
            s.exact   = (j - i == kPatchOff) && (i + kSigLen <= size) && MatchSig(base + i);
            out.push_back(s);
            break;   // 1 つの錨からは 1 つだけ拾う
        }
    }

    if (anchorCount == 0) return HandlerResult::NoAnchor;
    if (out.empty())      return HandlerResult::NotFound;
    if (out.size() > 1)   return HandlerResult::Ambiguous;
    return out[0].patched ? HandlerResult::AlreadyPatched : HandlerResult::Ok;
}

// 当たらなかったときの診断。錨の周辺を 16 進で吐く。
//
// ゲームの更新で周辺のコードが変わった場合、この出力があればログだけで
// 追随できる。錨が 0 件なら、そもそも .text が復号されていないか、
// 全く別のバージョンだと判断できる。
void DumpAnchors()
{
    uint8_t* base = nullptr;
    size_t   size = 0;
    if (!mixednuts::code::TextSection(base, size))
    {
        Log("[??] .text is unavailable, cannot diagnose");
        return;
    }

    constexpr size_t kDump = 48;
    int hits = 0;
    for (size_t i = 0; i + sizeof(kAnchor) <= size; ++i)
    {
        if (!IsAnchor(base + i)) continue;
        ++hits;
        if (hits > 3) continue;   // 件数は数えるが、出力は 3 件までにする
        const size_t n = (i + kDump <= size) ? kDump : size - i;
        Log("[??] anchor #%d at RVA 0x%llX: %s", hits, Rva(base + i),
            Hex(base + i, n).c_str());
    }

    Log("[??] anchor (mov edx,0x3B726180) hits: %d / .text %llu bytes",
        hits, static_cast<unsigned long long>(size));
    if (hits == 0)
        Log("[??] No anchor at all. Either the Steam DRM has not decrypted the"
            " code yet, or this build of the game differs from the one this mod"
            " was built against.");
    else
        Log("[??] The anchor is there but the surrounding code does not match."
            " A game update most likely changed it. Please report this log.");
}

HandlerResult PatchMenuHandler()
{
    std::vector<Site> sites;
    int anchors = 0;
    const HandlerResult r = CollectSites(sites, anchors);

    if (r == HandlerResult::Ambiguous)
    {
        Log("[NG] %llu candidate sites found; aborting instead of guessing",
            static_cast<unsigned long long>(sites.size()));
        for (const auto& s : sites)
            Log("[NG]   RVA 0x%llX (anchor 0x%llX + 0x%llX, exact=%s)",
                Rva(s.target), Rva(s.anchor),
                static_cast<unsigned long long>(s.off), s.exact ? "yes" : "no");
        return r;
    }
    if (r != HandlerResult::Ok && r != HandlerResult::AlreadyPatched) return r;

    const Site& s = sites[0];
    if (s.patched) return HandlerResult::AlreadyPatched;

    if (!mixednuts::code::Write(s.target, kPatch, sizeof(kPatch))) return HandlerResult::WriteFailed;

    if (s.exact)
    {
        Log("[OK] Menu handler unlocked (RVA 0x%llX, exact signature)", Rva(s.target));
    }
    else
    {
        // 完全一致しなかった場合は、こちらの検証環境とコードが違う。
        // 当たってはいるが、報告してもらう価値があるので目立たせる。
        Log("[OK] Menu handler unlocked (RVA 0x%llX, anchor match at +0x%llX)",
            Rva(s.target), static_cast<unsigned long long>(s.off));
        Log("[??] The surrounding code differs from the build this mod was tested"
            " against (anchor +0x%llX, expected +0x%X). The patch was applied"
            " anyway. If anything misbehaves, please report this log.",
            static_cast<unsigned long long>(s.off), static_cast<unsigned>(kPatchOff));
        Log("[??] bytes at anchor: %s", Hex(s.anchor, 48).c_str());
    }
    return HandlerResult::Ok;
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
        Log("[??] Could not read the frame rate index (RVA 0x%llX)",
            static_cast<unsigned long long>(g_fpsIndexRva));
        return;
    }
    Log("[??] Diagnose=1: watching the frame rate index (RVA 0x%llX, now %u"
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
        Log("[??] Frame rate index changed: %u -> %u", cur, v);
        cur = v;
    }
    Log("[??] Stopped watching the frame rate index (last value %u)", cur);
}

// ---- 設定読み込み -------------------------------------------------------

void LoadConfig(HMODULE self)
{
    namespace ini = mixednuts::ini;
    const std::wstring dir = mixednuts::ModuleDir(self);
    const std::wstring file = dir + L"native120fps.ini";
    mixednuts::log::Open(dir, L"native120fps.log", ini::Bool(file, L"General", L"Log", true));
    g_diagnose = ini::Bool(file, L"General", L"Diagnose", false);
    g_enabled  = ini::Bool(file, L"General", L"Enabled", true);
    g_fpsIndexRva = static_cast<uint32_t>(ini::Int(file, L"Patch", L"FpsIndexRva", 0x2C31808));
}

DWORD WINAPI Worker(LPVOID param)
{
    LoadConfig(static_cast<HMODULE>(param));
    if (!g_enabled)
    {
        Log("[--] Enabled=0, doing nothing");
        return 0;
    }

    Log("=== Native 120FPS Option %s / Created by MixedNuts ===", kVersion);
    LogEnvironment();

    // Steam DRM が .text を復号し終えるのを待つ。
    // 走査は .text だけ（実測 33MB / 15ms 程度）なので軽いが、
    // 念のため回数と実時間の両方で必ず打ち切る。
    constexpr int      kMaxTries   = 30;
    constexpr uint64_t kDeadlineMs = 60 * 1000;
    const uint64_t     startMs     = GetTickCount64();

    bool codeDone = false;
    HandlerResult last = HandlerResult::Pending;
    int tries = 0;
    while (!codeDone && tries < kMaxTries &&
           GetTickCount64() - startMs < kDeadlineMs)
    {
        ++tries;
        const uint64_t t0 = GetTickCount64();
        const HandlerResult r = PatchMenuHandler();
        const unsigned long long ms = GetTickCount64() - t0;

        // 同じ理由を毎回書くとログが埋まるので、初回と、変わったとき、
        // 成功したときだけ記録する
        if (tries == 1 || r != last || r == HandlerResult::Ok)
            Log("[..] Code scan %d: %llu ms (%s)", tries, ms, HandlerResultName(r));

        if (r == HandlerResult::Ok || r == HandlerResult::AlreadyPatched)
            codeDone = true;
        last = r;
        if (!codeDone) Sleep(500);
    }

    if (codeDone)
    {
        Log("=== Done (attempt %d). The FPS option now has three entries ===", tries);
    }
    else
    {
        Log("=== Gave up (%d attempts / %llu ms, code: %s) ===",
            tries, static_cast<unsigned long long>(GetTickCount64() - startMs),
            HandlerResultName(last));
        Log("[!!] The code patch did NOT apply. The third entry will still appear"
            " in the menu, but selecting it will give you 30 FPS.");
        DumpAnchors();
    }

    if (g_diagnose)
    {
        WatchFpsIndex();
    }
    else
    {
        Log("Scanning finished. The mod no longer touches the game.");
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
