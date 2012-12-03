@echo off

echo Replace backslashes '\' with forwardslashes '/' in Code::Block project files
echo WARNING! Unix sed tool should be available in PATH

dir /B /AD "%~dp0St*" > StProjectsDirsList.txt.tmp
echo sView >> StProjectsDirsList.txt.tmp

for /F %%d in (StProjectsDirsList.txt.tmp) do (
REM  echo "%%d"
  dir /B "%~dp0%%d\*.cbp" > StProjectsFilesList.txt.tmp
  for /F %%f in (StProjectsFilesList.txt.tmp) do (
    echo   Parsing %%f
    rem make a backup file
    move /Y "%~dp0%%d\%%f" "%~dp0%%d\%%f.bak"
    rem tr \\ / <"%~dp0%%d\%%f.bak" >"%~dp0%%d\%%f"
    rem replace slashes, ignore lines with pre/post build steps commands
    sed -f reslasher.sed "%~dp0%%d\%%f.bak" > "%~dp0%%d\%%f.seded"
    rem restore UNIX endlines
    tr -d '\015' <"%~dp0%%d\%%f.seded" >"%~dp0%%d\%%f"
    del "%~dp0%%d\%%f.seded"
  )
  del StProjectsFilesList.txt.tmp
)
del StProjectsDirsList.txt.tmp

pause
