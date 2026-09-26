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
//
// ログは英語で書く。配布先の利用者は大半が英語話者で、日本語のログでは
// 自分の状況を判断できず、こちらへ丸投げするしかなくなる（1.0.1 で実際に
// 起きた）。コメントは日本語のままにする。

#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

namespace {

constexpr char kVersion[] = "1.0.2";

std::wstring g_modDir;
bool         g_log = true;
bool         g_diagnose = false;   // ini: Diagnose=1 のときだけ追加の診断を出す

// ログは UTF-8 で書くので、ワイド文字列は明示的に変換する
// （%ls に任せるとロケール依存でパスが化ける）
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

// 錨だけを切り出したもの（mov edx,0x3B726180）
constexpr uint8_t kAnchor[5] = { 0xBA, 0x80, 0x61, 0x72, 0x3B };

// 錨の判定は .text 全域（実測 33MB）を 1 バイトずつ見るので、
// ここが遅いと起動直後に間に合わなくなる。1 バイト比較で振り落としてから
// 残りを u32 で一度に比べる。memcmp を毎バイト呼ぶと桁違いに遅い。
constexpr uint8_t  kAnchorOp  = 0xBA;          // mov edx,imm32
constexpr uint32_t kAnchorImm = 0x3B726180u;   // OPTION_MENU_ITEM_ECB の FPS 項目 ID

inline uint32_t Read32(const uint8_t* p)
{
    uint32_t v = 0;
    memcpy(&v, p, sizeof(v));   // 未アライメントでも安全。MSVC は 1 命令に落とす
    return v;
}

inline bool IsAnchor(const uint8_t* p)
{
    return p[0] == kAnchorOp && Read32(p + 1) == kAnchorImm;
}

// 錨の直後から、この範囲内で対象のバイト列を探す。
// 本来の距離は 0x18 なので、命令が数個挿入されても届く幅を取る。
constexpr size_t kRelaxedWindow = 0x60;

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

// モジュール先頭からの相対位置。ログに出すためだけに使う
unsigned long long Rva(const void* p)
{
    return static_cast<unsigned long long>(
        static_cast<const uint8_t*>(p) - reinterpret_cast<uint8_t*>(GetModuleHandleW(nullptr)));
}

// バイト列を "AA BB CC " 形式にする
std::string Hex(const uint8_t* p, size_t n)
{
    std::string s;
    s.reserve(n * 3);
    char buf[4]{};
    for (size_t i = 0; i < n; ++i)
    {
        sprintf_s(buf, "%02X ", p[i]);
        s += buf;
    }
    return s;
}

// 小さなテキストファイルを丸ごと読む（設定ファイルの照会用）
std::string ReadTextFile(const wchar_t* path, size_t maxBytes)
{
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return std::string();
    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(h, &sz) || sz.QuadPart <= 0)
    {
        CloseHandle(h);
        return std::string();
    }
    const size_t n = (static_cast<unsigned long long>(sz.QuadPart) < maxBytes)
                     ? static_cast<size_t>(sz.QuadPart) : maxBytes;
    std::string s(n, '\0');
    DWORD got = 0;
    const BOOL ok = ReadFile(h, &s[0], static_cast<DWORD>(n), &got, nullptr);
    CloseHandle(h);
    if (!ok) return std::string();
    s.resize(got);
    return s;
}

