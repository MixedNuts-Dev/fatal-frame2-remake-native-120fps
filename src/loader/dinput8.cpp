// FATAL FRAME II: Crimson Butterfly REMAKE — Mod ローダ（dinput8.dll プロキシ）
// Created by MixedNuts - https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps
// Licensed under the MIT License. See LICENSE for details.
//
// ゲームのルートに置くと自動的にロードされ、
// Mods\native120fps\native120fps.dll を読み込む。
// dinput8 本来の機能は System32 の実体へ転送するのでゲーム動作に影響しない。
//
// 併せて 2 つのアーカイブの読み込みを、Mods 配下の改変版へ差し替える。
//
//   archive_01.lnk : 最大FPSの選択肢テーブルを 2 択 → 3 択に拡張する
//   archive_06.lnk : 3 つ目の選択肢のラベルを "120" にする
//
// 改変版は初回起動時にゲームのファイルから生成するため、ゲームのファイルは
// 一切変更しないし、ゲームデータを同梱することもない。
//
// 選択肢テーブルは 1.0.2 まではメモリ上を走査して書き換えていた。ゲームが
// 確保する 4〜8GB の領域を舐める必要があり、ロード中に踏むと 1 パスに 13 秒
// かかるうえ、走査がゲームの作業セットを強制的に常駐させていた。過去に
// セーブロード中のクラッシュを起こしたのもこの走査である。ファイル差し替えに
// すれば走査そのものが不要になり、起動直後に確定する。

#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>

namespace {

HMODULE      g_real = nullptr;
std::wstring g_gameDir;
std::wstring g_modDir;

bool     g_enabled = true;
bool     g_log     = true;
uint32_t g_labelId = 0x00D9E01F;   // 3 つ目の選択肢に使う未使用の文字列ID

// ---- ログ ---------------------------------------------------------------

void Log(const char* fmt, ...)
{
    if (!g_log || g_modDir.empty()) return;
    const std::wstring path = g_modDir + L"native120fps.log";
    const bool isNew = (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES);
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"a") != 0 || !f) return;
    if (isNew) fwrite("\xEF\xBB\xBF", 1, 3, f);

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

// ---- ファイル入出力 -----------------------------------------------------

bool ReadWholeFile(const std::wstring& path, std::vector<uint8_t>& out)
{
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(h, &sz) || sz.QuadPart <= 0 || sz.QuadPart > (64LL << 20))
    {
        CloseHandle(h);
        return false;
    }
    out.resize(static_cast<size_t>(sz.QuadPart));

    size_t done = 0;
    while (done < out.size())
    {
        const size_t left = out.size() - done;
        const DWORD want = static_cast<DWORD>(left > (1u << 20) ? (1u << 20) : left);
        DWORD got = 0;
        if (!ReadFile(h, out.data() + done, want, &got, nullptr) || got == 0) break;
        done += got;
    }
    CloseHandle(h);
    return done == out.size();
}

bool WriteWholeFile(const std::wstring& path, const std::vector<uint8_t>& data)
{
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    size_t done = 0;
    while (done < data.size())
    {
        const size_t left = data.size() - done;
        const DWORD want = static_cast<DWORD>(left > (1u << 20) ? (1u << 20) : left);
        DWORD put = 0;
        if (!WriteFile(h, data.data() + done, want, &put, nullptr) || put == 0) break;
        done += put;
    }
    CloseHandle(h);
    return done == data.size();
}

bool FileSizeOf(const std::wstring& path, LONGLONG& size)
{
    WIN32_FILE_ATTRIBUTE_DATA fa{};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fa)) return false;
    size = (static_cast<LONGLONG>(fa.nFileSizeHigh) << 32) | fa.nFileSizeLow;
    return true;
}

uint32_t Read32(const uint8_t* p)
{
    uint32_t v = 0;
    memcpy(&v, p, sizeof(v));
    return v;
}

void Write32(uint8_t* p, uint32_t v)
{
    memcpy(p, &v, sizeof(v));
}

