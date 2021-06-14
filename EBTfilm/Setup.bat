:: $Id: $
:: Author: Valeriy Onuchin  28.07.2019
::

@echo off

:: Setup ROOT
root_v5.34.32.win32.vc10.exe
set ROOTSYS="c:\root_v5.34.32"

if not exist "%ROOTSYS%\bin\root.exe" (
set /P ROOTSYS=Enter ROOT path: 
)

:input
if not exist "%ROOTSYS%\bin\root.exe" (
echo %=% Wrong path to ROOT directory!
set /P ROOTSYS=Enter ROOT path: 
goto input
)

echo ROOTSYS=%ROOTSYS%

reg add "HKCU\Environment" /v ROOTSYS /t REG_SZ /d "%ROOTSYS%"
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v ROOTSYS /t REG_SZ /d "%ROOTSYS%"

xcopy /e "AnalyzeMacros" "%ROOTSYS%\macros\"
Tif2RootReg.bat "%ROOTSYS%"

pause