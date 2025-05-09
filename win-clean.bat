@echo off

for /d %%D in (cmake-build-*) do rmdir /s "%%D"
for /d %%D in (build*) do rmdir /s "%%D"
