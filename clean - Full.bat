@echo Just clean repo from temporary files
del /S *.bak
del /S *.exp
del /S *_build_log.html
del /S bin\*dll.manifest
del /S bin\*exe.manifest
del /S bin\*.lib
del /S *.pdb
del /S *.ilk

del /S *.VC.opendb
del /S *.VC.db
del /S *.sdf
del /S *.suo
del /S *.psess
del /S *.vsp

if exist "%~dp0.vs" rmdir /S /Q "%~dp0.vs"
