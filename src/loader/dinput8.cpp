// FATAL FRAME II: Crimson Butterfly REMAKE — Mod ローダ（dinput8.dll プロキシ）
// Created by MixedNuts - https://github.com/MixedNuts-Dev/fatal-frame2-remake-mods
// Licensed under the MIT License. See LICENSE for details.
//
// ゲームのルートに置くと自動的にロードされ、
// Mods\native120fps\native120fps.dll を読み込む。
// dinput8 本来の機能は System32 の実体へ転送するのでゲーム動作に影響しない。

#include <windows.h>
#include <string>

static HMODULE g_real = nullptr;

static void LoadRealDinput8()
{
    if (g_real) return;
    wchar_t path[MAX_PATH]{};
    GetSystemDirectoryW(path, MAX_PATH);
    wcscat_s(path, L"\\dinput8.dll");
    g_real = LoadLibraryW(path);
}

static FARPROC RealProc(const char* name)
{
    LoadRealDinput8();
    return g_real ? GetProcAddress(g_real, name) : nullptr;
}

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

// ---- Mod の読み込み -----------------------------------------------------

static DWORD WINAPI LoadMods(LPVOID)
{
    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring dir(exe);
    dir.resize(dir.find_last_of(L'\\') + 1);

    const std::wstring modDir = dir + L"Mods\\native120fps\\";
    const std::wstring dll = modDir + L"native120fps.dll";

    // Mod 側の DLL が依存物を自分のフォルダから引けるようにする
    SetDllDirectoryW(modDir.c_str());
    LoadLibraryW(dll.c_str());
    SetDllDirectoryW(nullptr);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        LoadRealDinput8();
        // DllMain 内でのロードは避け、別スレッドで行う
        if (HANDLE t = CreateThread(nullptr, 0, LoadMods, nullptr, 0, nullptr))
            CloseHandle(t);
    }
    return TRUE;
}
