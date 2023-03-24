#include "AppConfig.h"
#include <sstream>
#include "utils.h"

typedef ACE_Singleton<AppConfig, ACE_Null_Mutex> APPCONFIG;

AppConfig::AppConfig()
{
}

AppConfig::~AppConfig()
{
	if (!(m_handles.empty()))
	{
        m_handles.clear();
	}
}

AppConfig * AppConfig::instance()
{
	return APPCONFIG::instance();
}

void AppConfig::Destory()
{
	APPCONFIG::close();
}

int AppConfig::load(const std::string & path)//返回值不为0，为失败
{
	//获取配置文件路径
	m_filePath = path;

	//从文件读取数据
	char buff[10240];
	Json::Value jsonRoot;

	FILE *fp = fopen(m_filePath.c_str(), "rb");//exe 上一级目录
	if (fp == NULL)
	{
		printf("Open Configfile failed\n");
		return -1;
	}
    size_t size = fread(buff, 1, sizeof(buff), fp);
	buff[size] = 0;
	fclose(fp);

	//将文件解析成json，存在m_json中
	Json::Value configDataJson;
    int res = str2json(buff, configDataJson);
    if (res == 0)
    {
        m_json = configDataJson;
        return 0;
    }
    else
    {
        return -1;
    }
}

int AppConfig::save()
{
	//将修改过的json存入文件
	FILE *fp = fopen(m_filePath.c_str(), "wb");
    if (fp != nullptr)
    {
        fwrite((m_json.toStyledString().c_str()), strlen(m_json.toStyledString().c_str()) + 1, 1, fp);
        fclose(fp);
        notifyUpdate();
    }

	return 0;
}

int AppConfig::getStringParam(const std::string & mod, const std::string & key , std::string & value)
{
	//开始读取数据
	if (m_json.isMember(mod))
	{
		if (m_json[mod].isMember(key) && m_json[mod][key].isString())
		{
            value =  m_json[mod][key].asString();
		}
		else
		{
			return -1;//不存在这个key
		}
	}
	else
	{
		return -2;//不存在这个mod
	}
    return 0;
}

int AppConfig::getIntParam(const std::string & mod, const std::string & key, int & value)
{
	std::string string_value;
	//int int_value;
	//开始读取数据
	if (m_json.isMember(mod))
	{
		if (m_json[mod].isMember(key) && m_json[mod][key].isString())
		{
			string_value = m_json[mod][key].asString();
			std::stringstream ss;
			ss << string_value;
			ss >> value;
		}
		else
		{
			return -1;//不存在这个key
		}
	}
	else
	{
		return -2;//不存在这个mod
	}
    return 0;
}

int AppConfig::getBoolParam(const std::string & mod, const std::string & key,bool &value)
{
	std::string string_value;
	//开始读取数据
	if (m_json.isMember(mod))
	{
		if (m_json[mod].isMember(key) && m_json[mod][key].isString())
		{
			string_value = m_json[mod][key].asString();
			if (string_value == "true")
			{
                value = true;
			}
			else
			{
                value = false;
			}
		}
		else
		{
			return -1;//不存在这个key
		}
	}
	else
	{
		return -2;//不存在这个mod
	}
    return true;
}

int AppConfig::getStringVecParam(const std::string & mod, const std::string & key , std::vector<std::string> &value)
{
	std::vector<std::string> vec_value;
	//开始读取数据
	if (m_json.isMember(mod))
	{

		if (m_json[mod].isMember(key))
		{
			//参数值是字符串类型，则将字符串值作为数组中唯一的元素返回
			if (m_json[mod][key].isString())
			{
				vec_value.push_back(m_json[mod][key].asString());
                value = vec_value;
			}
			//参数值是数组,将元素分别push进去
			else if (m_json[mod][key].isArray())
			{
				for (unsigned int i = 0; i < m_json[mod][key].size(); i++)
				{
					vec_value.push_back(m_json[mod][key][i].asString());
				}
                value = vec_value;
			}
			else
			{
				return -3;
			}
		}
		else
		{
			return -1;//不存在这个key
		}
	}
	else
	{
		return -2;//不存在这个mod
	}
    return 0;
}

int AppConfig::setParam(const std::string & mod, const std::string & key, const std::string & value)
{
		m_json[mod][key] = value;//新增数据
	    return 0;
}

int AppConfig::setParam(const std::string & mod, const std::string & key, const int value)
{
	std::string str_value;
	//将value转成string 再存进去

	std::stringstream ss;
	ss << value;
	ss >> str_value;
	m_json[mod][key] = str_value;//新增数据

	return 0;
}

int AppConfig::setParam(const std::string & mod, const std::string & key, const bool value)
{
    //将value转成string 再存进去
	if (value == true)
	{
		m_json[mod][key] = "true";//新增数据
	}
	else
	{
		m_json[mod][key] = "false";//新增数据
	}
	return 0;
}

int AppConfig::setParam(const std::string & mod, const std::string & key, const std::vector<std::string>& value)
{
	Json::Value out_value;

	//将value的元素取出，然后依次存储。
	unsigned int i = 0;
	std::vector<std::string>::const_iterator it = value.begin();
	for (; it != value.end(), i < value.size(); it++, i++)
	{
		out_value[i] = *it;
	}
	m_json[mod][key] = out_value;//新增数据
	return 0;
}

void AppConfig::registryHandler(IConfigChangeHandler * handler)
{
	this->m_handles.push_back(handler);
}

void AppConfig::unregistryHandler(IConfigChangeHandler * handler)
{
	if (!(m_handles.empty()))
	{
		std::vector<IConfigChangeHandler* >::iterator it = m_handles.begin();
		for (; it != m_handles.end(); it++)
		{
			if ((*it) == handler)
			{
				m_handles.erase(it);
				break;
			}
		}
	}
}

void AppConfig::notifyUpdate()
{
	if (!(m_handles.empty()))
	{//执行回调
		std::vector<IConfigChangeHandler* >::iterator it = m_handles.begin();
		for (; it != m_handles.end(); it++)
		{
			(*it)->handleConfigChange();
		}
	}
	else
	{
		std::cout << "m_handles is empty" << std::endl;
	}
}
