@echo off

rem if ["%PATH_BACK%"] == [""] set "PATH_BACK=%PATH%"
rem set "PATH=%PATH_BACK%"
set "PATH=%SYSTEMROOT%\system32;%SYSTEMROOT%"
rem set "PATH_BACK=%PATH%"

if exist "%~dp0custom.bat" call "%~dp0custom.bat"

set "ink=inkscape.com"
set "ffm=ffmpeg.exe"
set "anSvgName=%1"
set "aPngName=%2"
if ["%aPngName%"] == [""] set "aPngName=%1"
if not exist "%anSvgName%" set "anSvgName=%anSvgName%.svg"

for %%s in (16 24 32 48 64 72 96 128 144 192 256) do (
  if exist "%aPngName%%%s.png" del "%aPngName%%%s.png"
  "%ink%" --file "%anSvgName%" --export-area-page --export-height %%s --export-png "%aPngName%%%s.tmp.png"
  "%ffm%" -i "%aPngName%%%s.tmp.png" -pix_fmt gray "%aPngName%%%s.png"
  del "%aPngName%%%s.tmp.png"
)
