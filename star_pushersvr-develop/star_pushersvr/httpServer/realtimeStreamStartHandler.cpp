#include "realtimeStreamStartHandler.h"
#include "streamModInterface.h"
#include "httpServerError.h"
#include "spdlogging.h"
#include "tickcount.h"
#include "AppConfig.h"
#include <iostream>
#include <memory>

void realtimeStreamStartHandler::post(const Json::Value & jsonReq, Json::Value & jsonResp)
{
    std::string id;
    std::string url;
    bool enableProcessCb = false;
    std::string source;
    int duration = 0;

    std::shared_ptr<StreamInfo> stream_info = std::shared_ptr<StreamInfo>(StreamModInterface::createStreamInfo());
    if (stream_info == NULL)
    {
        jsonResp["code"] = PUSHERSVR_ERROR_SYSTEM;
        jsonResp["msg"] = "system error, no memory!";
        return;
    }

    if (jsonReq.isMember("id") && jsonReq["id"].isString())
    {
        id = jsonReq["id"].asString();
    }
    else
    {
        jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
        jsonResp["msg"] = "id is not exist";        
        return;
    }

    if (jsonReq.isMember("url") && jsonReq["url"].isString())
    {
        url = jsonReq["url"].asString();
    }
    else
    {
        jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
        jsonResp["msg"] = "url is not exist";        
        return;
    }

    std::string urlTemp;
    if (AppConfig::instance()->getStringParam("callback", "process", urlTemp) == 0)
    {
        stream_info->processCallbackUrl(urlTemp);
    }
    if (AppConfig::instance()->getStringParam("callback", "stop", urlTemp) == 0)
    {
        stream_info->stopCallbackUrl(urlTemp);
    }

    if (jsonReq.isMember("enableProcessCb") && jsonReq["enableProcessCb"].isBool())
    {
        stream_info->enableProcessCallback(jsonReq["enableProcessCb"].asBool());
    }
    if (jsonReq.isMember("enableStopCb") && jsonReq["enableStopCb"].isBool())
    {
        stream_info->enableStopCallback(jsonReq["enableStopCb"].asBool());
    }
    if (jsonReq.isMember("processCbUrl") && jsonReq["processCbUrl"].isString())
    {
        stream_info->processCallbackUrl(jsonReq["processCbUrl"].asString());
    }
    if (jsonReq.isMember("stopCbUrl") && jsonReq["stopCbUrl"].isString())
    {
        stream_info->stopCallbackUrl(jsonReq["stopCbUrl"].asString());
    }

    if (jsonReq.isMember("audioConvert") && jsonReq["audioConvert"].isObject())
    {
        AudioConvertArgs_t audioConvertArgs;

        audioConvertArgs.enable = false;
        audioConvertArgs.codec = AC_UNSUPPORT;
        audioConvertArgs.sampleRate = 0;
        audioConvertArgs.bitrate = 0;
        audioConvertArgs.channelNum = 0;

        const Json::Value &audioConvert = jsonReq["audioConvert"];
        if (audioConvert.isMember("enable") && audioConvert["enable"].isBool())
        {
            audioConvertArgs.enable = audioConvert["enable"].asBool();
        }

        if (audioConvertArgs.enable)
        {
            if (audioConvert.isMember("codec") && audioConvert["codec"].isString())
            {
                std::string codec = audioConvert["codec"].asString();
                if (codec == "mp3")
                {
                    audioConvertArgs.codec = AC_MP3;
                }
                else if (codec == "aac")
                {
                    audioConvertArgs.codec = AC_AAC;
                }
                else
                {
                    jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
                    jsonResp["msg"] = "param error: unsupport codec";
                    return;
                }
            }
            if (audioConvert.isMember("ar") && audioConvert["ar"].isInt())
            {
                audioConvertArgs.sampleRate = audioConvert["ar"].asInt();
            }
            if (audioConvert.isMember("ab") && audioConvert["ab"].isInt())
            {
                audioConvertArgs.bitrate = audioConvert["ab"].asInt() * 1000;
            }
            if (audioConvert.isMember("ac") && audioConvert["ac"].isInt())
            {
                audioConvertArgs.channelNum = audioConvert["ac"].asInt();
            }
            if (audioConvertArgs.codec == AC_UNSUPPORT
                || audioConvertArgs.sampleRate == 0
                || audioConvertArgs.bitrate == 0
                || audioConvertArgs.channelNum <= 0
                || audioConvertArgs.channelNum > 2)
            {
                jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
                jsonResp["msg"] = "param error: audioConvert parameter error";
                return;
            }
            stream_info->audioConvertParam(audioConvertArgs);
        }
    }

    if (jsonReq.isMember("source") && jsonReq["source"].isString())
    {
        source = jsonReq["source"].asString();
    }
    else
    {
        jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
        jsonResp["msg"] = "source is not exist";
        return;
    }

    if (jsonReq.isMember("duration") && jsonReq["duration"].isInt())
    {
        duration = jsonReq["duration"].asInt();
    }

    stream_info->id(id);
    stream_info->url(url);
    stream_info->enableProcessCallback(enableProcessCb);
    stream_info->source(source);
    stream_info->duration(duration);

    std::shared_ptr<StreamBase> rts = StreamModInterface::realStreamMng()->getStream(id);
    if (rts != NULL)
    {
        rts->stop(false);
        rts = NULL;
    }

    rts = StreamModInterface::realStreamMng()->createStream(id);
    if (rts == NULL)
    {
        jsonResp["code"] = PUSHERSVR_ERROR_SYSTEM;
        jsonResp["msg"] = "system error, no memory!";
        return;
    }

    uint64_t startTick = GetMillisecondCounter();
    int resp_start = rts->start(stream_info.get());
    SPDINFO("[http] start realtime stream {} , duration: {} ms", id, GetMillisecondCounter() - startTick);

    if (resp_start != 0)
    {
        jsonResp["code"] = PUSHERSVR_ERROR_SYSTEM;
        jsonResp["msg"] = "start realtime stream error";        
        return;
    }
    jsonResp["code"] = PUSHERSVR_ERROR_OK;
    jsonResp["msg"] = "success";    
    return;
}
