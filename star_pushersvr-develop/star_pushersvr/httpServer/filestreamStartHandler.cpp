#include "filestreamStartHandler.h"
#include "streamModInterface.h"
#include "httpServerError.h"
#include "spdlogging.h"
#include "tickcount.h"
#include "AppConfig.h"
#include <map>
#include <iostream>
#include <memory>

void filestreamStartHandler::post(const Json::Value & jsonReq, Json::Value & jsonResp)
{
    std::string id;
    std::string url;
    std::map< int , FileInfo_t> temp_map;
    FileInfo_t fileInfo;
    FileInfoList_t fileList;
    int duration = 0;
    int cycle = 0;
    int mode = 1;

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
        jsonResp["msg"] = "stream id is not exist";
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
    if (AppConfig::instance()->getStringParam("callback", "filestart", urlTemp) == 0)
    {
        stream_info->fileStartCallbackUrl(urlTemp);
    }
    if (AppConfig::instance()->getStringParam("callback", "stop", urlTemp) == 0)
    {
        stream_info->stopCallbackUrl(urlTemp);
    }


    if (jsonReq.isMember("enableProcessCb") && jsonReq["enableProcessCb"].isBool())
    {
        stream_info->enableProcessCallback(jsonReq["enableProcessCb"].asBool());
    }
    if (jsonReq.isMember("enableFileStartCb") && jsonReq["enableFileStartCb"].isBool())
    {
        stream_info->enableFileStartCallback(jsonReq["enableFileStartCb"].asBool());
    }
    if (jsonReq.isMember("enableStopCb") && jsonReq["enableStopCb"].isBool())
    {
        stream_info->enableStopCallback(jsonReq["enableStopCb"].asBool());
    }
    if (jsonReq.isMember("processCbUrl") && jsonReq["processCbUrl"].isString())
    {
        stream_info->processCallbackUrl(jsonReq["processCbUrl"].asString());
    }
    if (jsonReq.isMember("fileStarCbUrl") && jsonReq["fileStarCbUrl"].isString())
    {
        stream_info->fileStartCallbackUrl(jsonReq["fileStarCbUrl"].asString());
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

    if (jsonReq.isMember("files") && jsonReq["files"].isArray())
    {
        for (unsigned int i = 0 ;i < jsonReq["files"].size();i++)
        {
            Json::Value files = jsonReq["files"][i];
            if (files.isMember("path") && files["path"].isString())
            {
                fileInfo.path = files["path"].asString();
            }
            else
            {
                jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
                jsonResp["msg"] = "file path is not exist";
                return;
            }

            if (files.isMember("index") && files["index"].isInt())
            {
                fileInfo.index = files["index"].asInt();
            }
            else
            {
                jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
                jsonResp["msg"] = "file index is not exist";                
                return;
            }

            if (files.isMember("id") && files["id"].isString())
            {
                fileInfo.id = files["id"].asString();
            }
            else
            {
                jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
                jsonResp["msg"] = "file id is not exist";
                return;
            }
            //将元素先插入到map中，之后再排序
            temp_map.insert(std::map<int, FileInfo_t>::value_type(fileInfo.index, fileInfo));
        }

        //遍历map，加入到vector
        std::map< int, FileInfo_t> ::iterator iter;
        for (iter = temp_map.begin(); iter != temp_map.end(); iter++)
        {
           fileList.push_back((*iter).second);
        }
    } 
    else
    {
        jsonResp["code"] = PUSHERSVR_ERROR_PARAMETER;
        jsonResp["msg"] = "param error: no file";
        return;
    }

    if (jsonReq.isMember("duration") && jsonReq["duration"].isInt())
    {
        duration = jsonReq["duration"].asInt();
    }
 
    if (jsonReq.isMember("cycle") && jsonReq["cycle"].isInt())
    {
        cycle = jsonReq["cycle"].asInt();
    }

    if (jsonReq.isMember("mode") && jsonReq["mode"].isInt())
    {
        mode = jsonReq["mode"].asInt();
    }

    stream_info->id(id);
    stream_info->url(url);
    stream_info->fileList(fileList);
    stream_info->duration(duration);
    stream_info->cycle(cycle);
    stream_info->mode((FileMode_t)mode);

    std::shared_ptr<StreamBase> ps = StreamModInterface::fileStreamMng()->getStream(id);
    if (ps != NULL)
    {
        ps->stop(false);
        ps = NULL;
    }

    ps = StreamModInterface::fileStreamMng()->createStream(id);
    if (ps == NULL)
    {
        jsonResp["code"] = PUSHERSVR_ERROR_SYSTEM;
        jsonResp["msg"] = "system error: no memory!";
        return;
    }

    uint64_t startTick = GetMillisecondCounter();
    int resp_start = ps->start(stream_info.get());
    SPDINFO("[http] start file stream {} , duration: {} ms", id, GetMillisecondCounter() - startTick);

    if (resp_start != 0)
    {
        jsonResp["code"] = PUSHERSVR_ERROR_SYSTEM;
        jsonResp["msg"] = "start file stream error";        
        return;
    }
    jsonResp["code"] = PUSHERSVR_ERROR_OK;
    jsonResp["msg"] = "success";    
    return;
}
