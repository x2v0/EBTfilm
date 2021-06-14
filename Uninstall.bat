:: $Id: Uninstall.bat 7219 2019-07-29 08:45:56Z onuchin $
:: Author: Valeriy Onuchin  28.07.2019
::

@echo off

:: Uninstall ROOT

:input
if not exist "%ROOTSYS%\Uninstall.exe" (
echo %=% Wrong path to ROOT directory!
set /P ROOTSYS=Enter ROOT path: 
goto input
)

%ROOTSYS%\Uninstall.exe

if exist "%ROOTSYS%" (
rmdir /s /q "%ROOTSYS%"
)

::
RemoveTif.reg


pause