// Steam の acf は `"key"  "value"` 形式
std::string AcfValue(const std::string& s, const char* key)
{
    const std::string k = std::string("\"") + key + "\"";
    size_t at = s.find(k);
    if (at == std::string::npos) return std::string();
    at = s.find('"', at + k.size());
    if (at == std::string::npos) return std::string();
    const size_t end = s.find('"', at + 1);
    if (end == std::string::npos) return std::string();
    return s.substr(at + 1, end - at - 1);
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
// 不具合報告のログだけで、報告者の環境がこちらの検証環境と同じかどうかを
// 判断できるようにする。ゲームのバージョン・Steam のビルド番号・言語・
// 実際のリフレッシュレート・保存された FPS 設定まで出す。

// exe のバージョンリソースから FileVersion / ProductVersion を読む
void LogExeVersion(const wchar_t* exe)
{
    DWORD dummy = 0;
    const DWORD n = GetFileVersionInfoSizeW(exe, &dummy);
    if (n == 0)
    {
        Log("Game version: (no version resource)");
        return;
    }
    std::vector<uint8_t> buf(n);
    if (!GetFileVersionInfoW(exe, 0, n, buf.data()))
    {
        Log("Game version: (unreadable)");
        return;
    }
    VS_FIXEDFILEINFO* ffi = nullptr;
    UINT len = 0;
    if (!VerQueryValueW(buf.data(), L"\\", reinterpret_cast<LPVOID*>(&ffi), &len) || !ffi)
    {
        Log("Game version: (unreadable)");
        return;
    }
    Log("Game version: file %u.%u.%u.%u / product %u.%u.%u.%u",
        HIWORD(ffi->dwFileVersionMS), LOWORD(ffi->dwFileVersionMS),
        HIWORD(ffi->dwFileVersionLS), LOWORD(ffi->dwFileVersionLS),
        HIWORD(ffi->dwProductVersionMS), LOWORD(ffi->dwProductVersionMS),
        HIWORD(ffi->dwProductVersionLS), LOWORD(ffi->dwProductVersionLS));
}

// Steam のインストール情報。exe から 2 階層上（steamapps）にある
//   steamapps\common\FatalFrameII\FatalFrameII.exe
//   steamapps\appmanifest_3920610.acf
// buildid はゲームのビルドを一意に示すので、バージョン違いの判定に一番効く。
void LogSteamManifest(const wchar_t* exe)
{
    std::wstring dir(exe);
    for (int up = 0; up < 3; ++up)   // exe 名 / FatalFrameII / common
    {
        const size_t slash = dir.find_last_of(L'\\');
        if (slash == std::wstring::npos) return;
        dir.resize(slash);
    }
    const std::wstring acf = dir + L"\\appmanifest_3920610.acf";
    const std::string s = ReadTextFile(acf.c_str(), 64 * 1024);
    if (s.empty())
    {
        Log("Steam manifest: not found (%s)", Utf8(acf.c_str()).c_str());
        return;
    }
    const std::string build = AcfValue(s, "buildid");
    const std::string lang  = AcfValue(s, "language");
    Log("Steam build: %s / install language: %s",
        build.empty() ? "?" : build.c_str(), lang.empty() ? "?" : lang.c_str());
}

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

    const std::string s = ReadTextFile(p.c_str(), 256 * 1024);
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

// 実際のリフレッシュレート。120Hz 以上になっていなければ 120FPS は出ない。
void LogDisplay()
{
    DEVMODEW dm{};
    dm.dmSize = sizeof(dm);
    if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm))
        Log("Display: %ux%u @ %u Hz (%u bpp)",
            dm.dmPelsWidth, dm.dmPelsHeight, dm.dmDisplayFrequency, dm.dmBitsPerPel);
    else
        Log("Display: unknown");

    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);
    if (EnumDisplayDevicesW(nullptr, 0, &dd, 0))
        Log("Adapter: %s", Utf8(dd.DeviceString).c_str());
}

// RTL_OSVERSIONINFOW と同じ並び。winternl.h を持ち込まずに使う
struct OsVerInfo {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR szCSDVersion[128];
};

void LogSystem()
{
    // GetVersionEx は互換シムで嘘をつくので ntdll を直接呼ぶ
    OsVerInfo vi{};
    vi.dwOSVersionInfoSize = sizeof(vi);
    using Fn = LONG(WINAPI*)(OsVerInfo*);
    if (HMODULE nt = GetModuleHandleW(L"ntdll.dll"))
    {
        if (auto fn = reinterpret_cast<Fn>(
                reinterpret_cast<void*>(GetProcAddress(nt, "RtlGetVersion"))))
        {
            if (fn(&vi) == 0)
                Log("OS: Windows %u.%u build %u",
                    vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber);
        }
    }

    wchar_t loc[LOCALE_NAME_MAX_LENGTH]{};
    if (GetUserDefaultLocaleName(loc, LOCALE_NAME_MAX_LENGTH) > 0)
        Log("Locale: %s / UI language: 0x%04X",
            Utf8(loc).c_str(), GetUserDefaultUILanguage());
}

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
        Log("exe size: %llu bytes / modified: %04d-%02d-%02d %02d:%02d",
            bytes, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    }

    LogExeVersion(exe);
    LogSteamManifest(exe);

    uint8_t* base = nullptr;
    size_t   size = 0;
    if (GetTextSection(base, size))
        Log("module: 0x%p / .text: 0x%p (%llu bytes)",
            GetModuleHandleW(nullptr), base, static_cast<unsigned long long>(size));
    else
        Log(".text: could not be located");

    LogDisplay();
    LogSystem();
    LogGraphicsOption();
}

