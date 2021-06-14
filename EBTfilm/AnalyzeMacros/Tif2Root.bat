@echo off

if exist rootlogon.C (
ren rootlogon.C rootlogon.C.sav
)

set ROOT=
for /f "tokens=2,*" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v "ROOTSYS" ^| findstr "ROOTSYS"') do (
    set ROOT=%%b
)

echo ROOT=%ROOT%
::pause
%ROOT%\bin\root.exe -l doseHistA.C(\"%~nx1\")"


if exist rootlogon.C.sav (
ren rootlogon.C.sav "rootlogon.C
)