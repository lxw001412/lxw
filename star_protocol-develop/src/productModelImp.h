/**
 * @file productModelImp.h
 * @brief  星广播平台终端模型
 * @author Bill
 * @version 0.0.1
 * @date 2021-05-31
 */

#pragma once
#include "property.h"
#include <stdint.h>
#include <mutex>

namespace star_protocol
{

class model;
class module;

class productModelRepo
{
public:
    static productModelRepo* instance();
    static void destory();

    ~productModelRepo();

    model* getProductModel(const char* productId);
    int addProductModel(const Json::Value &jsonRoot);

protected:
    productModelRepo();

private:
    std::map<std::string, model*> m_productModelMap;
    std::mutex m_productModelMapMutex;
    static productModelRepo* m_instance;
};

class model
{
public:
    model();
    ~model();
    int init(const Json::Value &jsonRoot);
    module* getModule(int index);
    module* getModule(const char* name);
    inline const std::string& productId() const {
        return m_productId;
    }
    inline const std::string& name() const {
        return m_name;
    }
    
private:
    std::string m_productId;
    std::string m_name;
    std::vector<module*> m_modules;
    std::map<int, module*> m_moduleIndexMap;
    std::map<std::string, module*> m_moduleNameMap;
};

class module
{
public:
    module();
    ~module();
    int init(const Json::Value &jsonRoot);
    property* getProperty(int index);
    property* getProperty(const char* name);
    int dataB2J(const uint8_t* data, int size, Json::Value &jsonRoot);
    int dataJ2B(const Json::Value &jsonRoot, uint8_t* buff, int buffSize, int &length);

    inline int index() const {
        return m_index;
    }
    inline const std::string& name() const {
        return m_name;
    }
    inline const std::string& source() const {
        return m_source;
    }
private:
    std::string m_name;
    int m_index;
    std::string m_source;
    std::vector<property*> m_properties;
    std::map<int, property*> m_propertyIndexMap;
    std::map<std::string, property*> m_propertyNameMap;
};

};
