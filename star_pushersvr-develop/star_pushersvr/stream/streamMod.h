/**
 * @file streamMod.h
 * @brief 推流模块接口
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description 继承服务模块接口
 */

#pragma once

#include "IService.h"

class StreamMod :
    public IService
{
public:
    StreamMod();
    virtual ~StreamMod();

    /**
     * @brief 服务初始化
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int init();


    /**
     * @brief 服务回收，用于在服务对象生命周期结束前调用，
     * 用于释放服务资源
     *
     */
    virtual void finit();

    /**
     * @brief 启动服务
     *
     * @return 0 表示成功，失败时返回其他值
     * @note 函数内不允许阻塞执行
     */
    virtual int open();

    /**
     * @brief 停止服务
     */
    virtual void close();
};

