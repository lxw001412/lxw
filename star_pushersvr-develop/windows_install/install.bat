@echo off 

::安装服务
winsw_pushersvr.exe install

if "%errorlevel%" neq "0" (
	echo Service install failed.
	::将失败结果保存
	echo pushersvrInstall=false>>../../installResult.ini
) else (
	echo Service install success.
	::将成功结果保存
	echo pushersvrInstall=true>>../../installResult.ini	
)

