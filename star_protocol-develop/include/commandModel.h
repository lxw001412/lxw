/**
 * @file commandModel.h
 * @brief  星广播平台命令模型接口
 * @author heyang
 * @version 0.0.1
 * @date 2022-07-04
 */

#pragma once

#include <json/json.h>

 /**
  * @brief  addCommandModel
  *      添加命令模型
  * @param jsonRoot 终端模型JSON对象根节点
  *
  * @return  0:  成功
  *          其它：失败
  */
int addCommandModel(const Json::Value &jsonRoot);

/**
 * @brief  destroyaddCommandModelRepo
 *      释放所有命令模型
 *
 * @return
 */
void destroyCommandModelRepo();