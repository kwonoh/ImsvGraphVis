@echo off
cd %~dp0
del /s /q /f *.DS_STORE
del /A *.sln
del /A *.suo
del /A *.sdf
del /A *.opensdf
del /A *.VC.db
for /d /r %%d in (Binaries, Intermediate) do @if exist "%%d" @rmdir /s /q "%%d"
rmdir /S /Q DerivedDataCache
rmdir /S /Q Intermediate
rmdir /S /Q "Saved\Autosaves"
rmdir /S /Q "Saved\Backup"
rmdir /S /Q "Saved\Config"
rmdir /S /Q "Saved\Cooked"
:: rmdir /S /Q "Saved/Logs"
rmdir /S /Q "Saved\StagedBuilds"
rmdir /S /Q ".vs"
rmdir /S /Q "x64"
rmdir /S /Q "Debug"
