@echo off

rem auxiliary script to copy resources (translation files, shaders) to destination folder
set "TARGET_OUTPUT_DIR=%1"
set "TARGET_OUTPUT_BASENAME=%2"
chcp 65001

if not exist "%TARGET_OUTPUT_DIR%lang\English"  mkdir "%TARGET_OUTPUT_DIR%lang\English"
if not exist "%TARGET_OUTPUT_DIR%lang\Español"  mkdir "%TARGET_OUTPUT_DIR%lang\Español"
if not exist "%TARGET_OUTPUT_DIR%lang\русский"  mkdir "%TARGET_OUTPUT_DIR%lang\русский"
if not exist "%TARGET_OUTPUT_DIR%lang\français" mkdir "%TARGET_OUTPUT_DIR%lang\français"
if not exist "%TARGET_OUTPUT_DIR%lang\Deutsch"  mkdir "%TARGET_OUTPUT_DIR%lang\Deutsch"
if not exist "%TARGET_OUTPUT_DIR%lang\ChineseS" mkdir "%TARGET_OUTPUT_DIR%lang\ChineseS"
if not exist "%TARGET_OUTPUT_DIR%lang\ChineseT" mkdir "%TARGET_OUTPUT_DIR%lang\ChineseT"
if not exist "%TARGET_OUTPUT_DIR%lang\Korean"   mkdir "%TARGET_OUTPUT_DIR%lang\Korean"
if not exist "%TARGET_OUTPUT_DIR%lang\Czech"    mkdir "%TARGET_OUTPUT_DIR%lang\Czech"

copy /Y lang\english\* "%TARGET_OUTPUT_DIR%lang\English\"
copy /Y lang\spanish\* "%TARGET_OUTPUT_DIR%lang\Español\"
copy /Y lang\russian\* "%TARGET_OUTPUT_DIR%lang\русский\"
copy /Y lang\french\*  "%TARGET_OUTPUT_DIR%lang\français\"
copy /Y lang\german\*  "%TARGET_OUTPUT_DIR%lang\Deutsch\"
copy /Y lang\chinese\* "%TARGET_OUTPUT_DIR%lang\ChineseS\"
copy /Y lang\chineset\* "%TARGET_OUTPUT_DIR%lang\ChineseT\"
copy /Y lang\korean\*  "%TARGET_OUTPUT_DIR%lang\Korean\"
copy /Y lang\czech\*   "%TARGET_OUTPUT_DIR%lang\Czech\"

if exist "shaders" (
  if not exist "%TARGET_OUTPUT_DIR%shaders\%TARGET_OUTPUT_BASENAME%" mkdir "%TARGET_OUTPUT_DIR%shaders\%TARGET_OUTPUT_BASENAME%"
  copy /Y shaders\* "%TARGET_OUTPUT_DIR%shaders\%TARGET_OUTPUT_BASENAME%\"
)

if exist "web" (
  if not exist "%TARGET_OUTPUT_DIR%web" mkdir "%TARGET_OUTPUT_DIR%web"
  copy /Y web\* "%TARGET_OUTPUT_DIR%web\"
)
