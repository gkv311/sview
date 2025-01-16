@echo off
rem This script prepare distribution package for sView
rem under Window platform

rem Configure environment

rem Paths to 3rd-party tools and libraries
set "aCmakeBin="
set "aVsVars="
set "aFreeType32="
set "aFreeType64="
set "anFFmpeg32="
set "anFFmpeg64="
set "anNVAPI32="
set "anNVAPI64="
set "anOpenAL32="
set "anOpenAL64="
set "anOpenVR32="
set "anOpenVR64="
set "aFreeImage32="
set "aFreeImage64="
set "aDevIL32="
set "aDevIL64="
set "anMsvcr32="
set "anMsvcr64="
set USE_OPENVR=ON
set USE_FREEIMAGE=ON
set USE_DEVIL=ON
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
echo "INNO_PATH=%INNO_PATH%"

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

set SVIEW_DISTR_PATH_X86=%~dp0temp\sView-win-x86
set SVIEW_DISTR_PATH_AMD64=%~dp0temp\sView-win-amd64

rem make backup of default config file
move /Y ..\include\stconfig.conf ..\include\stconfig.conf.buildbak

set SVIEW_BUILD_CONF=..\include\stconfig.conf

echo Build configuration:
echo     Target="WIN_vc_x86"

rem START creating config file
echo #ifndef __stConfig_conf_> "%SVIEW_BUILD_CONF%"
echo #define __stConfig_conf_>> "%SVIEW_BUILD_CONF%"

if not "%releaseStatus%"=="ST_RELEASE" (
  echo     Timebomb=ON
  echo #ifndef ST_TIMEBOMB>> "%SVIEW_BUILD_CONF%"
  echo   #define ST_TIMEBOMB>> "%SVIEW_BUILD_CONF%"
  echo #endif>> "%SVIEW_BUILD_CONF%"
) else (
  echo     Timebomb=OFF
)

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
echo #define SVIEW_DISTR_PATH_x86   "%SVIEW_DISTR_PATH_X86%">> config.iss
echo #define SVIEW_DISTR_PATH_AMD64 "%SVIEW_DISTR_PATH_AMD64%">> config.iss

echo #ifndef SVIEW_SDK_VER_STATUS>> "%SVIEW_BUILD_CONF%"
echo   #define SVIEW_SDK_VER_STATUS %releaseStatus%>> "%SVIEW_BUILD_CONF%"
echo #endif>> "%SVIEW_BUILD_CONF%"

echo #endif //__stConfig_conf_>> "%SVIEW_BUILD_CONF%"
rem END creating config file

echo Perform rebuild MSVC x86
set "aPathBack2=%PATH%"
set "SVIEW_BUILD_PATH_X86=%~dp0temp\bin\WIN_vc_x86"
set "SVIEW_BUILD_PATH_AMD64=%~dp0temp\bin\WIN_vc_AMD64"
rmdir /S /Q "%SVIEW_DISTR_PATH_X86%
rmdir /S /Q "%SVIEW_DISTR_PATH_AMD64%
if not exist "%~dp0temp" ( mkdir "%~dp0temp" )
if not exist "%~dp0temp\bin" ( mkdir "%~dp0temp\bin" )

call :build_sview "%SVIEW_BUILD_PATH_X86%" x86
if errorlevel 1 (
  move /Y ..\include\stconfig.conf.buildbak ..\include\stconfig.conf
  echo Build FAILED
  pause
  exit /B
  goto :eof
)
set "PATH=%aPathBack2%"

echo Perform rebuild MSVC x86_64
call :build_sview "%SVIEW_BUILD_PATH_AMD64%" x64
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

echo Copy files into intermidiate directory:
rem x86 binaries
echo "%SVIEW_DISTR_PATH_X86%"
xcopy /Y "%SVIEW_BUILD_PATH_X86%\*.dll"          "%SVIEW_DISTR_PATH_X86%\"
xcopy /Y "%SVIEW_BUILD_PATH_X86%\*.exe"          "%SVIEW_DISTR_PATH_X86%\"

rem x86_64 binaries
echo "%SVIEW_DISTR_PATH_AMD64%"
rmdir /S /Q "%SVIEW_DISTR_PATH_AMD64%
xcopy /Y "%SVIEW_BUILD_PATH_AMD64%\*.dll"        "%SVIEW_DISTR_PATH_AMD64%\"
xcopy /Y "%SVIEW_BUILD_PATH_AMD64%\*.exe"        "%SVIEW_DISTR_PATH_AMD64%\"

