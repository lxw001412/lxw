#include "fileStreamResumeHandler.h"
#include "streamModInterface.h"
#include "httpServerError.h"

void fileStreamResumeHandler::post(const Json::Value & jsonReq, Json::Value & jsonResp)
{
    Json::Value recv = jsonReq;
    Json::Value send;

    std::string id;

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

    std::shared_ptr<StreamBase> temp = StreamModInterface::fileStreamMng()->getStream(id);
    if (temp == NULL)
    {
        send["code"] = PUSHERSVR_ERROR_STREAM_NOT_EXIST;
        send["msg"] = "fail";
        send["data"] = { "stream is not exist" };

        jsonResp = send;
        return;
    }
    int resp_stop = temp->resume();
    if (resp_stop != 0)
    {
        send["code"] = PUSHERSVR_ERROR_OP_FAIL;
        send["msg"] = "fail";
        send["data"] = { "resume filestream error" };
        jsonResp = send;
        return;
    }
    send["code"] = PUSHERSVR_ERROR_OK;
    send["msg"] = "success";
    send["data"] = {};

    jsonResp = send;

    return;

}
