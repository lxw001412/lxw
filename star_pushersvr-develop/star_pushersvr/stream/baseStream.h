/**
 * @file baseStream.h
 * @brief 推流类接口
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-30
 * @description 推流类接口
 */

#pragma once

#include <string>
#include <stdint.h>
#include "streamInfo.h"
#include "messageQueue.h"
class StreamIn;
class StreamBase
{
public:
    virtual ~StreamBase() {};

    /**
     * @brief 启动推流
     * 
     * @param streamInfo 流信息
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int start(const StreamInfo *streamInfo) = 0;

    /**
     * @brief 停止推流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int stop(bool reportStop = true) = 0;

    /**
     * @brief 获取流信息
     *
     * @return 流信息
     */
    virtual const StreamInfo* info() = 0;

    /**
     * @brief 暂停推流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int pause() = 0;

    /**
     * @brief 恢复推流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int resume() = 0;


	/**
	 * @brief 跳转
	 *
	 * @param index 文件序号
	 * @param pos 时间偏移
	 * @param streamin 新的输入流
	 * @return 0 表示成功，失败时返回其他值
	 */
	virtual int jump(int index, int pos, StreamIn* streamin = nullptr) = 0;


	/**
	 * @brief 设置错误码
	 *
	 * @param code 错误码
	 * @param msg 错误信息
	 *
	 */
	virtual void setLastError(int code, const char* msg) = 0;


	/**
	 * @brief 更新播放栈
	 *
	 * @param code 需要放入栈顶的文件序号索引
	 *
	 */
	virtual void updatePlayStack(int firstIndex) = 0;


	/**
	 * @brief 获取播放栈
	 *
	 */
	virtual MessageQueue<int>* GetMessageQueue() = 0;
};
