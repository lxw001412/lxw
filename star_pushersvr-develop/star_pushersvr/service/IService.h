/**
 * @file IService.h
 * @brief 服务接口定义
 * @author Robin.L<luoxuan@comtom.cn>
 * @version 0.1
 * @date 2021-11-23
 * @description 系统中所有服务继承自该接口，进行统一管理
 */
#pragma once

#include <string>

/**
 * @brief 服务接口 
 */
class IService
{
public:
    IService(const std::string &name) : m_svrName(name) {};

    virtual ~IService() {};
    /**
     * @brief 服务初始化
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int init() { return 0; }


    /**
     * @brief 服务回收，用于在服务对象生命周期结束前调用，
     * 用于释放服务资源
     *
     */
    virtual void finit() { };

    /**
     * @brief 启动服务
     *
     * @return 0 表示成功，失败时返回其他值
     * @note 函数内不允许阻塞执行
     */
    virtual int open() = 0;

    /**
     * @brief 停止服务
     */
    virtual void close() = 0;

    /**
     * @brief 服务名称
     *
     * @return 服务名称
     */
    virtual const std::string& name() { return m_svrName; };

protected:
    std::string m_svrName;
};
