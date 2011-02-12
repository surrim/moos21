del moos21.7z > nul
(copy bin\release\moos21_upx.exe moos21.exe /y && 7z a -t7z -mx9 -x!moos.ini moos21.7z moos21.exe *.ini readme.txt) || pause
del moos21.exe > nul