// ---- 改変1: archive_01.lnk の選択肢テーブルを 3 択にする ----------------
//
// OPTION_MENU_SELECT_ECB の最大FPS 行（sel r30）は
//   [選択肢数][選択肢1のID][選択肢2のID][選択肢3のID]
// と並ぶ。選択肢1が "30"、選択肢2が "60" の文字列IDで、3 つ目は空。
// 選択肢数を 3 にして 3 つ目に未使用の文字列IDを入れると、メニューの
// 選択肢が 3 つになる。
//
// 位置は決め打ちにせず、ID の隣接で探して ECB ブロック先頭の 'ecb\0' で
// 裏取りする。実測ではファイル全体でちょうど 1 箇所だけ一致する。

const uint32_t kId30 = 0x00D24344;   // "30"
const uint32_t kId60 = 0x00CB7EE4;   // "60"

// ブロック先頭からの相対位置: 0x20（ヘッダ） + 30行 * 56バイト + 8 = 0x6B8
const size_t kEcbHeaderBack = 0x6B8;

bool PatchChoiceTable(std::vector<uint8_t>& buf, char* note, size_t cap)
{
    size_t at = 0;
    int hits = 0;
    for (size_t o = kEcbHeaderBack + 4; o + 12 <= buf.size(); o += 4)
    {
        if (Read32(&buf[o]) != kId30) continue;
        if (Read32(&buf[o + 4]) != kId60) continue;

        const uint8_t* h = &buf[o - kEcbHeaderBack];
        if (!(h[0] == 'e' && h[1] == 'c' && h[2] == 'b' && h[3] == 0)) continue;

        ++hits;
        at = o;
    }
    if (hits != 1)
    {
        sprintf_s(note, cap, "%d candidate rows found (expected exactly 1)", hits);
        return false;
    }

    uint8_t* count = &buf[at - 4];
    uint8_t* slot3 = &buf[at + 8];

    if (Read32(count) == 3 && Read32(slot3) == g_labelId)
    {
        sprintf_s(note, cap, "already 3 entries");
        return true;
    }
    // 想定外の値を上書きしない。空であることを確認してから書く。
    if (Read32(count) != 2 || Read32(slot3) != 0)
    {
        sprintf_s(note, cap, "unexpected values (count=%u, slot3=0x%08X)",
                  Read32(count), Read32(slot3));
        return false;
    }

    Write32(count, 3);
    Write32(slot3, g_labelId);
    sprintf_s(note, cap, "row at 0x%llX, label ID 0x%08X",
              static_cast<unsigned long long>(at - 4), g_labelId);
    return true;
}

// ---- 改変2: archive_06.lnk のラベルを "120" にする ----------------------
//
// 各言語の MES_MENU ブロックには、16バイト固定スロットで "30" と "60"
// （最大FPSの選択肢ラベル）が隣接して並んでいる。その 3 つ先のスロットが
// 未使用枠で、そこを "120" に書き換えると 3 つ目の選択肢のラベルになる。
//
// 「16バイト境界の "30" の直後16バイトが "60"」という条件で探すと、
// 実測では言語数ぶんちょうど 10 箇所だけが一致する。

const uint8_t kLbl30[6] = { 0x33, 0x00, 0x30, 0x00, 0x00, 0x00 };    // "30"
const uint8_t kLbl60[6] = { 0x36, 0x00, 0x30, 0x00, 0x00, 0x00 };    // "60"
const uint8_t kLbl120[16] = { 0x31, 0x00, 0x32, 0x00, 0x30, 0x00 };  // "120" + 0 埋め

// 書き込んでよいのは「空」か「既知のプレースホルダ」のスロットだけ。
//
// 言語によっては、この枠が直前のメッセージの続きとして使われている
// （イタリア語の "Macchina fotografica" など）。そこへ書くと元の文が
// 切り詰められてしまうため、そういう枠には触れない。
// 公式に対応するのは日本語と英語で、この 2 言語はいずれも条件を満たす。
const uint8_t kPlaceholder[22] = {                 // "[[3537079]]"（日本語版）
    0x5B, 0x00, 0x5B, 0x00, 0x33, 0x00, 0x35, 0x00, 0x33, 0x00, 0x37, 0x00,
    0x30, 0x00, 0x37, 0x00, 0x39, 0x00, 0x5D, 0x00, 0x5D, 0x00,
};

bool SlotIsWritable(const uint8_t* slot)
{
    if (slot[0] == 0x00 && slot[1] == 0x00) return true;              // 空
    return memcmp(slot, kPlaceholder, sizeof(kPlaceholder)) == 0;     // 既知の枠
}