// ---- パッチ1: メニューハンドラの 2 択ハードコード解除 --------------------

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
    if (!GetTextSection(base, size)) return HandlerResult::NoTextSection;

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
    if (!GetTextSection(base, size))
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

    if (!WriteMem(s.target, kPatch, sizeof(kPatch))) return HandlerResult::WriteFailed;

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

// 直前の走査の規模。
//
// この走査だけ 1 パスに 10 秒以上かかることがあり、時間だけ見ていても
// 「コードが遅いのか、対象が増えたのか」を区別できなかった。
//
// 見つけた時点で打ち切るので、「入った領域数」と「候補全体の量」は
// 別物になる。両方出さないと、94ms で 4GB 走査したように読めてしまう。
size_t g_visitedRegions = 0;   // 実際に入った領域数
size_t g_candRegions    = 0;   // 候補の領域数
unsigned long long g_candBytes = 0;   // 候補の総バイト数

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

    g_visitedRegions = 0;
    g_candRegions    = cands.size();
    g_candBytes      = 0;
    for (const auto& c : cands) g_candBytes += c.second;

    for (const auto& c : cands)
    {
        {
            uint8_t* const p = c.first;
            const size_t n = c.second;
            ++g_visitedRegions;

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
                    Log("[--] Choice table already patched (0x%p)", count);
                    g_tableDone = true;
                    break;
                }
                if (*count != 2) continue;

                const uint32_t three = 3;
                if (WriteMem(count, &three, 4) && WriteMem(slot3, &g_labelId, 4))
                {
                    Log("[OK] Choice table extended to 3 entries (0x%p, label ID 0x%08X)",
                        count, g_labelId);
                    g_tableDone = true;
                }
                else
                {
                    Log("[NG] Failed to write the choice table");
                }
                break;
            }
        }
        if (g_tableDone) break;
    }
    return g_tableDone;
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
        Log("[--] Enabled=0, doing nothing");
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
            // 錨の走査にかかった時間も出す。ここが遅いと起動直後の
            // オプション画面に間に合わなくなるので、実測値を残しておく。
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
        }
        if (!tableDone)
        {
            const uint64_t t0 = GetTickCount64();
            tableDone = PatchChoiceTable();
            Log("[..] Scan %d: %llu ms, visited %llu of %llu regions"
                " (%llu MB of candidates) (%s)", tries,
                static_cast<unsigned long long>(GetTickCount64() - t0),
                static_cast<unsigned long long>(g_visitedRegions),
                static_cast<unsigned long long>(g_candRegions),
                g_candBytes >> 20,
                tableDone ? "found" : "not found");
        }
        if (codeDone && tableDone) break;
        Sleep(1000);
    }

    if (codeDone && tableDone)
    {
        Log("=== Done (attempt %d). The FPS option now has three entries ===", tries);
    }
    else
    {
        Log("=== Gave up (%d attempts / %llu ms, code: %s, table: %s) ===",
            tries, static_cast<unsigned long long>(GetTickCount64() - startMs),
            codeDone ? "OK" : HandlerResultName(last), tableDone ? "OK" : "failed");

        // 2 つのパッチは独立しているので、片方だけ失敗した状態を明示する。
        // 特にコードパッチだけ失敗した場合は「120 を選べるのに 30FPS になる」
        // という紛らわしい症状になるため、ログに書いておく。
        if (!codeDone && tableDone)
        {
            Log("[!!] The code patch did NOT apply, only the menu entry was added."
                " Selecting the third entry will give you 30 FPS. This is the"
                " cause if 120 appears in the menu but will not stay selected.");
            DumpAnchors();
        }
        else if (!codeDone)
        {
            Log("[!!] The code patch did NOT apply.");
            DumpAnchors();
        }
        if (!tableDone)
            Log("[!!] The choice table was not found, so the option still has"
                " only two entries.");
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
