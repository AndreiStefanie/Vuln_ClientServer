@echo off

rmdir /s /q debug
rem rmdir /s /q release
del release\*.exe
del release\*.pdb

attrib -r -s -h *.ncb
attrib -r -s -h *.suo
del *.ncb
del *.suo

rmdir /s /q client\debug
rmdir /s /q client\release

rmdir /s /q server\debug
rmdir /s /q server\release

del client\*.user
del server\*.user