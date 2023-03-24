/**
 * @file streamModInterface.h
 * @brief 推流模块接口
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-12-02
 * @description 推流模块接口
 */

#pragma once

#include "streamMng.h"
#include "streamInfo.h"
#include "fileStream.h"
#include "realTimeStream.h"

class FileStream;
class RealTimeStream;

class StreamModInterface
{
public:
    /**
     * @brief 获取文件流管理对象指针
     *
     * @return 文件流管理对象指针
     */
    static StreamMng<FileStream>* fileStreamMng() {
        return m_fileStreamMng;
    }

    /**
     * @brief 获取实时流管理对象指针
     *
     * @return 实时流管理对象指针
     */
    static StreamMng<RealTimeStream>* realStreamMng() {
        return m_realStreamMng;
    }

    /**
     * @brief 创建流信息指针
     *
     * @return 流信息指针
     *
     * @notes 由调用者delete流信息对象
     */
    static StreamInfo* createStreamInfo();

protected:
    static int init();
    static void fini();
    static int open();
    static void close();
    friend class StreamMod;

private:
    static StreamMng<FileStream> *m_fileStreamMng;
    static StreamMng<RealTimeStream> *m_realStreamMng;
};