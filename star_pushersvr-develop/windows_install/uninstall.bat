@echo off 

winsw_pushersvr.exe uninstall

if "%errorlevel%" neq "0" (
	echo.
	echo Service uninstall failed.
) else (
	echo.
	echo Service uninstall success.	
)

