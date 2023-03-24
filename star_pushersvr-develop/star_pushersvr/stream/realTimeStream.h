#pragma once
#include "baseStream.h"
#include "streamInfoImp.h"
#include "streamIn.h"
#include "streamOut.h"
#include "audioDecoder.h"
#include "audioEncoder.h"

#include <thread>

class RealTimeStream : public StreamBase
{
public:
	RealTimeStream();
	virtual ~RealTimeStream();

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
	virtual int stop(bool reportStop = true) ;


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
	virtual void updatePlayStack(int firstIndex) { return; };

	/**
	 * @brief 获取播放栈
	 *
	 */
	virtual MessageQueue<int>* GetMessageQueue() { return nullptr; };
private:
	// 工作线程
	int svr();

	// 初始化输入、输出流
	bool initStream();

	// 释放输入、输出流
	void finiStream();

	// 拉取并推送媒体帧
	int pullAndpushFrame();

	// 是否到达播放时长
	bool isPlayOver();

	// 上报推流停止
	int reportStop();

	// 上报推流进度
	int reportProcess();

private:
	int initStreamOut();

    // 初始化解码器
    int initDecoder();

    // 初始化编码器
    int initEncoder();

    // 重新申请解码输出缓存
    int reallocOutputBuff(int size);

    // 推送帧
    int pushFrame(FrameData_t &fd);

    // 重连输入流
    int reconnectStreamIn(FrameData_t &fd);

private:
	StreamInfoImp m_info;
	StreamIn *m_streamIn;
	StreamOut *m_streamOut;
	std::thread m_thread;
	AudioCodec_t m_codec;
	int m_channel;
	int m_sampleRate;
	int m_streamOutErrorTimes;
	uint64_t m_pauseTime;
	uint64_t m_lastReportProcessTick;
	volatile bool m_stop;
    bool m_enableReportStop;
    AudioDecoder m_audioDecoder;
    AudioEncoder m_audioEncoder;
    uint8_t *m_outputBuffer;
    uint32_t m_outputBufferSize;

    // 流重连参数
    int m_streamReconnectTimes;
    int m_streamReconnectIntervalMilliseconds;
};