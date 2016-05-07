@echo off

rem Scripts prepares dependencies for building sView
set "aParties=%~dp0..\3rdparty\wnt"

if not exist "%aParties%\unpack.bat" git clone --depth 1 https://github.com/gkv311/sview-deps-wnt.git %aParties%
call "%aParties%\unpack.bat"
