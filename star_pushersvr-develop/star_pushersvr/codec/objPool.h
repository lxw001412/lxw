#pragma once

#include <ace/Singleton.h>
#include <ace/Mutex.h>


template <class ObjType, typename ParamType>
class ObjPollManager
{
public:
	inline void init(int count, ParamType param)
	{
		m_param = param;
		ACE_Guard<ACE_Mutex> guard(m_objs_mux);
		for (int i = 0; i < count; i++)
		{
			ObjType* obj = new ObjType(param);
			m_objs = (ObjType*)obj->InsertAsHead(m_objs);
			m_total++;
			m_inpool++;
		}
	}

	inline void clear()
	{
	    ObjType* obj = NULL;
		ACE_Guard<ACE_Mutex> guard(m_objs_mux);
		while (m_objs != NULL)
		{
		    obj = m_objs;
		    m_objs = (ObjType*)m_objs->RmSelf();
		    delete obj;
		}
	}

	inline ObjType* get_obj()
	{
		ObjType* obj = NULL;
		ACE_Guard<ACE_Mutex> guard(m_objs_mux);
		if (m_objs != NULL)
		{
		    m_inpool--;
			obj = m_objs;
			m_objs = (ObjType*)m_objs->RmSelf();
			return obj;
		}
		m_total++;
		return new ObjType(m_param);
	}

	inline void release_obj(ObjType* obj)
	{
		obj->clear();
		ACE_Guard<ACE_Mutex> guard(m_objs_mux);
		m_objs = (ObjType*)obj->InsertAsHead(m_objs);
		m_inpool++;
	}

	inline int total()
	{
		return m_total;
	}

	inline int inpool()
	{
	    return m_inpool;
	}

protected:
	ObjPollManager() : m_objs(NULL), m_total(0), m_inpool(0) {}

	virtual ~ObjPollManager()
	{
		clear();
	}

private:
	ACE_Mutex m_objs_mux;
	ObjType* m_objs;
	ParamType m_param;
	int m_total;
	int m_inpool;

protected:
	friend class ACE_Singleton<ObjPollManager<ObjType, ParamType>, ACE_Mutex>;
};

