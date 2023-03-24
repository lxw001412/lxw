/**
 * @file objlist.h
 * @brief  Object list class
 * @author Bill
 * @version 1.0
 * @date 2018-09-01
 */

#ifndef NP2P_OBJLIST_H
#define NP2P_OBJLIST_H

class ObjList
{
public:
    ObjList();
    virtual ~ObjList();
    
    /**
    * @brief InsertAsHead 
    *		                            将本对象插入到队列头
    * @param head			  		    队列头指针
    *
    * @return		                    队列头指针
    */
    ObjList* InsertAsHead(ObjList* head);
    
    /**
    * @brief InsertAsTail 
    *		                            将本对象插入到队列尾
    * @param head			  		    队列头指针
    *
    * @return		                    队列头指针
    */
    ObjList* InsertAsTail(ObjList* head);
    
    /**
    * @brief RmSelf 
    *		                            从队列中删除本节点
    *
    * @return		                    队列头指针
    */
    ObjList* RmSelf();
    
    /**
    * @brief Next 
    *		                            下一个节点
    *
    * @return		                    下一个节点指针
    */
    ObjList* Next();
    
    /**
    * @brief Next 
    *		                            上一个节点
    *
    * @return		                    上一个节点指针
    */
    ObjList* Pre();
    
protected:
    ObjList* m_pre;
    ObjList* m_next;
};

#endif // NP2P_OBJLIST_H
