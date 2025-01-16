@echo OFF

rem Auxiliary script for semi-automated building of sView for Windows platform.
rem mingw_custom.bat should be configured with paths to CMake, 3rd-parties and MSVC.

set "aSrcRoot=%~dp0.."
set "aBuildRoot=%~dp0temp"

set aNbJobs=%NUMBER_OF_PROCESSORS%

rem Paths to 3rd-party tools and libraries
set "aCmakeBin="
set "aVsVars="
set "aCmakeGen=Visual Studio 14 2015 Win64"
set "aFreeType="
set "anFFmpeg="
set "anNVAPI="
set "anOpenAL="
set "anOpenVR="
set "anMsvcr="

set USE_OPENVR=ON
set USE_MSVCR=OFF

rem Build stages to perform
set "toCMake=1"
set "toClean=0"
set "toMake=1"
set "toInstall=1"
set "toPack=0"
set "toDebug=0"

rem Archive tool
set "THE_7Z_PARAMS=-t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"
set "THE_7Z_PATH=%ProgramW6432%\7-Zip\7z.exe"

rem Configuration file
if exist "%~dp0msvc_custom.bat" call "%~dp0msvc_custom.bat"

if not ["%aVsVars%"] == [""] ( call "%aVsVars%" x64 )
rem call vcvarsall.bat x64 -vcvars_ver=14.0
if not ["%aCmakeBin%"] == [""] ( set "PATH=%aCmakeBin%;%PATH%" )

set "aBuildType=Release"
set "aBuildTypePrefix="
if ["%toDebug%"] == ["1"] (
  set "aBuildType=Debug"
  set "aBuildTypePrefix=-debug"
)

call :cmakeGenerate
if errorlevel 1 (
  if not ["%1"] == ["-nopause"] (
    pause
  )
  exit /B 1
  goto :eof
)

for /F "skip=1 delims=" %%F in ('
  wmic PATH Win32_LocalTime GET Day^,Month^,Year /FORMAT:TABLE
') do (
  for /F "tokens=1-3" %%L in ("%%F") do (
    set DAY00=0%%L
    set MONTH00=0%%M
    set YEAR=%%N
  )
)
set DAY00=%DAY00:~-2%
set MONTH00=%MONTH00:~-2%
set "aRevision=-%YEAR%-%MONTH00%-%DAY00%"
rem set "aRevision=-%aGitBranch%"
set "anArchName=sview-%aRevision%-msvc%aBuildTypePrefix%"
set "aTarget=%aBuildRoot%\%anArchName%"
if ["%toPack%"] == ["1"] (
  echo Creating archive %anArchName%.7z
  rmdir /S /Q "%aTarget%"
  if not exist "%aTarget%" ( mkdir "%aTarget%" )
  if exist "%aBuildRoot%/%anArchName%.7z" del "%aBuildRoot%/%anArchName%.7z"
  xcopy /S /Y "%aDestDir%\*" "%aTarget%\"

  echo Copying dependencies...
  for %%i in (libstdc++-6.dll libwinpthread-1.dll libgcc_s_seh-1.dll) do (
    if "%%~$PATH:i" == "" (
      echo "Error: could not find %%i"
    ) else (
      xcopy /Y "%%~$PATH:i" "%aTarget%\win64\gcc\bin"
    )
  )

  "%THE_7Z_PATH%" a -r %THE_7Z_PARAMS% "%aBuildRoot%/%anArchName%.7z" "%aTarget%"
)
if not ["%1"] == ["-nopause"] (
  pause
)

goto :eof

:cmakeGenerate
set "aPlatformAndCompiler=msvc%aBuildTypePrefix%"
set "aWorkDir=%aBuildRoot%\sview-%aPlatformAndCompiler%-make"
set "aDestDir=%aBuildRoot%\sview-%aPlatformAndCompiler%"
set "aLogFile=%aBuildRoot%\sview-%aPlatformAndCompiler%-build.log"
if ["%toCMake%"] == ["1"] (
  if ["%toClean%"] == ["1"] (
    rmdir /S /Q "%aWorkDir%"
    rmdir /S /Q "%aDestDir%"
  )
)
if not exist "%aWorkDir%" ( mkdir "%aWorkDir%" )
if     exist "%aLogFile%" ( del   "%aLogFile%" )

rem include some information into archive
echo ^<pre^>> "%aWorkDir%\VERSION.html"
git status >> "%aWorkDir%\VERSION.html"
git log -n 100 >> "%aWorkDir%\VERSION.html"
echo ^</pre^>>> "%aWorkDir%\VERSION.html"

echo Start building for %aPlatformAndCompiler%
echo Start building for %aPlatformAndCompiler%>> %aLogFile%

if ["%toCMake%"] == ["1"] (
  echo Configuring for Windows...
  cmake -G "%aCmakeGen%" ^
 -D CMAKE_BUILD_TYPE:STRING="%aBuildType%" ^
 -D CMAKE_INSTALL_PREFIX:PATH="%aDestDir%" ^
 -D BUILD_FORCE_RelWithDebInfo:BOOL="ON" ^
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
)

if ["%toClean%"] == ["1"] (
  cmake --build "%aWorkDir%" --config %aBuildType% --target clean
)

if ["%toMake%"] == ["1"] (
  echo Building...
  cmake --build "%aWorkDir%" --config %aBuildType% 2>> "%aLogFile%"
  if errorlevel 1 (
    type "%aLogFile%"
    exit /B 1
    goto :eof
  )
  type "%aLogFile%"
)

if ["%toInstall%"] == ["1"] (
  echo Installing into %aDestDir%...
  cmake --build "%aWorkDir%" --config %aBuildType% --target install 2>> "%aLogFile%"
  copy /Y "%aWorkDir%\VERSION.html" "%aDestDir%\VERSION.html"
)

goto :eof
