/**
*
* @file 	AppConfig.h
* @author	黄晨阳
* @date		2021.11.25
* @brief
* 单例
* 该类为配置模块类，实现了需求中配置模块的功能。
*
**/
#pragma once
#include <iostream>
#include <vector>
#include "IConfigChangeHandler.h"
#include <json/json.h>
#include <ace/Singleton.h>
#include <ace/Null_Mutex.h>

//#define filePath "../Conf/brdcastConfig_test.json"

class AppConfig
{
public:

	~AppConfig();
	AppConfig();

    /**
    * @brief  单例返回
    *
    * @return 返回值为单例指针
    */
	static AppConfig *instance() ;

    /**
    * @brief  模块关闭
    */
	static void Destory();

    /**
    * @brief  配置文件路径载入
    *
    * @param data 入参：
    *                  path ：配置文件路径 类型：string
    *
    * @return 返回值不为0，为失败
    *
    * @note 传入配置文件路径，解析文件成json
    */
	int load(const std::string &path);

    /**
    * @brief  数据保存
    *
    * @return 返回值不为0，为失败
    *
    * @note 读写操作完之后调用，内部实现回调
    */
	int save();

    /**
    * @brief  获取配置文件中string类型数据
    *
    *  @param data in：
    *                  key ：数据的key 类型：string
    *                  mod ：模块名称 类型：string
    *              out：
    *                  value：获取的值 类型：string
    * @return 返回值若不为0，为失败
    *         返回-1，为不存在这个key
    *         返回-2，为不存在这个mod
    */
	int getStringParam(const std::string &mod,const std::string &key, std::string & value);

    /**
    * @brief  获取配置文件中int类型数据
    *
    *  @param data in：
    *                  key ：数据的key 类型：string
    *                  mod ：模块名称 类型：string
    *               out：
    *                  value：获取的值 类型：int
    * @return 返回值若不为0，为失败
    *         返回-1，为不存在这个key
    *         返回-2，为不存在这个mod
    */
	int getIntParam(const std::string &mod, const std::string &key ,int &value);

    /**
    * @brief  获取配置文件中bool类型数据
    *
    *  @param data in：
    *                  key ：数据的key 类型：string
    *                  mod ：模块名称 类型：string
    *               out：
    *                  value：获取的值 类型：bool
    * @return 返回值若不为0，为失败
    *         返回-1，为不存在这个key
    *         返回-2，为不存在这个mod
    */
    int getBoolParam(const std::string &mod, const std::string &key,bool &value);

   /**
   * @brief  获取配置文件中vector<std::string>类型数据
   *
   *  @param data in：
   *                  key ：数据的key 类型：string
   *                  mod ：模块名称 类型：string
   *               out：
   *                  value：获取的值 类型：vector<std::string>
   * @return 返回值若不为0，为失败
   *         返回-1，为不存在这个key
   *         返回-2，为不存在这个mod
   *         返回-3，其他错误情况
   */
	int getStringVecParam(const std::string &mod, const std::string &key, std::vector<std::string> &value);

   /**
   * @brief  设置string类型数据
   *
   *  @param data 入参：
   *                  key ：数据的key 类型：string
   *                  mod ：模块名称 类型：string
   *                  value ：将要设置的数据  类型：string
   * @return 返回值若不为0，为失败
   */
	int setParam(const std::string &mod, const std::string &key, const std::string &value);

    /**
    * @brief  设置int类型数据
    *
    *  @param data 入参：
    *                  key ：数据的key 类型：string
    *                  mod ：模块名称 类型：string
    *                  value ：将要设置的数据  类型：int
    * @return 返回值若不为0，为失败
    */
	int setParam(const std::string &mod, const std::string &key, const int value);

    /**
    * @brief  设置bool类型数据
    *
    *  @param data 入参：
    *                  key ：数据的key 类型：string
    *                  mod ：模块名称 类型：string
    *                  value ：将要设置的数据  类型：bool
    * @return 返回值若不为0，为失败
    */
	int setParam(const std::string &mod, const std::string &key, const bool value);

    /**
    * @brief  设置vector<std::string>类型数据
    *
    *  @param data 入参：
    *                  key ：数据的key 类型：string
    *                  mod ：模块名称 类型：string
    *                  value ：将要设置的数据  类型：vector<std::string>
    * @return 返回值若不为0，为失败
    */
	int setParam(const std::string &mod, const std::string &key, const std::vector<std::string> &value);

    /**
   * @brief  回调函数注册
   */
	void registryHandler(IConfigChangeHandler *handler); 

    /**
   * @brief  回调函数注销
   */
	void unregistryHandler(IConfigChangeHandler *handler);

private:

    /**
   * @brief  调用回调函数
   */
	void notifyUpdate();
	friend class ACE_Singleton<AppConfig, ACE_Null_Mutex>;

private:
	std::string m_filePath;//用于保存配置文件路径
	std::vector<IConfigChangeHandler*> m_handles;//存储回调函数
	Json::Value m_json;//配置文件解析成json暂存

};
