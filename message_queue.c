#include "public.h"
#include "message_queue.h"

/***************************************************************************
* �������ƣ� create_message_queue
* ���������� ������Ϣ���� 
* �� ����    key         		��ֵ
* �� ����    msg_id             ��Ϣ����id
* �� ����    is_create_flag     ֮ǰ�Ƿ񴴽���־  
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  msgget_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
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
* �������ƣ� del_message_queue
* ���������� ������Ϣ���� 
* �� ����    msg_id             ��Ϣ����id 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  msgctl_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
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
* �������ƣ� send_message_to_queue
* ���������� ������Ϣ������ 
* �� ����    msg_id             ��Ϣ����id
* �� ����    msg_buf            ��Ϣ���� 
* �� ����    msg_buf_length     ��Ϣ����
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  msgsnd_fail
* ����˵����		���͵���Ϣ���ݲ�������Ϣid 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
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
* �������ƣ� recv_message_from_queue
* ���������� ����Ϣ�����л�ȡ 
* �� ����    msg_id             ��Ϣ����id
* �� ����    msg_type           ��Ϣ���� 
* �� ����    msg_buf            ��Ϣ���� 
* �� ����    msg_buf_length     ��Ϣ����
* �� ����    flag     			msgrcv���ƺ�����־ 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  msgrcv_fail
                  no_message_in_queue
* ����˵���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
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