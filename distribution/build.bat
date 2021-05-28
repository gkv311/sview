@echo off
rem This script prepare distribution package for sView
rem under Window platform

rem Configure environment
if "%INNO_PATH%"=="" set "INNO_PATH=%PROGRAMFILES%\Inno Setup 5"
if "%CB_PATH%"==""   set "CB_PATH=%PROGRAMFILES%\CodeBlocks"

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

if exist "%~dp0env.bat" call "%~dp0env.bat"

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

rem Activate experimental WebP support
echo #define ST_HAVE_WEBP>> "%SVIEW_BUILD_CONF%"
rem Activate notifications about sView updates available on sview.ru
echo #define ST_UPDATES_CHECK>> "%SVIEW_BUILD_CONF%"

rem Create configuration for InnoSetup build script
echo #define SVIEW_VER      "%YEAR%.%MONTH%.%SVIEW_VER_TYPE_NUM%.%DAY%"> config.iss
echo #define SVIEW_VER_FULL "v.%YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%">> config.iss
echo #define SVIEW_VER_NAME "sView (version %YEAR%.%MONTH00%%SVIEW_VER_TYPE%%DAY%)">> config.iss
echo #define SVIEW_DISTR_PATH_x86   "%SVIEW_DISTR_PATH_X86%">> config.iss
echo #define SVIEW_DISTR_PATH_AMD64 "%SVIEW_DISTR_PATH_AMD64%">> config.iss

echo #ifndef SVIEW_SDK_VER_STATUS>> "%SVIEW_BUILD_CONF%"
echo   #define SVIEW_SDK_VER_STATUS %releaseStatus%>> "%SVIEW_BUILD_CONF%"
echo #endif>> "%SVIEW_BUILD_CONF%"

echo #define ST_HAVE_OPENVR>> "%SVIEW_BUILD_CONF%"
echo #define ST_HAVE_WEBP>> "%SVIEW_BUILD_CONF%"
echo #define ST_HAVE_MONGOOSE>> "%SVIEW_BUILD_CONF%"

echo #endif //__stConfig_conf_>> "%SVIEW_BUILD_CONF%"
rem END creating config file

echo Perform rebuild MSVC x86
rem www.codeblocks.org
rem --multiple-instance
start /WAIT /D "%CB_PATH%" codeblocks.exe --rebuild --target="WIN_vc_x86" "%~dp0..\workspace.workspace"
if errorlevel 1 (
  move /Y ..\include\stconfig.conf.buildbak ..\include\stconfig.conf
  echo Build FAILED
  pause
  exit /B
  goto :eof
)
echo Perform rebuild MSVC x86_64
start /WAIT /D "%CB_PATH%" codeblocks.exe --rebuild --target="WIN_vc_AMD64" "%~dp0..\workspace.workspace"
if errorlevel 1 (
  move /Y ..\include\stconfig.conf.buildbak ..\include\stconfig.conf
  echo Build FAILED
  pause
  exit /B
  goto :eof
)

rem move default config file back
move /Y ..\include\stconfig.conf.buildbak ..\include\stconfig.conf

echo Copy files into intermidiate directory:
rem x86 binaries
echo "%SVIEW_DISTR_PATH_X86%"
rmdir /S /Q "%SVIEW_DISTR_PATH_X86%
xcopy /Y ..\bin\WIN_vc_x86\*.dll            %SVIEW_DISTR_PATH_X86%\
xcopy /Y ..\bin\WIN_vc_x86\*.exe            %SVIEW_DISTR_PATH_X86%\

rem x86_64 binaries
echo "%SVIEW_DISTR_PATH_AMD64%"
rmdir /S /Q "%SVIEW_DISTR_PATH_AMD64%
xcopy /Y ..\bin\WIN_vc_AMD64\*.dll          %SVIEW_DISTR_PATH_AMD64%\
xcopy /Y ..\bin\WIN_vc_AMD64\*.exe          %SVIEW_DISTR_PATH_AMD64%\

rem shared resources
xcopy /Y ..\share\sView\demo\demo.jps       %SVIEW_DISTR_PATH_X86%\
xcopy /Y ..\share\sView\demo\demo_robot.jps %SVIEW_DISTR_PATH_X86%\
xcopy /S /Y ..\bin\WIN_vc_x86\lang\*        %SVIEW_DISTR_PATH_X86%\lang\
xcopy /S /Y ..\bin\WIN_vc_x86\shaders\*     %SVIEW_DISTR_PATH_X86%\shaders\
xcopy /Y ..\bin\WIN_vc_x86\textures\*       %SVIEW_DISTR_PATH_X86%\textures\
xcopy /Y ..\bin\WIN_vc_x86\web\*            %SVIEW_DISTR_PATH_X86%\web\
xcopy /Y media\sView_JPS.ico                %SVIEW_DISTR_PATH_X86%\icons\
xcopy /Y media\sView_PNS.ico                %SVIEW_DISTR_PATH_X86%\icons\
xcopy /Y media\sView_Media.ico              %SVIEW_DISTR_PATH_X86%\icons\
xcopy /S /Y info\*                          %SVIEW_DISTR_PATH_X86%\info\
copy  /Y ..\docs\license-gpl-3.0.txt        %SVIEW_DISTR_PATH_X86%\info\license.txt

echo Compile distribution package
rem www.jrsoftware.org
move /Y config.iss temp\
start /WAIT /D "%INNO_PATH%" Compil32.exe /cc "%~dp0build.iss"

pause