bool PatchLabels(std::vector<uint8_t>& buf, char* note, size_t cap)
{
    int n = 0, skipped = 0;
    if (buf.size() >= 0x100)
    {
        for (size_t o = 0; o + 16 * 4 <= buf.size(); o += 16)
        {
            if (memcmp(&buf[o], kLbl30, sizeof(kLbl30)) != 0) continue;
            if (memcmp(&buf[o + 16], kLbl60, sizeof(kLbl60)) != 0) continue;

            uint8_t* slot = &buf[o + 16 * 3];        // スロット 257
            if (!SlotIsWritable(slot))
            {
                ++skipped;                           // 他のメッセージが使っている枠
                continue;
            }
            memcpy(slot, kLbl120, sizeof(kLbl120));
            ++n;
        }
    }
    if (n == 0)
    {
        sprintf_s(note, cap, "no writable label slot found");
        return false;
    }
    sprintf_s(note, cap, "%d languages patched / %d skipped", n, skipped);
    return true;
}

// ---- 改変版の生成 -------------------------------------------------------

using PatchFn = bool (*)(std::vector<uint8_t>&, char*, size_t);

struct ArchiveJob {
    const wchar_t* suffix;   // 差し替え対象（末尾一致で判定する）
    const wchar_t* name;     // 生成物のファイル名
    const char*    tag;      // 生成物の版。改変ロジックを変えたら上げる
    PatchFn        patch;
    LONG           state;    // 0=未実行 / 1=成功 / -1=失敗
};

ArchiveJob g_jobs[] = {
    { L"archive\\archive_01.lnk", L"archive_01.lnk",
      "native120fps-archive01-v1", &PatchChoiceTable, 0 },
    { L"archive\\archive_06.lnk", L"archive_06.lnk",
      "native120fps-archive06-v2", &PatchLabels,      0 },
};

// 目印ファイルの中身と一致しなければ作り直す。サイズ比較だけでは、
// 改変ロジックやラベルIDだけ変わった場合に古い生成物を使い続けてしまう。
void TagText(const ArchiveJob& job, LONGLONG srcSize, char* out, size_t cap)
{
    sprintf_s(out, cap, "%s %lld %08X", job.tag,
              static_cast<long long>(srcSize), g_labelId);
}

bool TagMatches(const ArchiveJob& job, const std::wstring& tagPath, LONGLONG srcSize)
{
    FILE* f = nullptr;
    if (_wfopen_s(&f, tagPath.c_str(), L"rb") != 0 || !f) return false;
    char buf[128]{};
    const size_t got = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[got] = 0;
    char want[128]{};
    TagText(job, srcSize, want, sizeof(want));
    return strcmp(buf, want) == 0;
}

void WriteTag(const ArchiveJob& job, const std::wstring& tagPath, LONGLONG srcSize)
{
    FILE* f = nullptr;
    if (_wfopen_s(&f, tagPath.c_str(), L"wb") != 0 || !f) return;
    char text[128]{};
    TagText(job, srcSize, text, sizeof(text));
    fputs(text, f);
    fclose(f);
}

// 生成済みならそのまま使う。ゲーム側の更新や改変ロジックの変更があれば作り直す。
bool EnsurePatchedArchive(const ArchiveJob& job, const std::wstring& src,
                          const std::wstring& dst)
{
    LONGLONG srcSize = 0, dstSize = 0;
    if (!FileSizeOf(src, srcSize)) return false;

    const std::wstring tag = dst + L".tag";
    if (FileSizeOf(dst, dstSize) && dstSize == srcSize && TagMatches(job, tag, srcSize))
        return true;

    std::vector<uint8_t> buf;
    if (!ReadWholeFile(src, buf))
    {
        Log("[NG] Cannot read %ls", job.name);
        return false;
    }

    char note[128]{};
    if (!job.patch(buf, note, sizeof(note)))
    {
        Log("[NG] Could not patch %ls: %s. A game update may have changed the"
            " layout. Please report this log.", job.name, note);
        return false;
    }

    CreateDirectoryW(g_modDir.c_str(), nullptr);
    const std::wstring tmp = dst + L".tmp";
    if (!WriteWholeFile(tmp, buf))
    {
        Log("[NG] Failed to write the patched %ls", job.name);
        DeleteFileW(tmp.c_str());
        return false;
    }
    DeleteFileW(dst.c_str());
    if (!MoveFileW(tmp.c_str(), dst.c_str()))
    {
        Log("[NG] Failed to install the patched %ls", job.name);
        DeleteFileW(tmp.c_str());
        return false;
    }
    WriteTag(job, tag, srcSize);
    Log("[OK] Generated patched %ls (%s / %lld bytes)",
        job.name, note, static_cast<long long>(buf.size()));
    return true;
}

