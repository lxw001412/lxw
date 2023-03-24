#include "productModelImp.h"
#include "protocolUtils.h"


namespace star_protocol
{

///////////////////////////////////////////////////////////////////////
// productModelRepo
///////////////////////////////////////////////////////////////////////
productModelRepo* productModelRepo::m_instance = NULL;

productModelRepo* productModelRepo::instance()
{
    if (m_instance == NULL)
    {
        m_instance = new productModelRepo();
    }
    return m_instance;
}

void productModelRepo::destory()
{
    if (m_instance != NULL)
    {
        delete m_instance;
        m_instance = NULL;
    }
}

productModelRepo::productModelRepo()
{

}

productModelRepo::~productModelRepo()
{
    for (std::map<std::string, model*>::iterator it = m_productModelMap.begin();
        it != m_productModelMap.end(); ++it)
    {
        delete it->second;
    }
    m_productModelMap.clear();
}

model* productModelRepo::getProductModel(const char* productId)
{
    std::lock_guard<std::mutex> guard(m_productModelMapMutex);
    std::map<std::string, model*>::iterator it = m_productModelMap.find(std::string(productId));
    if (it == m_productModelMap.end())
    {
        return NULL;
    }
    return it->second;
}

int productModelRepo::addProductModel(const Json::Value &jsonRoot)
{
    model* m = new model();
    int ret = m->init(jsonRoot);
    if (ret != 0)
    {
        delete m;
        return ret;
    }
    std::lock_guard<std::mutex> guard(m_productModelMapMutex);
    m_productModelMap[m->productId()] = m;
    return ret;
}

///////////////////////////////////////////////////////////////////////
// model
///////////////////////////////////////////////////////////////////////
model::model()
{

}

model::~model()
{
    m_moduleIndexMap.clear();
    m_moduleNameMap.clear();
    for (std::vector<module*>::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
    {
        delete *it;
    }
    m_modules.clear();
}

int model::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject() 
        || !jsonRoot.isMember("product") || !jsonRoot["product"].isString())
    {
        return -1;
    }
    m_productId = jsonRoot["product"].asString();
    if (jsonRoot.isMember("name") && jsonRoot["name"].isString())
    {
        m_name = jsonRoot["name"].asString();
    }
    if (jsonRoot.isMember("modules"))
    {
        const Json::Value jsonModule = jsonRoot["modules"];
        if (!jsonModule.isArray())
        {
            return -2;
        }
        for (unsigned int i = 0; i < jsonModule.size(); ++i)
        {
            module *m = new module();
            if (m->init(jsonModule[i]) != 0)
            {
                delete m;
                return -3;
            }
            m_modules.push_back(m);
            m_moduleIndexMap[m->index()] = m;
            m_moduleNameMap[m->name()] = m;
        }
    }
    return 0;
}

module* model::getModule(int index)
{
    std::map<int, module*>::iterator it = m_moduleIndexMap.find(index);
    if (it == m_moduleIndexMap.end())
    {
        return NULL;
    }
    return it->second;
}

module* model::getModule(const char* name)
{
    std::map<std::string, module*>::iterator it = m_moduleNameMap.find(name);
    if (it == m_moduleNameMap.end())
    {
        return NULL;
    }
    return it->second;
}

///////////////////////////////////////////////////////////////////////
// module
///////////////////////////////////////////////////////////////////////
module::module()
{

}

module::~module()
{
    m_propertyIndexMap.clear();
    m_propertyNameMap.clear();
    for (std::vector<property*>::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
    {
        delete *it;
    }
    m_properties.clear();
}

int module::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject() 
        ||!jsonRoot.isMember("module") || !jsonRoot["module"].isString()
        || !jsonRoot.isMember("index") || !jsonRoot["index"].isInt()
        || !jsonRoot.isMember("source") || !jsonRoot["source"].isString())
    {
        return -1;
    }
    m_index = jsonRoot["index"].asInt();
    m_name = jsonRoot["module"].asString();
    m_source = jsonRoot["source"].asString();
    if (jsonRoot.isMember("properties"))
    {
        const Json::Value jsonProperties = jsonRoot["properties"];
        if (!jsonProperties.isArray())
        {
            return -2;
        }
        for (unsigned int i = 0; i < jsonProperties.size(); ++i)
        {
            property *p = new property();
            if (p->init(jsonProperties[i]) != 0)
            {
                delete p;
                return -3;
            }
            m_properties.push_back(p);
            m_propertyIndexMap[p->index()] = p;
            m_propertyNameMap[p->indentifer()] = p;
        }
    }
    return 0;
}

property* module::getProperty(int index)
{
    std::map<int, property*>::iterator it = m_propertyIndexMap.find(index);
    if (it == m_propertyIndexMap.end())
    {
        return NULL;
    }
    return it->second;
}

property* module::getProperty(const char* name)
{
    std::map<std::string, property*>::iterator it = m_propertyNameMap.find(name);
    if (it == m_propertyNameMap.end())
    {
        return NULL;
    }
    return it->second;
}

int module::dataB2J(const uint8_t* data, int size, Json::Value &jsonRoot)
{
    int offset = 0;
    while (offset < size)
    {
        int type = *(data + offset);
        offset++;
        int bytes;
        int length = getTlvLength(data + offset, bytes);
        offset += bytes;
        const uint8_t* pData = data + offset;
        offset += length;
        if (offset > size)
        {
            break;
        }
        property* prop = getProperty(type);
        if (prop == NULL)
        {
            continue;
        }
        prop->getDataType()->b2j(prop->indentifer().c_str(), pData, length, jsonRoot);
    }
    return 0;
}

int module::dataJ2B(const Json::Value &jsonRoot, uint8_t* buff, int buffSize, int &length)
{
    int offset = 0;
    Json::Value::Members members = jsonRoot.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
    {
        property* p = getProperty(it->c_str());
        if (NULL == p)
        {
            continue;
        }
        int dataLength = 0;
        if (0 == p->getDataType()->j2b(jsonRoot[*it], 
            p->index(), buff + offset, buffSize - offset, dataLength))
        {
            offset += dataLength;
        }
    }
    length = offset;
    return 0;
}

}
