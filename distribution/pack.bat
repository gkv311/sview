@echo off
rem This script prepare distribution package for sView under Window platform

rem Configure environment (paths to 3rd-party tools and libraries)
set "aCmakeBin="
set "aVsVars="
set "aFreeType="
set "anFFmpeg="
set "anOpenAL="
set "anOpenVR="
set "anMsvcr="
set USE_OPENVR=ON
set USE_MSVCR=ON
set USE_MONGOOSE=ON
rem Activate notifications about sView updates available on sview.ru
set USE_UPDATER=ON

rem Configuration file
if exist "%~dp0msvc_custom.bat" call "%~dp0msvc_custom.bat"

if "%INNO_PATH%"=="" (
  if exist "%PROGRAMFILES%\Inno Setup 6\Compil32.exe" set "INNO_PATH=%PROGRAMFILES%\Inno Setup 6"
)
if "%INNO_PATH%"=="" (
  if exist "%PROGRAMFILES%\Inno Setup 5\Compil32.exe" set "INNO_PATH=%PROGRAMFILES%\Inno Setup 5"
)

rem Build type
set releaseStatus=ST_RELEASE
set SVIEW_VER_TYPE=_
set SVIEW_VER_TYPE_NUM=4

if "%1"=="ST_ALPHA" (
  set releaseStatus=ST_ALPHA
  set SVIEW_VER_TYPE=alpha
  set SVIEW_VER_TYPE_NUM=1
)

if "%1"=="ST_RELEASE_CANDIDATE" (
  set releaseStatus=ST_RELEASE_CANDIDATE
  set SVIEW_VER_TYPE=rc
  set SVIEW_VER_TYPE_NUM=3
)

for /F "skip=1 delims=" %%F in ('
  wmic PATH Win32_LocalTime GET Day^,Month^,Year /FORMAT:TABLE
') do (
  for /F "tokens=1-3" %%L in ("%%F") do (
    set DAY=%%L
    set DAY00=0%%L
    set MONTH=%%M
    set MONTH00=0%%M
    set YEAR=%%N
  )
)
set YEAR=%YEAR:~-2,4%

set "anArchName=sView_v.%YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%_amd64"
rem set "SVIEW_DISTR_PATH=%~dp0temp\sView-win-amd64"
set "SVIEW_DISTR_PATH=%~dp0temp\%anArchName%"

rem Archive tool
set "THE_7Z_PARAMS=-t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"
set "THE_7Z_PATH=%ProgramW6432%\7-Zip\7z.exe"
echo Creating archive %anArchName%.7z
if exist "%~dp0repository\win\%anArchName%.7z" del "%~dp0repository\win\%anArchName%.7z"
pushd "%~dp0temp"
"%THE_7Z_PATH%" a -r %THE_7Z_PARAMS% "%~dp0repository/win/%anArchName%.7z" "%anArchName%"
popd

echo Compile distribution package

rem Create configuration for InnoSetup build script
echo #define SVIEW_VER      "%YEAR%.%MONTH%.%SVIEW_VER_TYPE_NUM%.%DAY%"> config.iss
echo #define SVIEW_VER_FULL "v.%YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%">> config.iss
echo #define SVIEW_VER_NAME "sView (version %YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%)">> config.iss
echo #define SVIEW_DISTR_PATH "%SVIEW_DISTR_PATH%">> config.iss

rem www.jrsoftware.org
move /Y config.iss temp\
start /WAIT /D "%INNO_PATH%" Compil32.exe /cc "%~dp0build.iss"

pause

goto :eof
