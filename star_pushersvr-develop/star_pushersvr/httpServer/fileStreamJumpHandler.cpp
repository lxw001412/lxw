#include "fileStreamJumpHandler.h"
#include "streamModInterface.h"
#include "httpServerError.h"
#include "streamInImp.h"
void filestreamJumpHandler::post(const Json::Value & jsonReq, Json::Value & jsonResp)
{
    Json::Value recv = jsonReq;
    Json::Value send;

    std::string id;
    int index;
    int pos;

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

    if (recv.isMember("pos") && recv["pos"].isInt())
    {
        pos = recv["pos"].asInt();
    }
    else
    {
        send["code"] = PUSHERSVR_ERROR_PARAMETER;
        send["msg"] = "param error";
        send["data"] = { " pos error " };

        jsonResp = send;
        return;
    }
	int rc = -1;
	m_streamIn = new StreamInImp();                           //创建新流
    if (recv.isMember("index") && recv["index"].isInt())
    {
		index = recv["index"].asInt();
		temp->updatePlayStack(index);
		temp->GetMessageQueue()->pop(index);
		const FileInfo_t& fi = temp->info()->fileList()[index];

		rc = m_streamIn->open(temp->info()->id(), fi.path);
		if (rc != 0)
		{
			std::stringstream ss;
			ss << "Open " << fi.path << " failed";
			temp->setLastError(SEC_STREAMIN, ss.str().c_str());
			return;
		}
		if (pos != 0)
		{
			m_streamIn->seekFile(pos * 1000);
		}
        //如果有输入index
        int resp_stop = temp->jump(index, pos,m_streamIn);
        if (resp_stop != 0)
        {
            send["code"] = PUSHERSVR_ERROR_OP_FAIL;
            send["msg"] = "fail";
            send["data"] = { "jump filestream error1" };
            jsonResp = send;
            return;

        }
        send["code"] = PUSHERSVR_ERROR_OK;
        send["msg"] = "success";
        send["data"] = {};
        jsonResp = send;
        return;
    }
    else 
    {
        //没输入index，使用当前index
		temp->updatePlayStack(temp->info()->curFileIndex());
		temp->GetMessageQueue()->pop(index);
		const FileInfo_t& fi = temp->info()->fileList()[index];
		rc = m_streamIn->open(temp->info()->id(), fi.path);
		if (rc != 0)
		{
			std::stringstream ss;
			ss << "Open " << fi.path << " failed";
			temp->setLastError(SEC_STREAMIN, ss.str().c_str());
			return;
		}
		if (pos != 0)
		{
			m_streamIn->seekFile(pos * 1000);
		}
        int resp_stop = temp->jump(temp->info()->curFileIndex(), pos,m_streamIn);
        if (resp_stop != 0)
        {
            send["code"] = PUSHERSVR_ERROR_OP_FAIL;
            send["msg"] = "fail";
            send["data"] = { "jump filestream error" };
            jsonResp = send;
            return;
        }
        send["code"] = PUSHERSVR_ERROR_OK;
        send["msg"] = "success";
        send["data"] = {};
        jsonResp = send;
        return;
    }

    return;
}
