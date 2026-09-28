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
#include <cstring>
#include <string>
#include <vector>

#include <mixednuts/bytes.hpp>
#include <mixednuts/file.hpp>
#include <mixednuts/iat.hpp>
#include <mixednuts/ini.hpp>
#include <mixednuts/log.hpp>
#include <mixednuts/path.hpp>
#include <mixednuts/proxy.hpp>

namespace {

using mixednuts::Log;
using mixednuts::Rd;
using mixednuts::Utf8;
using mixednuts::Wr;

std::wstring g_modDir;

bool     g_enabled = true;
uint32_t g_labelId = 0x00D9E01F;   // 3 つ目の選択肢に使う未使用の文字列ID

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
        if (Rd<uint32_t>(&buf[o]) != kId30) continue;
        if (Rd<uint32_t>(&buf[o + 4]) != kId60) continue;

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

    if (Rd<uint32_t>(count) == 3 && Rd<uint32_t>(slot3) == g_labelId)
    {
        sprintf_s(note, cap, "already 3 entries");
        return true;
    }
    // 想定外の値を上書きしない。空であることを確認してから書く。
    if (Rd<uint32_t>(count) != 2 || Rd<uint32_t>(slot3) != 0)
    {
        sprintf_s(note, cap, "unexpected values (count=%u, slot3=0x%08X)",
                  Rd<uint32_t>(count), Rd<uint32_t>(slot3));
        return false;
    }

    Wr<uint32_t>(count, 3);
    Wr<uint32_t>(slot3, g_labelId);
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
    namespace file = mixednuts::file;
    LONGLONG srcSize = 0, dstSize = 0;
    if (!file::Size(src, srcSize)) return false;

    const std::wstring tag = dst + L".tag";
    if (file::Size(dst, dstSize) && dstSize == srcSize && TagMatches(job, tag, srcSize))
        return true;

    std::vector<uint8_t> buf;
    if (!file::ReadAll(src, buf, 64ull << 20))
    {
        Log("[NG] Cannot read %s", Utf8(job.name).c_str());
        return false;
    }

    char note[128]{};
    if (!job.patch(buf, note, sizeof(note)))
    {
        Log("[NG] Could not patch %s: %s. A game update may have changed the"
            " layout. Please report this log.", Utf8(job.name).c_str(), note);
        return false;
    }

    CreateDirectoryW(g_modDir.c_str(), nullptr);
    if (!file::WriteAll(dst, buf))
    {
        Log("[NG] Failed to write the patched %s", Utf8(job.name).c_str());
        return false;
    }
    WriteTag(job, tag, srcSize);
    Log("[OK] Generated patched %s (%s / %lld bytes)",
        Utf8(job.name).c_str(), note, static_cast<long long>(buf.size()));
    return true;
}

// ---- CreateFileW のフックによる差し替え ---------------------------------

using PFN_CreateFileW = HANDLE(WINAPI*)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
                                        DWORD, DWORD, HANDLE);

PFN_CreateFileW  g_origCreateFileW = nullptr;
PVOID*           g_iatSlot = nullptr;
CRITICAL_SECTION g_lock{};

HANDLE WINAPI MyCreateFileW(LPCWSTR name, DWORD access, DWORD share,
                            LPSECURITY_ATTRIBUTES sa, DWORD disp, DWORD flags,
                            HANDLE tmpl)
{
    if (g_enabled && (access & GENERIC_READ))
    {
        for (auto& job : g_jobs)
        {
            if (!mixednuts::EndsWithPath(name, job.suffix)) continue;

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
                Log("[NG] Cannot open the patched %s; falling back to the"
                    " game's own file", Utf8(job.name).c_str());
            }
            break;
        }
    }
    return g_origCreateFileW(name, access, share, sa, disp, flags, tmpl);
}

bool ApplyHook()
{
    if (!g_iatSlot) return false;
    if (*g_iatSlot == reinterpret_cast<PVOID>(&MyCreateFileW)) return true;
    return mixednuts::iat::Swap(g_iatSlot, reinterpret_cast<PVOID>(&MyCreateFileW),
                                g_origCreateFileW);
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

// 差し替えはゲームがアーカイブを開く前に決まっていなければならないので、
// 設定はここで読む。Enabled=0 のときは一切差し替えない。
void LoadConfig()
{
    namespace ini = mixednuts::ini;
    const std::wstring file = g_modDir + L"native120fps.ini";
    g_enabled = ini::Bool(file, L"General", L"Enabled", true);
    mixednuts::log::Open(g_modDir, L"native120fps.log",
                         ini::Bool(file, L"General", L"Log", true));
    g_labelId = static_cast<uint32_t>(ini::Int(file, L"Patch", L"LabelId", 0x00D9E01F));
    if (g_labelId == 0) g_labelId = 0x00D9E01F;
}

} // namespace

// ---- 転送用エクスポート -------------------------------------------------
//
// dinput8.dll の関数はどれも整数・ポインタの引数を 8 個以下しか取らず、
// 浮動小数点の引数も無いので、8 個そのまま受け渡す転送で済ませる（proxy.hpp）。

MIXEDNUTS_FORWARD(Proxy_DirectInput8Create, "DirectInput8Create", E_FAIL)
MIXEDNUTS_FORWARD(Proxy_DllCanUnloadNow, "DllCanUnloadNow", S_FALSE)
MIXEDNUTS_FORWARD(Proxy_DllGetClassObject, "DllGetClassObject", E_FAIL)
MIXEDNUTS_FORWARD(Proxy_DllRegisterServer, "DllRegisterServer", E_FAIL)
MIXEDNUTS_FORWARD(Proxy_DllUnregisterServer, "DllUnregisterServer", E_FAIL)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        InitializeCriticalSection(&g_lock);

        g_modDir = mixednuts::GameDir() + L"Mods\\native120fps\\";

        LoadConfig();
        mixednuts::proxy::Init(L"dinput8.dll");

        // 差し替えはゲームがアーカイブを開く前に仕掛ける必要があるので、
        // ここで同期的に掛ける。実際の生成は最初に開かれたときに行う。
        g_iatSlot = mixednuts::iat::FindImport(GetModuleHandleW(nullptr), "KERNEL32.dll",
                                               "CreateFileW");
        ApplyHook();

        if (HANDLE t = CreateThread(nullptr, 0, HookGuard, nullptr, 0, nullptr))
            CloseHandle(t);
        if (HANDLE t = CreateThread(nullptr, 0, LoadMods, nullptr, 0, nullptr))
            CloseHandle(t);
    }
    return TRUE;
}
