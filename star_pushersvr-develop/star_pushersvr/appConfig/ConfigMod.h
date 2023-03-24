/**
*
* @file 	ConfigMod.h
* @author	黄晨阳
* @date		2021.11.25
* @brief
* 该类为模块接口类，继承IService，用于配置模块的初始化和关闭。
*
**/

#pragma once

#include "IService.h"
#include <iostream>

class ConfigMod : public IService
{
public:
    ConfigMod() : IService("Config module") {}; 

     /**
     * @brief  初始化函数
     *
     * @param data 入参： 
     *                  val ：配置文件路径 类型：string
     *
     * @return 返回值不为0，为失败
     */
	int init(const std::string &val);

    /**
    * @brief  模块关闭
    */
	virtual void finit();

	virtual int init() { return 0; }
	virtual int open() { return 0; };
	virtual void close() { };
};