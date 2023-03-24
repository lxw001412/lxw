/**
 * @file commandModelImp.h
 * @brief  星广播平台命令模型
 * @author heyang
 * @version 0.0.1
 * @date 2022-07-04
 */

#pragma once
#include "property.h"
#include <stdint.h>
#include <mutex>

namespace star_protocol
{

    class commandModel;

    struct TopicCmd
    {
        int topic;
        int cmd;
        bool operator < (const TopicCmd &tc) const 
        {
            if (topic < tc.topic)
            {
                return true;
            }
            if (topic == tc.topic)
            {
                return cmd < tc.cmd;
            }
            return false;
        }

        bool operator==(const TopicCmd &tc)
        {
            return(topic == tc.topic && cmd == tc.cmd);
        }
    };

    class commandModelRepo
    {
    public:
        static commandModelRepo* instance();
        static void destory();

        ~commandModelRepo();
        int init(const Json::Value &jsonRoot);
        commandModel* getCommandModel(int topic, int cmd);
        int addCommandModel(const Json::Value &jsonRoot);

    protected:
        commandModelRepo();

    private:
        std::vector<commandModel*> m_commands;
       // std::map<std::string, commandModel*> m_commandTopicCmdMap;
        std::map<TopicCmd, commandModel*> m_commandTopicCmdMap;
        std::mutex m_commandModelMapMutex;
        static commandModelRepo* m_instance;
    };

   
    class commandModel
    {
    public:
        commandModel();
        ~commandModel();
        int init(const Json::Value &jsonRoot);
        property* getProperty(int index);
        property* getProperty(const char* name);
        int dataB2J(const uint8_t* data, int size, Json::Value &jsonRoot);
        int dataJ2B(const Json::Value &jsonRoot, uint8_t* buff, int buffSize, int &length);

        inline int topic() const {
            return m_topic;
        }
        inline int cmd() const {
            return m_cmd;
        }
    
    private:
        int m_cmd;
        int m_topic;
        std::vector<property*> m_properties;
        std::map<int, property*> m_propertyIndexMap;
        std::map<std::string, property*> m_propertyNameMap;
    };

};
