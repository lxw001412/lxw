#include "fileStreamStopHandler.h"
#include "streamModInterface.h"
#include "httpServerError.h"
#include "spdlogging.h"
#include "tickcount.h"

void fileStreamStopHandler::post(const Json::Value & jsonReq, Json::Value & jsonResp)
{
    Json::Value recv = jsonReq;
    Json::Value send;

    std::string id;
    bool disableStopCb = false;

    if (recv.isMember("id") && recv["id"].isString())
    {
        id = recv["id"].asString();
    }
    else
    {
        send["code"] = PUSHERSVR_ERROR_PARAMETER;
        send["msg"] = "param error";
        send["data"] = { "stream id error " };

        jsonResp = send;
        return;
    }
    if (recv.isMember("disableStopCb") && recv["disableStopCb"].isBool())
    {
        disableStopCb = recv["disableStopCb"].asBool();
    }

    std::shared_ptr<StreamBase> temp = StreamModInterface::fileStreamMng()->getStream(id);
    if (temp == NULL)
    {
        send["code"] = PUSHERSVR_ERROR_STREAM_NOT_EXIST;
        send["msg"] = "fail";
        send["data"] = { "stream is not exist" };

        jsonResp = send;
        return;
    }
    uint64_t startTick = GetMillisecondCounter();
    int resp_stop = temp->stop(!disableStopCb);
    SPDINFO("[http] stop file stream {} , duration: {} ms", id, GetMillisecondCounter() - startTick);

    if (resp_stop != 0)
    {
        send["code"] = PUSHERSVR_ERROR_OP_FAIL;
        send["msg"] = "fail";
        send["data"] = { "stop filestream error" };
        jsonResp = send;
        return;
    }
    send["code"] = PUSHERSVR_ERROR_OK;
    send["msg"] = "success";
    send["data"] = {};

    jsonResp = send;

    return;

}
