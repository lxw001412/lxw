#pragma once

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CMemoryStruct 
{
	CMemoryStruct()
	{
		//will be grown as needed by the realloc above
		cBuffer = (char*)malloc(1);
		memset(cBuffer, 0, 1);
		len = 0;
	};

	char* cBuffer;
	size_t len;
};

//解析websvr反馈的数据
class CParseHttpDataBase
{
public:
	CParseHttpDataBase(){}
    ~CParseHttpDataBase()
	{
		free(m_buffer.cBuffer);
	};

	CMemoryStruct* getBufferPointer()
	{
		return &m_buffer;
	}

protected:
	CMemoryStruct m_buffer;
};