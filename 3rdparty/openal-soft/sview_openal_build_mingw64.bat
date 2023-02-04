@echo OFF

rem Auxiliary script for semi-automated building of OpenAL soft (https://openal-soft.org/) for WNT platform.
rem Script should be placed into root of OpenAL repository, edited with paths to CMake.

rem CMake can be downloaded from official site:
rem https://cmake.org/download/

set "aSrcRoot=%~dp0"

set "aVcVer=mingw64"
set "anArch=win64"
call c:\mingw64-x86_64-12.2.0-release-posix-seh-msvcrt-rt_v10-rev2\mingwvars.bat
set "PATH=c:\Storage\Develop\DEVsoft\CMake\bin;%PATH%"

set aNbJobs=%NUMBER_OF_PROCESSORS%

call :cmakeGenerate

pause

goto :eof

:cmakeGenerate
set "anApi=%1"
set "anAbi=%2"
set "aPlatformAndCompiler=wnt-%aVcVer%-%anArch%"
set "aWorkDir=work\%aPlatformAndCompiler%-make"
set "aLogFile=%~dp0build-%aPlatformAndCompiler%.log"
set "aLogFileBuild=%~dp0build-%aPlatformAndCompiler%-build.log"
set "aLogFileInstall=%~dp0build-%aPlatformAndCompiler%-install.log"
if not exist "%aWorkDir%"        ( mkdir "%aWorkDir%" )
if     exist "%aLogFile%"        ( del   "%aLogFile%" )
if     exist "%aLogFileBuild%"   ( del   "%aLogFileBuild%" )
if     exist "%aLogFileInstall%" ( del   "%aLogFileInstall%" )

echo Start building OpenAL soft for %aPlatformAndCompiler%
echo Start building OpenAL soft for %aPlatformAndCompiler%>> %aLogFile%

echo Configuring OpenAL soft for WNT...
cmake -G "MinGW Makefiles" ^
 -D CMAKE_BUILD_TYPE:STRING="Release" ^
 -D BUILD_LIBRARY_TYPE:STRING="Shared" ^
 -D CMAKE_INSTALL_PREFIX:PATH="%~dp0work/%aPlatformAndCompiler%" ^
 -D ALSOFT_EXAMPLES:BOOL="OFF" ^
 -D ALSOFT_TESTS:BOOL="OFF" ^
 -D ALSOFT_UTILS:BOOL="OFF" ^
 -D ALSOFT_NO_CONFIG_UTIL:BOOL="OFF" ^
 -D ALSOFT_BUILD_ROUTER:BOOL="OFF" ^
 -D ALSOFT_REQUIRE_WINMM:BOOL="ON" ^
 -D ALSOFT_REQUIRE_DSOUND:BOOL="ON" ^
 -D ALSOFT_REQUIRE_WASAPI:BOOL="ON" ^
 -D ALSOFT_STATIC_LIBGCC:BOOL="ON" ^
 -D ALSOFT_STATIC_STDCXX:BOOL="ON" ^
 -D ALSOFT_STATIC_WINPTHREAD:BOOL="ON" ^
 -B "%aWorkDir%" -S "%aSrcRoot%"

if errorlevel 1 (
  pause
  exit /B 1
  goto :eof
)

cmake --build "%aWorkDir%" --config "Release" -- -j%aNbJobs%
cmake --build "%aWorkDir%" --config "Release" --target install

goto :eof
