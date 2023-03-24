
@echo off 

winsw_pushersvr.exe stop

if "%errorlevel%" neq "0" (
	echo Service stop failed.
) else (
	echo Service stop success.
)