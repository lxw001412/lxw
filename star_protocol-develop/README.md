## 星云广播终端协议库

### 协议文档
http://git.comtom.cn:18000/starplatform/star_protocol/-/wikis/home


### 编译
#### 依赖库
1. spdlog
2. jsoncpp

#### linux编译
```
mkdir build
cd build
cmake ..
make
```

编译结果： libstarprotocol.so

#### windows编译
vs2017

### 测试程序
```
./test
```


### 接口说明
#### 初始化
1. 日志初始化：initProtocolLog
2. 装载终端模型：addProductModel

#### 报文解析为消息
```
std::shared_ptr<termPackage> p(termPackage::makeTermPackage());

// 1. 使用报文数据初始化termPackage对象
p->init(data, size);

// 2. 报文校验
if (!p->verify())
{
    // 报文校验失败
}

// 3. 读取报文头相关数据
p->isAck()
p->topic();
p->cmd();

// 4. 创建消息对象
std::shared_ptr<termMessage> msg(termMessage::makeTermMessage());

// 5. 使用报文对象初始化消息对象，第二个参数为终端模型中的产品ID
int rc = msg->initFromTermPackage(p.get(), "bffa9c91-3cd0-407a-b43e-5748e1482fda");

if (0 == rc)
{
    // 消息报文已完整。
}

// 6. 检查消息对象是否完整
if (msg->isComplete())
{
    // 7. 获取JSON格式的消息数据
    const Json::Value& jsonMsg = msg->getJsonMsg();
}

```

#### JSON格式消息转换为报文数据
```
1. 创建消息对象
std::shared_ptr<termMessage> msg(termMessage::makeTermMessage());

2. 使用JSON数据初始化消息对象
Json::Value jsonMsg;
int rc = msg->initFromJson(jsonMsg, "bffa9c91-3cd0-407a-b43e-5748e1482fda", true, NULL);

if (rc != 0)
{
    // json 消息解析失败
}

3. 依次获取报文数据
for (int i = 0; i < msg->termPackageCount(); ++i)
{ 
    const termPackage* pkg = msg->getTermPackage(i);
    // 发送数据 send(pkg->data(), pkg->length());
}
```

