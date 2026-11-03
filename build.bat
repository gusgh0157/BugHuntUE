@echo off
rem Build the BugHuntUE editor module from the command line. Close the editor first.
rem Usage: build.bat            -> Development Editor
rem        build.bat DebugGame  -> DebugGame Editor
rem Engine location defaults to F:\Program Files\UE_5.8 ; override with the UE_ROOT environment variable.
setlocal
set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Development"
if "%UE_ROOT%"=="" set "UE_ROOT=F:\Program Files\UE_5.8"

call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" BugHuntUEEditor Win64 %CONFIG% -Project="%~dp0BugHuntUE.uproject" -WaitMutex
exit /b %ERRORLEVEL%
