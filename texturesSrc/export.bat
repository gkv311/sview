@echo off

set "ink=inkscape.com"
set "anSvgName=%1"
set "aPngName=%2"
if ["%aPngName%"] == [""] set "aPngName=%1"
if not exist "%anSvgName%" set "anSvgName=%anSvgName%.svg"

for %%s in (16 24 32 48 64 72 96 128 144 192 256) do (
  %ink% --file %anSvgName% --export-area-page --export-height %%s --export-png %aPngName%%%s.png
)
