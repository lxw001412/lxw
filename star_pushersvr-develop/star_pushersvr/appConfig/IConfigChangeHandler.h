
/**
*
* @file 	IConfigChangeHandler.h
* @author	黄晨阳
* @date		2021.11.25
* @brief
* 该类为回调函数接口类，用于当配置文件数据变化时的回调。所有模块都需要继承并且实现回调函数。
*
**/
#pragma once
class IConfigChangeHandler
{
public:
	IConfigChangeHandler() {};
	~IConfigChangeHandler() {};

    /**
     * @brief  回调函数，当配置文件有变化时，配置模块会对用对应模块的回调函数
     */
	virtual void handleConfigChange() = 0;

private:
    
};