// ---- CreateFileW のフックによる差し替え ---------------------------------

using PFN_CreateFileW = HANDLE(WINAPI*)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
                                        DWORD, DWORD, HANDLE);

PFN_CreateFileW  g_origCreateFileW = nullptr;
PVOID*           g_iatSlot = nullptr;
CRITICAL_SECTION g_lock{};

bool EndsWithNoCase(const wchar_t* s, const wchar_t* suffix)
{
    if (!s) return false;
    const size_t ls = wcslen(s), lt = wcslen(suffix);
    if (ls < lt) return false;
    return _wcsicmp(s + (ls - lt), suffix) == 0;
}

HANDLE WINAPI MyCreateFileW(LPCWSTR name, DWORD access, DWORD share,
                            LPSECURITY_ATTRIBUTES sa, DWORD disp, DWORD flags,
                            HANDLE tmpl)
{
    if (g_enabled && (access & GENERIC_READ))
    {
        for (auto& job : g_jobs)
        {
            if (!EndsWithNoCase(name, job.suffix)) continue;

            const std::wstring dst = g_modDir + job.name;
            EnterCriticalSection(&g_lock);
            if (job.state == 0)
                job.state = EnsurePatchedArchive(job, name, dst) ? 1 : -1;
            const bool ok = (job.state == 1);
            LeaveCriticalSection(&g_lock);

            if (ok)
            {
                HANDLE h = g_origCreateFileW(dst.c_str(), access, share, sa,
                                             disp, flags, tmpl);
                if (h != INVALID_HANDLE_VALUE) return h;
                Log("[NG] Cannot open the patched %ls; falling back to the"
                    " game's own file", job.name);
            }
            break;
        }
    }
    return g_origCreateFileW(name, access, share, sa, disp, flags, tmpl);
}

PVOID* FindIatSlot(HMODULE mod, const char* dll, const char* func)
{
    auto base = reinterpret_cast<BYTE*>(mod);
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (!dos || dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;

    const auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!dir.VirtualAddress) return nullptr;

    auto imp = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + dir.VirtualAddress);
    for (; imp->Name; ++imp)
    {
        if (_stricmp(reinterpret_cast<const char*>(base + imp->Name), dll) != 0) continue;

        auto thunk = reinterpret_cast<IMAGE_THUNK_DATA64*>(base + imp->FirstThunk);
        auto orig = reinterpret_cast<IMAGE_THUNK_DATA64*>(
            base + (imp->OriginalFirstThunk ? imp->OriginalFirstThunk : imp->FirstThunk));
        for (; orig->u1.AddressOfData; ++orig, ++thunk)
        {
            if (orig->u1.Ordinal & IMAGE_ORDINAL_FLAG64) continue;
            auto ibn = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + orig->u1.AddressOfData);
            if (strcmp(reinterpret_cast<const char*>(ibn->Name), func) == 0)
                return reinterpret_cast<PVOID*>(&thunk->u1.Function);
        }
    }
    return nullptr;
}

bool ApplyHook()
{
    if (!g_iatSlot) return false;
    if (*g_iatSlot == reinterpret_cast<PVOID>(&MyCreateFileW)) return true;

    DWORD old = 0;
    if (!VirtualProtect(g_iatSlot, sizeof(PVOID), PAGE_READWRITE, &old)) return false;
    g_origCreateFileW = reinterpret_cast<PFN_CreateFileW>(*g_iatSlot);
    *g_iatSlot = reinterpret_cast<PVOID>(&MyCreateFileW);
    DWORD tmp = 0;
    VirtualProtect(g_iatSlot, sizeof(PVOID), old, &tmp);
    return true;
}

