@echo off
rem FATAL FRAME II: Crimson Butterfly REMAKE - Native 120FPS Option
rem Builds dist\ containing the files users drop into the game root.
setlocal
set "VS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VS%" (echo [NG] vcvars64.bat not found: %VS% & exit /b 1)
call "%VS%" >nul
if errorlevel 1 exit /b 1

set "ROOT=%~dp0"
set "OUT=%ROOT%dist"
set "OBJ=%ROOT%obj"
if not exist "%OUT%\Mods\native120fps" mkdir "%OUT%\Mods\native120fps"
if not exist "%OBJ%" mkdir "%OBJ%"

echo === loader (dinput8.dll) ===
cl /nologo /LD /O2 /EHsc /MT /W3 /std:c++17 /utf-8 /DNDEBUG /Fo"%OBJ%\l_" /Fe"%OUT%\dinput8.dll" "%ROOT%src\loader\dinput8.cpp" /link /DEF:"%ROOT%src\loader\dinput8.def" /OPT:REF /OPT:ICF user32.lib
if errorlevel 1 exit /b 1

echo === payload (native120fps.dll) ===
cl /nologo /LD /O2 /EHsc /MT /W3 /std:c++17 /utf-8 /DNDEBUG /Fo"%OBJ%\p_" /Fe"%OUT%\Mods\native120fps\native120fps.dll" "%ROOT%src\payload\native120fps.cpp" /link /OPT:REF /OPT:ICF version.lib user32.lib
if errorlevel 1 exit /b 1

echo === copying package files ===
copy /y "%ROOT%package\Mods\native120fps\native120fps.ini" "%OUT%\Mods\native120fps\" >nul
copy /y "%ROOT%package\Mods\native120fps\README.md" "%OUT%\Mods\native120fps\" >nul
copy /y "%ROOT%LICENSE" "%OUT%\Mods\native120fps\LICENSE.txt" >nul

rem stale output from earlier builds
if exist "%OUT%\Mods\native120fps\README.txt" del "%OUT%\Mods\native120fps\README.txt"
rem import library / export file are build by-products
if exist "%OUT%\dinput8.lib" del "%OUT%\dinput8.lib"
if exist "%OUT%\dinput8.exp" del "%OUT%\dinput8.exp"

echo.
echo === done: %OUT% ===
dir /b /s "%OUT%"
endlocal
