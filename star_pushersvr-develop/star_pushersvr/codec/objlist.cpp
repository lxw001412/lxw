/**
 * @file objlist.cpp
 * @brief  Object list class
 * @author Bill
 * @version 1.0
 * @date 2018-09-01
 */

#include "objlist.h"
#include <string.h>

ObjList::ObjList() : m_pre(NULL), m_next(NULL)
{
}
 
ObjList::~ObjList()
{
}
 
ObjList* ObjList::InsertAsHead(ObjList* head)
{
    this->m_pre = NULL;
    this->m_next = head;
    if (head != NULL)
    {
        head->m_pre = this;
    }
    return this;
}
 
ObjList* ObjList::InsertAsTail(ObjList* head)
{
    ObjList* tail = head;
    while (tail != NULL && tail->m_next != NULL)
    {
        tail = tail->m_next;
    }
    if (tail == NULL)
    {
        this->m_pre = NULL;
        head = this;
    }
    else
    {
        tail->m_next = this;
        this->m_pre = tail;
    }
    this->m_next = NULL;
    return head;
}
 
ObjList* ObjList::RmSelf()
{
    ObjList* head = this->m_pre;
    
    if (this->m_pre != NULL)
    {
        this->m_pre->m_next = this->m_next;
        while (head->m_pre != NULL) 
        {
            head = head->m_pre;
        }
    }
    if (this->m_next != NULL)
    {
        this->m_next->m_pre = this->m_pre;
        if (head == NULL)
        {
            head = this->m_next;
        }
    }
    return head;
}

ObjList* ObjList::Next()
{
    return m_next;
}

ObjList* ObjList::Pre()
{
    return m_pre;
}
