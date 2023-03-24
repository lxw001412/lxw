#include "HttpRequest.h"

static size_t receiveServiceFeedback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct CMemoryStruct *mem = (struct CMemoryStruct *)userp;

    char *ptr = (char*)realloc(mem->cBuffer, mem->len + realsize + 1);
    if (ptr == NULL)
    {
        return 0;
    }
    else {}
    
    mem->cBuffer = ptr;
    memcpy(&(mem->cBuffer[mem->len]), contents, realsize);
    mem->len += realsize;
    mem->cBuffer[mem->len] = 0;

    return realsize;
}

static int operateFaildClew(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
    if (itype == CURLINFO_HEADER_IN)
    {
        //XINFO("[HEADER_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_HEADER_OUT)
    {
        //XINFO("[HEADER_OUT]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_IN)
    {
        //XINFO("[DATA_IN]%s\n", pData);
    }
    else if (itype == CURLINFO_DATA_OUT)
    {
        //XINFO("[DATA_OUT]%s\n", pData);
    }
    else {}

    return 0;
}

HttpRequest * HttpRequest::create(std::string url, std::string methodType)
{    
    HttpRequest * http_obj = new HttpRequest(url, methodType); 
    return http_obj;
}

HttpRequest::HttpRequest(std::string url, std::string methodType)
{
    m_url = url;
    m_methodType = methodType;
}

void HttpRequest::destroy(HttpRequest *& obj)
{
    if (obj != NULL)
    {
        delete obj;
    } 
}

int HttpRequest::action(std::string & resp)
{
    CURLcode res;
    m_curl = curl_easy_init();
    if (NULL == m_curl)
    {
        return -1;
    }
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1);

    curl_slist *http_headers = NULL;
    http_headers = curl_slist_append(http_headers, "Accept: */*");
    http_headers = curl_slist_append(http_headers, m_header.c_str());
    http_headers = curl_slist_append(http_headers, m_contentType.c_str());

//    printf("Head: <%s>\n", m_header.c_str());
//    printf("Url : <%s>\n", m_url.c_str());
//    printf("MethodType : <%s>\n", m_methodType.c_str());
//    printf("Body : <%s>\n", m_body.c_str());

    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, http_headers);
    //设定url
    curl_easy_setopt(m_curl, CURLOPT_URL, m_url.c_str());
    //设置调试回调函数
    curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, operateFaildClew);
    //设置请求模式
    if (m_methodType == "GET")
    {
        curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1);
    }
    else if (m_methodType == "POST")
    {
       
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, m_body.c_str());
        curl_easy_setopt(m_curl, CURLOPT_POST, 1);
    }
    else if (m_methodType == "PUT")
    {
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, m_body.c_str());
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "PUT");
    }
    else if (m_methodType == "DELETE")
    {
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, m_body.c_str());
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    else
    {
        curl_slist_free_all(http_headers);
        curl_easy_cleanup(m_curl);
        return -1;
    }
    //有需要保存的数据时，curl调用回调函数receiveServiceFeedback
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, receiveServiceFeedback);
    //将  获取到的数据 的指针getBufferPointer，返回值时数据指针  传递给 回调函数receiveServiceFeedback中的第四个参数
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, (void *)this->getBufferPointer());
    curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 10);//超过10s没有响应，断开http连接
    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 20);

    curl_easy_setopt(m_curl, CURLOPT_COOKIEFILE, "cookie_open.txt");
    curl_easy_setopt(m_curl, CURLOPT_COOKIEJAR, "cookie_open.txt");

    res = curl_easy_perform(m_curl);

    curl_slist_free_all(http_headers);
    curl_easy_cleanup(m_curl);

    resp = m_buffer.cBuffer;
    return res;
}

void HttpRequest::setHeader(std::string header)
{
    m_header = header;
}

void HttpRequest::setParam(std::string body, std::string contentType)
{
    m_body = body;
    m_contentType = "Content-type: ";
    m_contentType += contentType;
}
