/**
 * @file productModel.h
 * @brief  星广播平台终端模型接口
 * @author Bill
 * @version 0.0.1
 * @date 2021-05-31
 */

#pragma once

#include <json/json.h>

/**
 * @brief  addProductModel 
 *      添加终端模型 
 * @param jsonRoot 终端模型JSON对象根节点
 *
 * @return  0:  成功
 *          其它：失败
 */
int addProductModel(const Json::Value &jsonRoot);

/**
 * @brief  destroyProductModelRepo
 *      释放所有终端模型
 *
 * @return  
 */
void destroyProductModelRepo();