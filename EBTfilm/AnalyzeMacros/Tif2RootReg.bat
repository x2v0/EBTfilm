:: $Id: Tif2RootReg.bat 7219 2019-07-29 08:45:56Z onuchin $
:: Author: Valeriy Onuchin  16.12.2018
::

@echo off

::if not "%ROOTSYS%"=="" goto build

:: define ROOTSYS enviromental variable
:input
set INPUT=%~nx1

if "%INPUT%"=="" (
	set /P INPUT=Enter ROOT path. Press "Enter" to use the current directory: 
	if "%INPUT%"=="" (
	set ROOTSYS="%CD%\.."
	goto setreg
	)
)

set ROOTSYS=%INPUT%

:setreg
echo ROOTSYS=%ROOTSYS%
pause

if not exist "%ROOTSYS%\bin\root.exe" (
echo %=% Wrong path to ROOT directory!
goto input
)


pause

reg add "HKCU\Environment" /v ROOTSYS /t REG_SZ /d "%ROOTSYS%"
::reg add "HKCU\Environment" /v Path /t REG_SZ /d "%PATH%;%ROOTSYS%\bin"

reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v ROOTSYS /t REG_SZ /d "%ROOTSYS%"
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_SZ /d "%PATH%;%ROOTSYS%\bin"


:: building ...
:build

reg add HKLM\SOFTWARE\Classes\.tif /v "" /t REG_SZ /d "Tif2Root" /f
reg add HKLM\SOFTWARE\Classes\Tif2Root\shell\open\command  /v "" /t REG_SZ /d "%ROOTSYS%\macros\Tif2Root.bat %%1" /f

reg add HKLM\SOFTWARE\Classes\.tif /v "" /t REG_SZ /d "Tif2Root" /f
reg add HKLM\SOFTWARE\Classes\Tif2Root\shell\open\command  /v "" /t REG_SZ /d "%ROOTSYS%\macros\Tif2Root.bat %%1" /f

reg add HKCU\SOFTWARE\Classes\.tif /v "" /t REG_SZ /d "Tif2Root" /f
reg add HKCU\SOFTWARE\Classes\Tif2Root\shell\open\command  /v "" /t REG_SZ /d "%ROOTSYS%\macros\Tif2Root.bat %%1" /f