// Steam の DRM は起動時に輸入テーブルを組み直すことがあるため、
// 短時間だけ見張って、外されていたら掛け直す。
DWORD WINAPI HookGuard(LPVOID)
{
    for (int i = 0; i < 100; ++i)      // 約 10 秒
    {
        ApplyHook();
        Sleep(100);
    }
    return 0;
}

// ---- Mod 本体の読み込み -------------------------------------------------

DWORD WINAPI LoadMods(LPVOID)
{
    // Mod 側の DLL が依存物を自分のフォルダから引けるようにする
    SetDllDirectoryW(g_modDir.c_str());
    LoadLibraryW((g_modDir + L"native120fps.dll").c_str());
    SetDllDirectoryW(nullptr);
    return 0;
}

void LoadRealDinput8()
{
    if (g_real) return;
    wchar_t path[MAX_PATH]{};
    GetSystemDirectoryW(path, MAX_PATH);
    wcscat_s(path, L"\\dinput8.dll");
    g_real = LoadLibraryW(path);
}

FARPROC RealProc(const char* name)
{
    LoadRealDinput8();
    return g_real ? GetProcAddress(g_real, name) : nullptr;
}

// 差し替えはゲームがアーカイブを開く前に決まっていなければならないので、
// 設定はここで読む。Enabled=0 のときは一切差し替えない。
void LoadConfig()
{
    const std::wstring ini = g_modDir + L"native120fps.ini";
    g_enabled = GetPrivateProfileIntW(L"General", L"Enabled", 1, ini.c_str()) != 0;
    g_log     = GetPrivateProfileIntW(L"General", L"Log", 1, ini.c_str()) != 0;
    g_labelId = static_cast<uint32_t>(
        GetPrivateProfileIntW(L"Patch", L"LabelId", 0x00D9E01F, ini.c_str()));
    if (g_labelId == 0) g_labelId = 0x00D9E01F;
}

} // namespace

// ---- 転送用エクスポート -------------------------------------------------

extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD ver, REFIID riid,
                                             LPVOID* out, LPUNKNOWN outer)
{
    using Fn = HRESULT(WINAPI*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
    static Fn fn = reinterpret_cast<Fn>(RealProc("DirectInput8Create"));
    return fn ? fn(hinst, ver, riid, out, outer) : E_FAIL;
}

extern "C" HRESULT WINAPI DllCanUnloadNow()
{
    using Fn = HRESULT(WINAPI*)();
    static Fn fn = reinterpret_cast<Fn>(RealProc("DllCanUnloadNow"));
    return fn ? fn() : S_FALSE;
}

extern "C" HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    using Fn = HRESULT(WINAPI*)(REFCLSID, REFIID, LPVOID*);
    static Fn fn = reinterpret_cast<Fn>(RealProc("DllGetClassObject"));
    return fn ? fn(rclsid, riid, ppv) : E_FAIL;
}

extern "C" HRESULT WINAPI DllRegisterServer()
{
    using Fn = HRESULT(WINAPI*)();
    static Fn fn = reinterpret_cast<Fn>(RealProc("DllRegisterServer"));
    return fn ? fn() : E_FAIL;
}

extern "C" HRESULT WINAPI DllUnregisterServer()
{
    using Fn = HRESULT(WINAPI*)();
    static Fn fn = reinterpret_cast<Fn>(RealProc("DllUnregisterServer"));
    return fn ? fn() : E_FAIL;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        InitializeCriticalSection(&g_lock);

        wchar_t exe[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exe, MAX_PATH);
        g_gameDir.assign(exe);
        g_gameDir.resize(g_gameDir.find_last_of(L'\\') + 1);
        g_modDir = g_gameDir + L"Mods\\native120fps\\";

        LoadConfig();
        LoadRealDinput8();

        // 差し替えはゲームがアーカイブを開く前に仕掛ける必要があるので、
        // ここで同期的に掛ける。実際の生成は最初に開かれたときに行う。
        g_iatSlot = FindIatSlot(GetModuleHandleW(nullptr), "KERNEL32.dll", "CreateFileW");
        ApplyHook();

        if (HANDLE t = CreateThread(nullptr, 0, HookGuard, nullptr, 0, nullptr))
            CloseHandle(t);
        if (HANDLE t = CreateThread(nullptr, 0, LoadMods, nullptr, 0, nullptr))
            CloseHandle(t);
    }
    return TRUE;
}
