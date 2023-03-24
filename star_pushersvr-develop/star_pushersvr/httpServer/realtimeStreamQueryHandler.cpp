#include "realtimeStreamQueryHandler.h"
#include "streamModInterface.h"
#include "httpServerError.h"

void realtimeStreamQueryHandler::post(const Json::Value & jsonReq, Json::Value & jsonResp)
{
    Json::Value recv = jsonReq;
    Json::Value send;
    std::vector<std::string> id_array;

    if (recv.isMember("id") && recv["id"].isArray())
    {
        for (unsigned int i = 0; i < recv["id"].size(); i++)
        {
            id_array.push_back(recv["id"][i].asString());
        }
    }
    else
    {
        send["code"] = PUSHERSVR_ERROR_PARAMETER;
        send["msg"] = "param error";
        send["data"] = { "stream id error " };

        jsonResp = send;
        return;
    }

    std::vector<const StreamInfo*> streamInfoFinal;

    //若id_array为空，返回全部的已有的流信息
    if (id_array.empty())
    {
        //取出现有的流指针 
        std::map<std::string, std::shared_ptr<StreamBase>> allStream;

        int res = StreamModInterface::realStreamMng()->getAllStream(allStream);

        if (res != 0)
        {
            send["code"] = PUSHERSVR_ERROR_STREAM_NOT_EXIST;
            send["msg"] = "fail";
            send["data"] = { "stream is not exist" };

            jsonResp = send;
            return;
        }
        //获取所有流之后，取出所有流信息的指针

        std::vector<const StreamInfo*> allStreamInfo;
        std::map<std::string, std::shared_ptr<StreamBase>>::iterator map_it = allStream.begin();
        for (; map_it != allStream.end(); map_it++)
        {
            allStreamInfo.push_back(map_it->second->info());
        }

        streamInfoFinal = allStreamInfo;

    }
    else //若id_array不为空，返回指定的的流信息
    {
        std::vector<std::shared_ptr<StreamBase>> stream_ptr;
        std::vector<std::string>::iterator it_id = id_array.begin();
        std::shared_ptr<StreamBase> temp_ptr;

        for (; it_id != id_array.end(); it_id++)
        {
            temp_ptr = StreamModInterface::realStreamMng()->getStream((*it_id));
            if (temp_ptr != NULL)
            {
                stream_ptr.push_back(temp_ptr);
            }
            ;
        }

        std::vector<const StreamInfo*> streamInfo;
        std::vector<std::shared_ptr<StreamBase>>::iterator it_stream_ptr = stream_ptr.begin();
        for (; it_stream_ptr != stream_ptr.end(); it_stream_ptr++)
        {
            streamInfo.push_back((*it_stream_ptr)->info());
        }

        streamInfoFinal = streamInfo;
    }

    //通过取流信息指针，取数据到json
    Json::Value temp_stream;
    std::vector<const StreamInfo*>::iterator it_streamInfo = streamInfoFinal.begin();
    int i = 0;

    for (; it_streamInfo != streamInfoFinal.end(); it_streamInfo++, i++)
    {
        temp_stream[i]["id"] = (*it_streamInfo)->id();
        temp_stream[i]["url"] = (*it_streamInfo)->url();
        temp_stream[i]["source"] = (*it_streamInfo)->source();
        temp_stream[i]["duration"] = (*it_streamInfo)->duration();
        temp_stream[i]["startTime"] = (*it_streamInfo)->startTime();
        temp_stream[i]["playSeconds"] = (*it_streamInfo)->playSeconds();
        temp_stream[i]["stopTime"] = (*it_streamInfo)->stopTime();
        if ((*it_streamInfo)->status() == 1)
        {
            temp_stream[i]["status"] = "running";
        }
        if ((*it_streamInfo)->status() == 2)
        {
            temp_stream[i]["status"] = "paused";
        }
        if ((*it_streamInfo)->status() == 3)
        {
            temp_stream[i]["status"] = "stopped";
        }
        temp_stream[i]["errorCode"] = (*it_streamInfo)->lastErrorCode();
        temp_stream[i]["errorMsg"] = (*it_streamInfo)->lastErrorMsg();
    }

    send["code"] = PUSHERSVR_ERROR_OK;
    send["msg"] = "success";
    send["data"]["streams"] = temp_stream;

    jsonResp = send;
    return;


}
