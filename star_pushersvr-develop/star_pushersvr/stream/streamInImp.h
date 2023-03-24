/**
 * @file streamInImp.h
 * @brief 流读取实现类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description 流读取实现类
 */

#include "streamIn.h"
#include "AAC_ADTS.h"
#include <string>

 //ffmpeg
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h> 
#include <libswscale/swscale.h> 
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
};

typedef enum _StreamProtocol
{
    SP_FILE = 0,
    SP_RTMP,
    SP_RTSP,
    SP_RTP,
    SP_HTTP,
    SP_UDP
}StreamProtocol_t;

class StreamInImp : public StreamIn
{
public:
    StreamInImp();

    virtual ~StreamInImp();

    /**
     * @brief 打开流
     *
     * @param file 文件路径或流地址
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int open(const std::string &id, const std::string &file);

    /**
     * @brief 关闭流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int close();

    /**
     * @brief 读取流
     *
     * @param data 帧数据
     *
     * @return 0 表示成功，失败时返回其他值
     *
     * @notes frameData中的帧数据的有效期为下一次调用read函数前
     */
    virtual int readData(FrameData_t &frameData);

    /**
     * @brief 文件类型流：跳转到指定偏移
     *
     * @param posMs 偏移时长，单位毫秒
     *
     * @return 0 表示成功，失败时返回其他值
     *
     * @notes
     */
    virtual int seekFile(uint64_t posMs);

    /**
     * @brief 返回当前播放时间点
     *
     * @param
     *
     * @return 当前播放时间点，单位毫秒
     *
     * @notes
     */
    virtual uint64_t current();

    /**
     * @brief 返回总时长
     *
     * @param
     *
     * @return 总时长，单位毫秒
     *
     * @notes 非文件流返回0
     */
    virtual uint64_t total();

    /**
     * @brief 返回采用率
     *
     * @param
     *
     * @return 采用率
     *
     * @notes
     */
    virtual int sampleRate();

    /**
     * @brief 比特率
     *
     * @param
     *
     * @return 比特率
     *
     * @notes
     */
    virtual int bitrate();

    /**
     * @brief 返回声道数
     *
     * @param
     *
     * @return 声道数
     *
     * @notes
     */
    virtual int channel();

    /**
     * @brief 返回音频编码格式
     *
     * @param
     *
     * @return 音频编码格式
     *
     * @notes
     */
    virtual AudioCodec_t audioCodec();

    /**
     * @brief 返回ffmpeg音频编码格式
     *
     * @param
     *
     * @return 音频编码格式
     *
     * @notes
     */
    virtual int audioAvCodec();

    /**
     * @brief 暂停
     *
     * @notes
     */
    virtual void pause();

    /**
     * @brief 恢复
     *
     * @notes
     */
    virtual void resume();

    /**
     * @brief 等待最后一帧时长
     *
     * @notes
     */
    virtual void lastFrameDelay();

protected:
    static int statiCallBack(void *p);
    //回调函数
    int callback(void);

    int readAvPacket(AVPacket *pkt);
    int audiotime(AVPacket *pkt);

private:
    std::string m_id;
    std::string m_file;
    AAC_ADTS *m_adts;
    AVFormatContext *m_ifmtCtx;
    int m_audioIndex;
    int m_videoIndex;
    int64_t m_duration;         // 文件持续时间, 单位 微秒
    uint64_t m_starttime;       // 开始时间戳，单位 微秒
    time_t m_lastFrameTick;     // 最近帧时间戳，单位 毫秒
    uint64_t m_audioCurrent;   // 已播放时长，单位 微秒
    uint64_t m_frameDuration;   // 帧时长，单位 微秒
    std::string m_lastError;
    AudioCodec_t m_audioCodec;
    int m_audioAvCodec;
    int m_sampleRate;
    int m_channel;
    int m_bitrate;
    bool m_stop;
    StreamProtocol_t m_sp;
    uint8_t *m_frameBuffer;
    int m_frameBufferLength;

    AVCodecContext *m_avctx;    // 解码
    int64_t m_pauseTime;
    int64_t m_pauseTick;
    volatile bool m_seekedFile;

    int64_t m_startPtsTime;     // 起始帧时间戳
    volatile uint64_t m_interruptStartTickCount;     // 超时计时器起始时间
    uint64_t m_streamInTimeout; // 输入流超时时长
};