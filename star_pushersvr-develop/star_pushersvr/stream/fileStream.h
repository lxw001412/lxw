/**
 * @file fileStream.h
 * @brief 文件推流类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-30
 * @description 文件推流类
 */

#pragma once

#include "baseStream.h"
#include "streamIn.h"
#include "streamOut.h"
#include "streamInfoImp.h"
#include "messageQueue.h"
#include "audioDecoder.h"
#include "audioEncoder.h"
#include <mutex>
#include <thread>

enum FS_CMD
{
    FC_PAUSE = 0,
    FC_RESUME,
    FC_JUMP
};
class FileStreamCmd;

class FileStream : public StreamBase
{
public:
    FileStream();

    virtual ~FileStream();

    /**
     * @brief 启动推流
     * 
     * @param streamInfo 流信息
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int start(const StreamInfo *streamInfo);

    /**
     * @brief 停止推流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int stop(bool reportStop = true);

    /**
     * @brief 获取流信息
     *
     * @return 流信息
     */
    virtual const StreamInfo* info();

    /**
     * @brief 暂停推流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int pause();

    /**
     * @brief 恢复推流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int resume();

	/**
	 * @brief 跳转
	 *
	 * @param index 文件序号
	 * @param pos 时间偏移
	 * @param streamin 新的输入流
	 * @return 0 表示成功，失败时返回其他值
	 */
	virtual int jump(int index, int pos, StreamIn* streamin = nullptr);

	/**
	 * @brief 设置错误码
	 *
	 * @param code 错误码
	 * @param msg 错误信息
	 *
	 */
	virtual void setLastError(int code, const char* msg);

	/**
	 * @brief 更新播放栈
	 *
	 * @param code 需要放入栈顶的文件序号索引
	 *
	 */
	virtual void updatePlayStack(int firstIndex);

	/**
	 * @brief 获取播放栈
	 *
	 */
	virtual MessageQueue<int>* GetMessageQueue();

private:
    // 工作线程
    int svr();
    void setCmd(FS_CMD cmd, int index, int pos,StreamIn* streamin = nullptr);
    FileStreamCmd* getCmd();

    // 检查流信息
    int checkStreamInfo(const StreamInfo *streamInfo);

    // 输入文件检查
    int checkFiles();

    // 上报推流停止
    int reportStop();

    // 上报推流进度
    int reportProcess();

    // 上报文件切换
    int reportFileStart();

    // 初始化推流对象
    int initStreamOut();

    // 生成播放堆栈
    void createPlayStack();

    // 检查持续时长
    bool checkDuration();

    // 推送文件
    int pushFile(const FileInfo_t& fi);

    // 推送帧
    int pushFrame(FrameData_t &fd);

    // 处理命令
    int doCmd(FileStreamCmd* cmd);

    // 创建静音输入流
    StreamIn* createSlienceStreamIn(AudioCodec_t codec, int sampleRate, int channel, int sampleBits);

    // 暂停处理
    void onPause();

    // 初始化解码器
    int initDecoder();

    // 初始化编码器
    int initEncoder();

    // 重新申请解码输出缓存
    int reallocOutputBuff(int size);

private:
    StreamInfoImp m_info;
    FileStreamCmd *m_cmd;
    std::mutex m_cmdLock;
    StreamIn *m_streamIn;
    StreamOut *m_streamOut;
    std::thread m_thread;
    volatile bool m_stop;
    MessageQueue<int> m_playStack;
    int m_cycle;
    AudioCodec_t m_codec;
    int m_channel;
    int m_sampleRate;
    uint64_t m_pauseTime;
    uint64_t m_lastReportProcessTick;
    uint32_t m_streamTimeStamp;
    int m_streamOutErrorTimes;
    bool m_enableReportStop;
    AudioDecoder m_audioDecoder;
    AudioEncoder m_audioEncoder;
    uint8_t *m_outputBuffer;
    uint32_t m_outputBufferSize;

    // 文件跳转偏移
    volatile int m_seekOffset;

    // 流重连参数
    int m_streamReconnectTimes;
    int m_streamReconnectIntervalMilliseconds;
};

// 文件流控制命令
class FileStreamCmd
{
public:
	FileStreamCmd(FS_CMD cmd, int index, int pos, StreamIn* stream = nullptr) :
		m_cmd(cmd), m_index(index), m_pos(pos), m_streamIn(stream) {};
	FS_CMD cmd() const { return m_cmd; };
	int index() const { return m_index; };
	int pos() const { return m_pos; }
	StreamIn* GetStreamIn() { return m_streamIn; }
private:
	FS_CMD m_cmd;
	int m_index;
	int m_pos;
	StreamIn* m_streamIn;
};
#define REPORT_PROCESS_INTERVAL 1