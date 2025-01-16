@echo off
rem This script prepare distribution package for sView under Window platform

rem Configure environment (paths to 3rd-party tools and libraries)
set "aCmakeBin="
set "aVsVars="
set "aFreeType="
set "anFFmpeg="
set "anNVAPI="
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
rmdir /S /Q "%SVIEW_DISTR_PATH%

rem make backup of default config file
move /Y ..\include\stconfig.conf ..\include\stconfig.conf.buildbak

set SVIEW_BUILD_CONF=..\include\stconfig.conf

rem START creating config file
echo #ifndef __stConfig_conf_> "%SVIEW_BUILD_CONF%"
echo #define __stConfig_conf_>> "%SVIEW_BUILD_CONF%"

echo     Version=^%YEAR%^, ^%MONTH%^, ^%SVIEW_VER_TYPE_NUM%^, ^%DAY%
echo #ifndef SVIEW_SDK_VERSION>> "%SVIEW_BUILD_CONF%"
echo   #define SVIEW_SDK_VERSION ^%YEAR%^, ^%MONTH%^, ^%SVIEW_VER_TYPE_NUM%^, ^%DAY%>> "%SVIEW_BUILD_CONF%"
echo #endif>> "%SVIEW_BUILD_CONF%"

echo     Version String="%YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%"
echo #ifndef SVIEW_SDK_VER_STRING>> "%SVIEW_BUILD_CONF%"
echo   #define SVIEW_SDK_VER_STRING "%YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%">> "%SVIEW_BUILD_CONF%"
echo #endif>> "%SVIEW_BUILD_CONF%"

rem Create configuration for InnoSetup build script
echo #define SVIEW_VER      "%YEAR%.%MONTH%.%SVIEW_VER_TYPE_NUM%.%DAY%"> config.iss
echo #define SVIEW_VER_FULL "v.%YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%">> config.iss
echo #define SVIEW_VER_NAME "sView (version %YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%)">> config.iss
echo #define SVIEW_DISTR_PATH "%SVIEW_DISTR_PATH%">> config.iss

echo #ifndef SVIEW_SDK_VER_STATUS>> "%SVIEW_BUILD_CONF%"
echo   #define SVIEW_SDK_VER_STATUS %releaseStatus%>> "%SVIEW_BUILD_CONF%"
echo #endif>> "%SVIEW_BUILD_CONF%"

echo #endif //__stConfig_conf_>> "%SVIEW_BUILD_CONF%"
rem END creating config file

set "aPathBack2=%PATH%"
if not exist "%~dp0temp" ( mkdir "%~dp0temp" )

echo Perform rebuild MSVC x86_64
call :build_sview "%SVIEW_DISTR_PATH%" x64
if errorlevel 1 (
  move /Y ..\include\stconfig.conf.buildbak ..\include\stconfig.conf
  echo Build FAILED
  pause
  exit /B
  goto :eof
)
set "PATH=%aPathBack2%"

rem move default config file back
move /Y ..\include\stconfig.conf.buildbak ..\include\stconfig.conf

rem Archive tool
set "THE_7Z_PARAMS=-t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"
set "THE_7Z_PATH=%ProgramW6432%\7-Zip\7z.exe"
echo Creating archive %anArchName%.7z
if exist "%~dp0repository\win\%anArchName%.7z" del "%~dp0repository\win\%anArchName%.7z"
pushd "%~dp0temp"
"%THE_7Z_PATH%" a -r %THE_7Z_PARAMS% "%~dp0repository/win/%anArchName%.7z" "%anArchName%"
popd

echo Compile distribution package
rem www.jrsoftware.org
move /Y config.iss temp\
start /WAIT /D "%INNO_PATH%" Compil32.exe /cc "%~dp0build.iss"

pause

goto :eof

:build_sview
set "aSrcRoot=%~dp0.."
set "aBuildRoot=%~1"

if /i "%~2" == "x86" (
  call "%aVsVars%" x86
)
if /i "%~2" == "x64" (
  call "%aVsVars%" x64
)
if not ["%aCmakeBin%"] == [""] ( set "PATH=%aCmakeBin%;%PATH%" )

set "aWorkDir=%aBuildRoot%-make"
set "aDestDir=%aBuildRoot%"
set "aLogFile=%aBuildRoot%-build.log"

if exist "%aWorkDir%" ( rmdir /S /Q "%aWorkDir%" )
if exist "%aDestDir%" ( rmdir /S /Q "%aDestDir%" )
if exist "%aLogFile%" ( del         "%aLogFile%" )
mkdir "%aWorkDir%"

cmake -G "Ninja" ^
 -D CMAKE_BUILD_TYPE:STRING="Release" ^
 -D CMAKE_INSTALL_PREFIX:PATH="%aDestDir%" ^
 -D BUILD_FORCE_RelWithDebInfo:BOOL="ON" ^
 -D USE_MONGOOSE:BOOL="%USE_MONGOOSE%" ^
 -D USE_UPDATER:BOOL="%USE_UPDATER%" ^
 -D FREETYPE_DIR:PATH="%aFreeType%" ^
 -D FFMPEG_DIR:PATH="%anFFmpeg%" ^
 -D NVAPI_DIR:PATH="%anNVAPI%" ^
 -D OPENAL_DIR:PATH="%anOpenAL%" ^
 -D USE_OPENVR:BOOL="%USE_OPENVR%" ^
 -D OPENVR_DIR:PATH="%anOpenVR%" ^
 -D USE_MSVCR:BOOL="%USE_MSVCR%" ^
 -D MSVCR_DIR:PATH="%anMsvcr%" ^
 -B "%aWorkDir%" -S "%aSrcRoot%"

if errorlevel 1 (
  exit /B 1
  goto :eof
)

cmake --build "%aWorkDir%" --config Release 2>> "%aLogFile%"
if errorlevel 1 (
  type "%aLogFile%"
  exit /B 1
  goto :eof
)

cmake --build "%aWorkDir%" --config Release --target install 2>> "%aLogFile%"
if errorlevel 1 (
  type "%aLogFile%"
  exit /B 1
  goto :eof
)

type "%aLogFile%"
