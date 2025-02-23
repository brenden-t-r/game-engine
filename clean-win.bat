@echo off

for /d %%D in (cmake-build-*) do rmdir /s "%%D"
