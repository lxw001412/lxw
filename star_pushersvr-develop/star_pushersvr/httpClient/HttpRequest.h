/**
*
* @file 	HttpRequest.h
* @author	黄晨阳
* @date		2021.11.30
* @brief
* 该类为http请求封装
* @brief

调用步骤：1、先调用create，产生请求对象
          2、调用setHeader，设置header
          3、调用setParam，设置body
          4、调用action，开始请求
          5、调用destroy，释放
**/
#pragma once
#include <iostream>
#include "curl/curl.h"
#include "parse-http-data-base.h"
#include "json/json.h"
#include <string>
#include <vector>
#include <mutex>

class HttpRequest : public CParseHttpDataBase
{
public:

    /**
    * @brief  创建http请求对象
    *
    * @param data 入参：
    *                  url ：访问地址 类型：string
    *                  methodType ：请求方式 类型：string
    *
    * @return 返回http请求对象指针
    *
    * @note 如果是get方法，参数要在url中设置完整之后，再传入create
    *  methodType 有四种情况分别为："GET" "POST" "PUT" "DELETE"
    */
    static HttpRequest* create(std::string url, std::string methodType);

    /**
    * @brief  销毁http请求对象
    *
    * @param data 入参：
    *                  obj ：http请求对象指针 类型：HttpRequest*
    */
    static void destroy(HttpRequest * &obj);

    /**
    * @brief  设置header
    *
    * @param data 入参：
    *                  header ：头  类型：string
    *
    * @note 参数按照标准形式传入，比如"Content-Type:application/x-www-form-urlencoded"
    *                                 "Content-Type:application/json"
    */
    void setHeader(std::string header);

    /**
    * @brief  设置body和编码形式
    *
    * @param data 入参：
    *                  body ：体  类型：string
    *                  contentType ：编码形式  类型：string
    *
    * @note body为设置好形式的字符串
          例如，当contenttype为x-www-form-urlencoded时，
          body为"username=kttest&password=a123456"
    */
    void setParam(std::string body, std::string contentType);

    /**
    * @brief  开始执行请求
    *
    * @param data 出参：
    *                  resp ：访问地址的答复，没有解析，直接透传过来  类型：string
    *
    * @note 如果是get方法，参数要在url中设置完整之后，再传入create
    */
    int action(std::string &resp);//resp为out参数，请求返回的响应

public:
    CURL *m_curl;//curl对象
    std::string m_url;//访问地址
    std::string m_methodType;//请求方式
    std::string m_body;//体
    std::string m_contentType;//编码方式
    std::string m_header;//头
private:

    /**
    * @brief  http请求构造函数
    *
    * @param data 入参：
    *                  url ：访问地址 类型：string
    *                  methodType ：请求方式 类型：string
    *
    * @note 如果是get方法，参数要在url中设置完整之后，再传入create
    */
    HttpRequest(std::string url, std::string methodType);
};