rem shared resources
xcopy /Y "..\share\sView\demo\demo.jps"          "%SVIEW_DISTR_PATH_X86%\"
xcopy /Y "..\share\sView\demo\demo_robot.jps"    "%SVIEW_DISTR_PATH_X86%\"
xcopy /S /Y "%SVIEW_BUILD_PATH_AMD64%\lang\*"    "%SVIEW_DISTR_PATH_X86%\lang\"
xcopy /S /Y "%SVIEW_BUILD_PATH_AMD64%\shaders\*" "%SVIEW_DISTR_PATH_X86%\shaders\"
xcopy /Y "%SVIEW_BUILD_PATH_AMD64%\textures\*"   "%SVIEW_DISTR_PATH_X86%\textures\"
xcopy /Y "%SVIEW_BUILD_PATH_AMD64%\web\*"        "%SVIEW_DISTR_PATH_X86%\web\"
xcopy /Y "media\sView_JPS.ico"                   "%SVIEW_DISTR_PATH_X86%\icons\"
xcopy /Y "media\sView_PNS.ico"                   "%SVIEW_DISTR_PATH_X86%\icons\"
xcopy /Y "media\sView_Media.ico"                 "%SVIEW_DISTR_PATH_X86%\icons\"
xcopy /S /Y "info\*"                             "%SVIEW_DISTR_PATH_X86%\info\"
copy  /Y "..\docs\license-gpl-3.0.txt"           "%SVIEW_DISTR_PATH_X86%\info\license.txt"

rem Archive tool
set "THE_7Z_PARAMS=-t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"
set "THE_7Z_PATH=%ProgramW6432%\7-Zip\7z.exe"
set "anArchName=sView_v.%YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%_amd64"
set "SVIEW_DISTR_PATH_ARCH=%~dp0temp\arch\%anArchName%"
echo Creating archive %anArchName%.7z
rmdir /S /Q "%~dp0temp\arch"
if exist "%~dp0repository\win\%anArchName%.7z" del "%~dp0repository\win\%anArchName%.7z"
xcopy /Y "%SVIEW_BUILD_PATH_AMD64%\*.dll"        "%SVIEW_DISTR_PATH_ARCH%\"
xcopy /Y "%SVIEW_BUILD_PATH_AMD64%\*.exe"        "%SVIEW_DISTR_PATH_ARCH%\"
xcopy /Y "..\share\sView\demo\demo.jps"          "%SVIEW_DISTR_PATH_ARCH%\"
xcopy /Y "..\share\sView\demo\demo_robot.jps"    "%SVIEW_DISTR_PATH_ARCH%\"
xcopy /S /Y "%SVIEW_BUILD_PATH_AMD64%\lang\*"    "%SVIEW_DISTR_PATH_ARCH%\lang\"
xcopy /S /Y "%SVIEW_BUILD_PATH_AMD64%\shaders\*" "%SVIEW_DISTR_PATH_ARCH%\shaders\"
xcopy /Y "%SVIEW_BUILD_PATH_AMD64%\textures\*"   "%SVIEW_DISTR_PATH_ARCH%\textures\"
xcopy /Y "%SVIEW_BUILD_PATH_AMD64%\web\*"        "%SVIEW_DISTR_PATH_ARCH%\web\"
xcopy /Y "media\sView_JPS.ico"                   "%SVIEW_DISTR_PATH_ARCH%\icons\"
xcopy /Y "media\sView_PNS.ico"                   "%SVIEW_DISTR_PATH_ARCH%\icons\"
xcopy /Y "media\sView_Media.ico"                 "%SVIEW_DISTR_PATH_ARCH%\icons\"
xcopy /S /Y "info\*"                             "%SVIEW_DISTR_PATH_ARCH%\info\"
copy  /Y "..\docs\license-gpl-3.0.txt"           "%SVIEW_DISTR_PATH_ARCH%\info\license.txt"
pushd "%~dp0temp\arch"
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
  set "aFreeType=%aFreeType32%"
  set "anFFmpeg=%anFFmpeg32%"
  set "anNVAPI=%anNVAPI32%"
  set "anOpenAL=%anOpenAL32%"
  set "anOpenVR=%anOpenVR32%"
  set "aFreeImage=%aFreeImage32%"
  set "aDevIL=%aDevIL32%"
  set "anMsvcr=%anMsvcr32%"
)
if /i "%~2" == "x64" (
  call "%aVsVars%" x64
  set "aFreeType=%aFreeType64%"
  set "anFFmpeg=%anFFmpeg64%"
  set "anNVAPI=%anNVAPI64%"
  set "anOpenAL=%anOpenAL64%"
  set "anOpenVR=%anOpenVR64%"
  set "aFreeImage=%aFreeImage64%"
  set "aDevIL=%aDevIL64%"
  set "anMsvcr=%anMsvcr64%"
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
 -D USE_FREEIMAGE:BOOL="%USE_FREEIMAGE%" ^
 -D FREEIMAGE_DIR:PATH="%aFreeImage%" ^
 -D USE_DEVIL:BOOL="%USE_DEVIL%" ^
 -D DEVIL_DIR:PATH="%aDevIL%" ^
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
