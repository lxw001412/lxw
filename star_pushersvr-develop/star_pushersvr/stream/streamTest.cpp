/**
 * @file streamTest.cpp
 * @brief 推流模块测试代码
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-30
 * @description 推流模块测试代码
 */

#include "streamMod.h"
#include "spdlogging.h"
#include "streamInfoImp.h"
#include "fileStream.h"
#include "streamModInterface.h"
#include <chrono>
#include <iostream>
#include <sstream>

static int fileStreamTestDuraiton();
static int fileStreamTestRandom();
static int fileStreamTestCmd();
static int realtimeStreamTestDuration(int duration);
static StreamMod sm;

int streamMemoryTest(int count)
{
    for (int i = 0; i < count; ++i)
    {
        std::stringstream ssId;
        ssId << i;
        std::stringstream ssUrl;
        ssUrl << "rtmp://113.240.243.236/test123456/aaa" << i;

        std::shared_ptr<StreamInfo> stream_info = std::shared_ptr<StreamInfo>(StreamModInterface::createStreamInfo());
        stream_info->id(ssId.str());
        stream_info->url(ssUrl.str());
        stream_info->enableProcessCallback(true);
        stream_info->cycle(1);
        FileInfoList_t fileList;
        FileInfo_t fileInfo;
        fileInfo.id = "0";
        fileInfo.path = "http://192.168.112.221/media/jam.mp3";
        fileInfo.index = 0;
        fileList.push_back(fileInfo);
        stream_info->fileList(fileList);
        std::shared_ptr<StreamBase> ps = StreamModInterface::fileStreamMng()->createStream(ssId.str());
        ps->start(stream_info.get());        
    }
    return 0;
}

int streamModTest()
{
    int rc = sm.init();
    if (rc != 0)
    {
        SPDERROR("stream mod init failed: {}", rc);
        return 1;
    }
    rc = sm.open();
    if (rc != 0)
    {
        SPDERROR("stream mod open failed: {}", rc);
        return 1;
    }

	rc = realtimeStreamTestDuration(10);
	if (rc != 0)
	{
		SPDERROR("realtime stream duration test failed: {}", rc);
		return 1;
	}
	else
	{
		SPDERROR("realtime stream duration test ok");
	}

    rc = fileStreamTestCmd();
    if (rc != 0)
    {
        SPDERROR("file stream random test failed: {}", rc);
        return 1;
    }
    else
    {
        SPDERROR("file stream random test ok");
    }

    rc = fileStreamTestDuraiton();
    if (rc != 0)
    {
        SPDERROR("file stream duration test failed: {}", rc);
        return 1;
    }
    else
    {
        SPDERROR("file stream duration test ok");
    }

    rc = fileStreamTestRandom();
    if (rc != 0)
    {
        SPDERROR("file stream random test failed: {}", rc);
        return 1;
    }
    else
    {
        SPDERROR("file stream random test ok");
    }
    
    sm.close();
    sm.finit();
    return 0;
}

static int realtimeStreamTestDuration(int duration)
{
	StreamInfoImp si;
	si.id("realtimeStreamTestDuration");
	si.url("rtmp://113.240.243.236/star/remotestream");
	si.enableProcessCallback(true);
	si.source("rtmp://113.240.243.236/star/sourcestream");
	si.duration(duration);


	std::shared_ptr<StreamBase> fs = StreamModInterface::realStreamMng()->createStream("realtimeStreamTestDuration");
	if (fs == nullptr)
	{
		SPDERROR("create realtime stream failed!\n");
		return -1;
	}

	int rc = fs->start(&si);
	if (rc != 0)
	{
		return rc;
	}

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (fs->info()->status() == SS_STOPPED)
		{
			break;
		}
	}

	fs->stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	return fs->info()->lastErrorCode();
}

static int fileStreamTestDuraiton()
{
    StreamInfoImp si;
    si.id("fileStreamTestDuraiton");
    si.url("rtmp://113.240.243.236/star/test");
    si.enableProcessCallback(true);
    FileInfoList_t fl;
    FileInfo_t fi;

    char tmp[64] = { 0 };
    for (int i = 1; i < 10; i++)
    {
        sprintf(tmp, "%d", i);
        fi.id = tmp;
        fi.index = i - 1;
        sprintf(tmp, "http://192.168.112.221/media/%d.aac", i);
        fi.path = tmp;
        fl.push_back(fi);
    }
    si.fileList(fl);
    si.duration(100);
    si.mode(FM_SEQ);

    std::shared_ptr<StreamBase> fs = StreamModInterface::fileStreamMng()->createStream(si.id());
    int rc = fs->start(&si);
    if (rc != 0)
    {
        return rc;
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (fs->info()->status() == SS_STOPPED)
        {
            break;
        }
    }
    fs->stop();
    return fs->info()->lastErrorCode();
}

static int fileStreamTestRandom()
{
    StreamInfoImp si;
    si.id("fileStreamTestRandom");
    si.url("rtmp://113.240.243.236/star/test");
    si.enableProcessCallback(true);
    FileInfoList_t fl;
    FileInfo_t fi;

    char tmp[64] = { 0 };
    for (int i = 1; i < 10; i++)
    {
        sprintf(tmp, "%d", i);
        fi.id = tmp;
        fi.index = i - 1;
        sprintf(tmp, "http://192.168.112.221/media/%d.aac", i);
        fi.path = tmp;
        fl.push_back(fi);
    }
    si.fileList(fl);
    si.cycle(2);
    si.mode(FM_RANDOM);

    std::shared_ptr<StreamBase> fs = StreamModInterface::fileStreamMng()->createStream(si.id());
    int rc = fs->start(&si);
    if (rc != 0)
    {
        return rc;
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (fs->info()->status() == SS_STOPPED)
        {
            break;
        }
    }
    fs->stop();
    return fs->info()->lastErrorCode();
}

static int fileStreamTestCmd()
{
    StreamInfoImp si;
    si.id("fileStreamTestCmd");
    si.url("rtmp://113.240.243.236/star/test");
    si.enableProcessCallback(false);
    FileInfoList_t fl;
    FileInfo_t fi;

    char tmp[64] = { 0 };
    for (int i = 1; i < 5; i++)
    {
        sprintf(tmp, "%d", i);
        fi.id = tmp;
        fi.index = i - 1;
        sprintf(tmp, "http://192.168.112.221/media/%d.aac", i);
        fi.path = tmp;
        fl.push_back(fi);
    }
    si.fileList(fl);
    si.cycle(2);
    si.mode(FM_SEQ);

    std::shared_ptr<StreamBase> fs = StreamModInterface::fileStreamMng()->createStream(si.id());
    int rc = fs->start(&si);
    if (rc != 0)
    {
        return rc;
    }

    while (true)
    {
        std::cout << "input command: jump, pause, resume, stop" << std::endl;
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "jump")
        {
            std::cout << "please input file index: ";
            int index;
            std::cin >> index;
            std::cout << "please input file pos: ";
            int pos;
            std::cin >> pos;
            fs->jump(index, pos);
        }
        else if (cmd == "pause")
        {
            fs->pause();
        }
        else if (cmd == "resume")
        {
            fs->resume();
        }
        else if (cmd == "stop")
        {
            break;
        }
    }
    fs->stop();
    return fs->info()->lastErrorCode();
}