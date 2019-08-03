@echo OFF

rem Auxiliary script for semi-automated building of mbedtls (https://tls.mbed.org/) for Android platform.
rem Script should be placed into root of mbedtls repository, edited with paths
rem to CMake, 3rd-parties, Android NDK and MinGW64 make tool.

rem CMake toolchain definition should be cloned from the following git repository:
rem https://github.com/taka-no-me/android-cmake

rem CMake can be downloaded from official site:
rem https://cmake.org/download/

rem Android NDK can be downloaded from offical site:
rem https://developer.android.com/ndk/downloads

rem x86 build is broken on old gcc due to https://github.com/ARMmbed/mbedtls/issues/1910
rem include/mbedtls/bn_mul.h
rem -#if defined(__i386__) && defined(__OPTIMIZE__) && !defined(__ANDROID__)
rem +#if defined(__i386__) && defined(__OPTIMIZE__) && !defined(__ANDROID__)
rem
rem #define MULADDC_INIT

set "anMbedTlsSrc=%~dp0"
set aNbJobs=%NUMBER_OF_PROCESSORS%

call c:\develop\MinGW\TDM-GCC-64\mingwvars.bat
set "PATH=c:\develop\CMake\bin;%PATH%"
set "anNdkPath=c:/develop/android/android-ndk-r12"
set "aToolchain=c:/develop/android-cmake/android.toolchain.cmake"

set "anInstDir=%~dp0work/mbedtls-android"
if exist "%anInstDir%" ( rmdir /S /Q "%anInstDir%" )

call :cmakeGenerate "15" "armeabi-v7a"
call :cmakeGenerate "21" "arm64-v8a"
call :cmakeGenerate "15" "x86"

pause

goto :eof

:cmakeGenerate
set "anApi=%1"
set "anAbi=%2"
set "aPlatformAndCompiler=android-%anAbi%-gcc"
set "aWorkDir=work\%aPlatformAndCompiler%-make"
set "aLogFile=%~dp0build-%anAbi%.log"
if not exist "%aWorkDir%" ( mkdir "%aWorkDir%" )
if     exist "%aLogFile%" ( del   "%aLogFile%" )

pushd "%aWorkDir%"

echo Configuring mbedtls for Android %anAbi% (API level %anApi%)...
cmake -G "MinGW Makefiles" ^
 -D CMAKE_TOOLCHAIN_FILE:FILEPATH="%aToolchain%" ^
 -D ANDROID_NDK:FILEPATH="%anNdkPath%" ^
 -D CMAKE_BUILD_TYPE:STRING="Release" ^
 -D ANDROID_ABI:STRING="%anAbi%" ^
 -D ANDROID_NATIVE_API_LEVEL:STRING="%anApi%" ^
 -D ANDROID_STL:STRING="gnustl_shared" ^
 -D CMAKE_INSTALL_PREFIX:PATH="%anInstDir%" ^
 -D LIBRARY_OUTPUT_PATH:PATH="%anInstDir%/libs/%anAbi%" ^
 -D USE_SHARED_MBEDTLS_LIBRARY:BOOL="OFF" ^
 -D USE_STATIC_MBEDTLS_LIBRARY:BOOL="ON" ^
 -D ENABLE_TESTING:BOOL="OFF" ^
 -D ENABLE_PROGRAMS:BOOL="OFF" ^
 "%anMbedTlsSrc%"

if errorlevel 1 (
  popd
  pause
  exit /B
  goto :eof
)

mingw32-make clean

echo Building mbedtls...
mingw32-make -j %aNbJobs% 2>> %aLogFile%
type %aLogFile%
if errorlevel 1 (
  popd
  pause
  exit /B
  goto :eof
)
echo Installing mbedtls into %~dp0work/%aPlatformAndCompiler%...
mingw32-make install 2>> %aLogFile%
if exist "%anInstDir%/lib" ( rmdir /S /Q "%anInstDir%/lib" )

popd
goto :eof
