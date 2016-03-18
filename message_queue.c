#include "public.h"
#include "message_queue.h"

/***************************************************************************
* 函数名称： create_message_queue
* 功能描述： 创建消息队列 
* 参 数：    key         		键值
* 参 数：    msg_id             消息队列id
* 参 数：    is_create_flag     之前是否创建标志  
* 返回值：   成功:               
                  OK
             失败:                 
                  msgget_fail
* 其它说明： 
* 修改日期        修改人        修改内容
* --------------------------------------------------------------------------
  2016-01-08      张超          创建
****************************************************************************/
int create_message_queue(int key, int *msg_id, bool is_create_flag)
{
	int ret 			= 0;
	int exitent_msg_id 	= 0;
	
	if(is_create_flag)
	{
		ret = msgget(key, IPC_CREAT|IPC_EXCL|0666);
		if((-1 == ret) && (errno == EEXIST))
		{
			exitent_msg_id = msgget(key, IPC_CREAT|0666);
			msgctl(exitent_msg_id, IPC_RMID, NULL);
		}
	}
	
	ret = msgget(key, IPC_CREAT|0666);
	if(-1 == ret)
	{
#ifdef DEBUG
        echo_error_prompt(msgget_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(msgget_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return msgget_fail; 		
	}
	
	*msg_id = ret;
	return OK;
}

/***************************************************************************
* 函数名称： del_message_queue
* 功能描述： 创建消息队列 
* 参 数：    msg_id             消息队列id 
* 返回值：   成功:               
                  OK
             失败:                 
                  msgctl_fail
* 其它说明： 
* 修改日期        修改人        修改内容
* --------------------------------------------------------------------------
  2016-01-08      张超          创建
****************************************************************************/
int del_message_queue(int msg_id)
{
	int ret = 0;
	
	ret = msgctl(msg_id, IPC_RMID, NULL);
	if(-1 == ret)
	{
#ifdef DEBUG
        echo_error_prompt(msgctl_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(msgctl_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return msgctl_fail; 		
	}
	
	return OK;
}

/***************************************************************************
* 函数名称： send_message_to_queue
* 功能描述： 发送消息至队列 
* 参 数：    msg_id             消息队列id
* 参 数：    msg_buf            消息内容 
* 参 数：    msg_buf_length     消息长度
* 返回值：   成功:               
                  OK
             失败:                 
                  msgsnd_fail
* 其它说明：		发送的消息内容不包括消息id 		
* 修改日期        修改人        修改内容
* --------------------------------------------------------------------------
  2016-01-08      张超          创建
****************************************************************************/
int send_message_to_queue(int msg_id, void *msg_buf, int msg_buf_length)
{
	int ret = 0;
	
	ret = msgsnd(msg_id, msg_buf, msg_buf_length-sizeof(long), 0);
	if(-1 == ret)
	{
#ifdef DEBUG
        echo_error_prompt(msgsnd_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(msgsnd_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return msgsnd_fail;		
	} 
	
	return OK;
} 

/***************************************************************************
* 函数名称： recv_message_from_queue
* 功能描述： 从消息队列中获取 
* 参 数：    msg_id             消息队列id
* 参 数：    msg_type           消息类型 
* 参 数：    msg_buf            消息内容 
* 参 数：    msg_buf_length     消息长度
* 参 数：    flag     			msgrcv控制函数标志 
* 返回值：   成功:               
                  OK
             失败:                 
                  msgrcv_fail
                  no_message_in_queue
* 其它说明： 		
* 修改日期        修改人        修改内容
* --------------------------------------------------------------------------
  2016-01-08      张超          创建
****************************************************************************/
int recv_message_from_queue(int msg_id, long msg_type, void *msg_buf, int msg_buf_length, int flag)
{
	int ret = 0;
	
	ret = msgrcv(msg_id, msg_buf, msg_buf_length-sizeof(long), msg_type, flag); 
    if((-1 == ret) && (errno != ENOMSG))
    {
#ifdef DEBUG
        echo_error_prompt(msgrcv_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(msgrcv_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return msgrcv_fail;
    }  

	if((-1 == ret) && (errno == ENOMSG))
    {
        return no_message_in_queue;
	}
	 
 	return OK;
} 