// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "productModelImp.h"
#include "protocolUtils.h"
#include "tickcount.h"
#include "protocol.h"
#include "spdlogging.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

using namespace star_protocol;

void printfBuf(const void *buf, int size, int lineChar = 8);

void devidCacheTest();

int test1();

int termMessageTest();

int main()
{
    std::vector<std::string> modelFiles;
    modelFiles.push_back("product_v0.2.json");
    modelFiles.push_back("configModel_v0.2.json");

    initProtocolLog("trace", "protocol.log", 1, 10, true);

    for (std::vector<std::string>::const_iterator it = modelFiles.begin(); it != modelFiles.end(); ++it)
    {
        char *buff;
        FILE *fp = fopen(it->c_str(), "rb");
        if (fp == NULL)
        {
            printf("open product_v0.2.json failed\n");
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        int length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buff = new char[length + 1];
        int size = fread(buff, 1, length, fp);
        buff[size] = 0;
        fclose(fp);
        Json::Value jsonRoot;
        Json::Features features;
        Json::Reader reader(features);
        reader.parse(buff, jsonRoot);
        delete[]buff;

        int ret = addProductModel(jsonRoot);
        if (ret != 0)
        {
            printf("add product model error\n");
            return 1;
        }
    }

    termMessageTest();

    destroyProductModelRepo();
    finiProtocolLog();

    return 0;
}

int termMessageTest()
{
    char buff[10240];
    Json::Value jsonRoot;
    Json::Features features;
    Json::Reader reader(features);
    FILE *fp = fopen("message.json", "rb");
    if (fp == NULL)
    {
        printf("Open message.json failed\n");
        return 1;
    }
    int size = fread(buff, 1, sizeof(buff), fp);
    buff[size] = 0;
    fclose(fp);

    Json::Value msgRoot;
    reader.parse(buff, msgRoot);
    
    try
    {
        Json::Value &cmds = msgRoot["cmds"];
        for (Json::Value::const_iterator it = cmds.begin(); it != cmds.end(); ++it)
        {
            const Json::Value cmd = *it;
            termMessage* msg = termMessage::makeTermMessage();

            printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@\n%s\n", cmd.toStyledString().c_str());

            if (cmd["topic"].asInt() == 4)
            {
                msg->initFromJson(cmd, "382cc936-bb12-4382-a435-964af709c2bb", true, NULL);
            }
            else
            {
                msg->initFromJson(cmd, "bffa9c91-3cd0-407a-b43e-5748e1482fda", true, NULL);
            }

            termMessage* msgRecv = termMessage::makeTermMessage();
            for (int i = 0; i < msg->termPackageCount(); ++i)
            { 
                const termPackage* pkg = msg->getTermPackage(i);
                printfBuf(pkg->data(), pkg->length());
                msgRecv->initFromTermPackage(pkg, "bffa9c91-3cd0-407a-b43e-5748e1482fda");
            }
            if (!msgRecv->isComplete())
            {
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Message error 1\n");
                continue;
            }
            if (msgRecv->getJsonMsg().toStyledString() != cmd.toStyledString())
            {
                printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!message not equal: \n%s\n%s\n", cmd.toStyledString().c_str(), msgRecv->getJsonMsg().toStyledString().c_str());
                for (int i = 0; i < msg->termPackageCount(); ++i)
                {
                    printfBuf(msg->getTermPackage(i)->data(), msg->getTermPackage(i)->size(), 8);
                }
                return 1;
            }
        }
    }
    catch (const std::exception &e)
    {
        printf("term message test exception: %s\n", e.what());
        return 1;
    }
    printf("term message test pass\n");
    return 0;
}

int test1()
{
    char buff[10240];
    Json::Value jsonRoot;
    Json::Features features;
    Json::Reader reader(features);
    FILE *fp = fopen("E:\\config.json", "rb");
    int size = fread(buff, 1, sizeof(buff), fp);
    buff[size] = 0;
    fclose(fp);

    Json::Value configRoot;
    reader.parse(buff, configRoot);


    uint8_t moduleData[1600] = { 0 };
    int length = 0;
    model* m = productModelRepo::instance()->getProductModel("bffa9c91-3cd0-407a-b43e-5748e1482fda");
    if (m == NULL)
    {
        printf("model is NULL\n");
        return 2;
    }
    int ret = m->getModule("config")->dataJ2B(configRoot, moduleData, sizeof(moduleData), length);
    if (ret != 0)
    {
        printf("data json to bin error\n");
        return 3;
    }
    Json::Value configRoot2;
    ret = m->getModule("config")->dataB2J(moduleData, length, configRoot2);
    if (ret != 0)
    {
        printf("data bin to json error\n");
        return 4;
    }

    printfBuf(moduleData, length);

    printf("config module: \n%s\n", configRoot2.toStyledString().c_str());
    printf("pass\n");

    int length1 = 234;
    uint8_t temp[2];
    int bytes = 0;
    setTlvLength(length1, temp, 2, bytes);
    int length2 = getTlvLength(temp, bytes);
    std::cout << "length:" << length2 << std::endl;


    fp = fopen("E:\\setdatareq.json", "rb");
    size = fread(buff, 1, sizeof(buff), fp);
    buff[size] = 0;
    fclose(fp);
    Json::Value setDataReq;
    reader.parse(buff, setDataReq);

    {
        termMessage* msg = termMessage::makeTermMessage();
        msg->initFromJson(setDataReq, "bffa9c91-3cd0-407a-b43e-5748e1482fda", true, "20090401");
        termMessage* msg2 = termMessage::makeTermMessage();
        for (int i = 0; i < msg->termPackageCount(); ++i)
        {
            printfBuf(msg->getTermPackage(i)->data(), msg->getTermPackage(i)->size());
            msg2->initFromTermPackage(msg->getTermPackage(i), "bffa9c91-3cd0-407a-b43e-5748e1482fda");
        }
        std::cout << "term message:\n" << msg2->getJsonMsg().toStyledString() << std::endl;
        if (msg2->getJsonMsg().toStyledString() == msg->getJsonMsg().toStyledString())
        {
            std::cout << "term message 1 pass" << std::endl;
        }
    }

    {
        termMessage* msg = termMessage::makeTermMessage();
        msg->initFromJson(setDataReq, "bffa9c91-3cd0-407a-b43e-5748e1482fda", true, NULL);
        termMessage* msg2 = termMessage::makeTermMessage();
        for (int i = 0; i < msg->termPackageCount(); ++i)
        {
            printfBuf(msg->getTermPackage(i)->data(), msg->getTermPackage(i)->size());
            msg2->initFromTermPackage(msg->getTermPackage(i), "bffa9c91-3cd0-407a-b43e-5748e1482fda");
        }
        std::cout << "term message:\n" << msg2->getJsonMsg().toStyledString() << std::endl;
        if (msg2->getJsonMsg().toStyledString() == msg->getJsonMsg().toStyledString())
        {
            std::cout << "term message 2 pass" << std::endl;
        }
    }

    {
        termMessage* msg = termMessage::makeTermMessage();
        msg->initFromJson(setDataReq, "bffa9c91-3cd0-407a-b43e-5748e1482fda", false, "20090401");
        termMessage* msg2 = termMessage::makeTermMessage();
        for (int i = 0; i < msg->termPackageCount(); ++i)
        {
            printfBuf(msg->getTermPackage(i)->data(), msg->getTermPackage(i)->size());
            msg2->initFromTermPackage(msg->getTermPackage(i), "bffa9c91-3cd0-407a-b43e-5748e1482fda");
        }
        std::cout << "term message:\n" << msg2->getJsonMsg().toStyledString() << std::endl;
        if (msg2->getJsonMsg().toStyledString() == msg->getJsonMsg().toStyledString())
        {
            std::cout << "term message 3 pass" << std::endl;
        }
    }

    devidCacheTest();

    Json::Value jsonTest1;
    jsonTest1["qos"] = 1;
    jsonTest1["msg"]["url"] = "rtmp://x.x.x.x/x/y";
    std::cout << jsonTest1.toStyledString() << std::endl;
    return 0;
}

void devidCacheTest()
{
    char sn[64];
    for (int i = 0; i < 10000; ++i)
    {
        sprintf(sn, "150-015-03378-20210526-%05d", i + 1);
        uint64_t devid = devidCache::instance()->str2devid(sn);
    }
    
    uint64_t s1 = GetMillisecondCounter();
    for (int i = 0; i < 10000; ++i)
    {
        sprintf(sn, "150-015-03378-20210526-%05d", i + 1);
        uint64_t devid = str2devid(sn);
    }
    uint64_t e1 = GetMillisecondCounter();

    uint64_t s2 = GetMillisecondCounter();
    for (int i = 0; i < 10000; ++i)
    {
        sprintf(sn, "150-015-03378-20210526-%05d", i + 1);
        uint64_t devid = devidCache::instance()->str2devid(sn);
    }
    uint64_t e2 = GetMillisecondCounter();

    std::cout << "devid string to uint64 old:" << e1 - s1 << "ms, new:" << e2 - s2 << "ms" << std::endl;

    uint64_t devid = devidCache::instance()->str2devid("150-015-03378-20210526-00001");


    uint64_t ss1 = GetMillisecondCounter();
    for (int i = 0; i < 10000; ++i)
    {
        devid2str(devid + i, sn, sizeof(sn));
    }
    uint64_t ee1 = GetMillisecondCounter();

    uint64_t ss2 = GetMillisecondCounter();
    for (int i = 0; i < 10000; ++i)
    {
        devidCache::instance()->devid2str(devid + i);
    }
    uint64_t ee2 = GetMillisecondCounter();

    std::cout << "devid uint64 to string old:" << ee1 - ss1 << "ms, new:" << ee2 - ss2 << "ms" << std::endl;

    devidCache::destory();

}

void printfBuf(const void *buf, int size, int lineChar)
{
    std::stringstream ss;
    const uint8_t *p = (uint8_t*)buf;
    for (int i = 0; i < size; i++)
    {
        if (i % lineChar == 0)
        {
            ss << "\n";
        }
        char str[16] = { 0 };
        sprintf(str, "0x%02X ", *(p + i));
        ss << str;
    }
    ss << "\n";
    SPDINFO("{}", ss.str());
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
