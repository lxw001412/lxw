
@echo off 

winsw_pushersvr.exe start

if "%errorlevel%" neq "0" (
	echo.
	echo Service start failed.
	::将失败结果保存
	echo pushersvrStart=false>>../../installResult.ini
) else (
	echo.
	echo Service start success.
	::将成功结果保存
	echo pushersvrStart=true>>../../installResult.ini
)