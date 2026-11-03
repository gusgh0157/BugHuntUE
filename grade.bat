@echo off
rem Grade every BugHunt UE lab headless (-nullrhi): PASS / FAIL / TIMEOUT / CRASH / SKIP and a total score.
rem Output: a table on screen, grade-report.json in the project folder, per-lab logs in Saved\Grade\LabNN.log
rem Builds first with build.bat, so close the editor before running.
rem
rem Usage: grade.bat                  all labs, hidden inputs (random seed)
rem        grade.bat -labs=22-28      only some labs (e.g. -labs=1-4,22)
rem        grade.bat -seed=1234       fixed hidden-input seed
rem        grade.bat -public          the public inputs from README instead of hidden ones
rem        grade.bat -nobuild         skip the build step
rem        grade.bat -gpu             run without -nullrhi (also grades labs that need a GPU)
rem        grade.bat -timeout=300     time limit per lab in seconds (default 180)
rem Engine location defaults to F:\Program Files\UE_5.8 ; override with the UE_ROOT environment variable.
setlocal
if "%UE_ROOT%"=="" set "UE_ROOT=F:\Program Files\UE_5.8"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if not exist "%EDITOR_CMD%" (
	echo Cannot find "%EDITOR_CMD%".
	echo Set UE_ROOT to your engine folder, e.g.  set UE_ROOT=D:\Epic\UE_5.8
	exit /b 2
)

echo %* | findstr /i /c:"-nobuild" >nul
if errorlevel 1 (
	call "%~dp0build.bat"
	if errorlevel 1 (
		echo Build failed. If the editor is open, close it and run grade.bat again.
		exit /b 2
	)
)

if exist "%~dp0Saved\Grade\grade-summary.txt" del "%~dp0Saved\Grade\grade-summary.txt"
"%EDITOR_CMD%" "%~dp0BugHuntUE.uproject" -run=BugHuntGrade -nullrhi -nosplash -unattended -AllowStdOutLogVerbosity -LogCmds="global warning" %*
set "GRADE_EXIT=%ERRORLEVEL%"

rem Print the summary table again at the end, away from the engine log lines.
if exist "%~dp0Saved\Grade\grade-summary.txt" (
	powershell -NoProfile -Command "Get-Content -Encoding UTF8 -LiteralPath '%~dp0Saved\Grade\grade-summary.txt'"
)
exit /b %GRADE_EXIT%
