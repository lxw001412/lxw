#include "ConfigMod.h"
#include "AppConfig.h"
int ConfigMod::init(const std::string & val)
{
	AppConfig::instance()->load(val);
	return 0;
}

void ConfigMod::finit()
{
	AppConfig::instance()->Destory();
}
