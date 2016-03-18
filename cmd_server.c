#include "public.h"
#include "cmd_server.h"
#include "message_queue.h"
#include "net_trans.h"

/***************************************************************************
* �������ƣ� recv_cmd
* ���������� ��������
* �� ����    cmd_socketfd        ���������socket���
* �� ����    point_to_cmd        ָ�������ڴ�ռ��ָ��
* �� ����    cmd_len        	 ָ��������ĳ���	
* ����ֵ��   �ɹ�:OK
             ʧ��:recv_cmd_data_length_fail
                  recv_cmd_data_fail
* ����˵����		Ϊ�˷�ֹtcp�����Ƚ���������ȣ��ٽ�������ţ��������������� 
* �޸�����        �޸���            �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          	����
****************************************************************************/
int recv_cmd(int cmd_socketfd, char *cmd, int *cmd_len)
{
	int recv_len 	= 0;
	int ret			= 0;
	
	ret =  tcp_recv_data(cmd_socketfd, (char *)cmd_len, sizeof(unsigned int), MSG_DONTWAIT);		//������������������� 
	if(ret != no_data_to_recv)
	{
		if(OK != ret)
		{
#ifdef DEBUG
			echo_error_prompt(recv_cmd_data_length_fail, __FILE__, __FUNCTION__, __LINE__);
#else
			write_log(recv_cmd_data_length_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
			return  ret;
		}
			
		ret = tcp_recv_data(cmd_socketfd, cmd, *cmd_len, 0);		//�������������
		if(OK != ret)
		{
#ifdef DEBUG
			echo_error_prompt(recv_cmd_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
			write_log(recv_cmd_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
			return ret;				
		}
			
		return OK; 
	}
	else 
	{
		return no_data_to_recv;
	}
}

int take_syn_time(syn_time *syn_time_cmd)
{
	int ret = 0;
	
	fprintf(stdout, "OK\n");
	ret = stime(&(syn_time_cmd->time));
	fprintf(stdout, "OK\n");
	if(OK != ret)
	{
#ifdef DEBUG
        echo_error_prompt(take_syn_time_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(take_syn_time_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return take_syn_time_fail;		
	}
	
	return OK;
}

int send_syn_time_echo(int socket_fd, bool flag)
{
	int ret = 0;
	syn_time_echo time_echo;
	
	//time_echo.packet_size = htonl(sizeof(syn_time_echo)-4);
	time_echo.packet_size = htonl(5);
	time_echo.CMD = ECHO_SYN_TIME_CMD;
	time_echo.is_succeed = flag;
	
	ret = tcp_send_data(socket_fd, &time_echo, sizeof(syn_time_echo));
	if(OK != ret)
	{
#ifdef DEBUG
        echo_error_prompt(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return tcp_send_data_fail;		
	}
	
	return OK;
}

int dispose_syn_time_cmd(char *cmd, int socket_fd)
{
	int ret = 0;
	syn_time *syn_time_cmd = NULL;
	
	syn_time_cmd = (syn_time *)cmd;
#ifdef DEBUG
	fprintf(stdout, "[dispose_syn_time_cmd] CMD = %d\n", syn_time_cmd->CMD);
	fprintf(stdout, "[dispose_syn_time_cmd] is_succeed = %d\n", syn_time_cmd->is_succeed);
	fprintf(stdout, "[dispose_syn_time_cmd] time = %d\n", syn_time_cmd->time);
#endif
	ret = take_syn_time(syn_time_cmd);
	if(OK != ret)
	{
		ret = send_syn_time_echo(socket_fd, false);
		if(OK != ret)
		{
#ifdef DEBUG
            echo_error_prompt(send_syn_time_echo_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(send_syn_time_echo_fail, __FILE__, __FUNCTION__, __LINE__);
#endif 
            return send_syn_time_echo_fail;			
		}
	}
	else 
	{
		ret = send_syn_time_echo(socket_fd, true);
		if(OK != ret)
		{
#ifdef DEBUG
            echo_error_prompt(send_syn_time_echo_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(send_syn_time_echo_fail, __FILE__, __FUNCTION__, __LINE__);
#endif 
            return send_syn_time_echo_fail;			
		}
	}
	
	return OK;
}

/***************************************************************************
* �������ƣ� dispose_take_photo_cmd
* ���������� ���������������������Ϣ���� 
* �� ����    cmd        			��������� 
* �� ����    msg_id        			������Ϣ����id	
* ����ֵ��   �ɹ�:OK
             ʧ��:send_message_to_queue_fail
* ����˵���� 
* �޸�����        �޸���            �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          	����
****************************************************************************/
int dispose_take_photo_cmd(char *cmd, int msg_id)
{
	int ret 								= 0;
	photo_snapshot *photo_snapshot_cmd 		= NULL;
	struct_take_photo_msg take_photo_msg 	= {0};
	
	photo_snapshot_cmd = (photo_snapshot *)cmd;
#ifdef DEBUG
	fprintf(stdout, "[dispose_take_photo_cmd] is_succeed: %d\n", photo_snapshot_cmd->is_succeeed);
	fprintf(stdout, "[dispose_take_photo_cmd] photo_name: %s\n", photo_snapshot_cmd->photo_name);
#endif
	
	take_photo_msg.type = MESSAGE_QUEUE_TAKE_PHOTO_TYPE;
	memcpy(take_photo_msg.photo_name, photo_snapshot_cmd->photo_name, MAX_PHOTO_NAME_LENGTH);
	
	ret = send_message_to_queue(msg_id, &take_photo_msg, sizeof(struct_take_photo_msg));
	if(OK != ret)
	{
#ifdef DEBUG
        echo_error_prompt(send_message_to_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(send_message_to_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return send_message_to_queue_fail;		
	}
	
	return OK;
}

/***************************************************************************
* �������ƣ� send_take_photo_cmd_echo
* ���������� ���ֻ��������ս��
* �� ����    cmd_socketfd            ��������socket
* �� ����    photo_name            	 �ļ���       
* �� ����    is_take_photo_succeed   �Ƿ����ճɹ�
* �� ����    photo_size              ��Ƭ��С
* ����ֵ��    
* ����˵����    
* �޸�����        �޸���            �޸�����
* --------------------------------------------------------------------------
* 2016-01-08      �ų�              ����
* 2016-01-19	  �ų�				�޸ķ��ؿͻ�����������ȷ����������С(ע�������ֽ����������ֽ����ת��) 
****************************************************************************/
int send_take_photo_cmd_echo(int socket_fd, char *photo_name, bool is_take_photo_succeed, int photo_size)
{
	int ret 							= 0;
	photo_snapshot_echo snapshot_echo 	= {0}; 
	
	snapshot_echo.packet_size = htonl(sizeof(photo_snapshot));
	snapshot_echo.CMD = htonl(ECHO_TAKE_PHOTO_CMD);
	snapshot_echo.is_succeed = is_take_photo_succeed;
	memcpy(snapshot_echo.photo_name, photo_name, MAX_PHOTO_NAME_LENGTH);
	snapshot_echo.photo_size = htonl(photo_size);
	
	ret = tcp_send_data(socket_fd, &snapshot_echo, sizeof(photo_snapshot_echo));
	if(OK != ret) 
	{
#ifdef DEBUG
        echo_error_prompt(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return tcp_send_data_fail;
	}
	
	return OK;
}

int check_take_photo_results(int cmd_socketfd, int msg_id_for_take_photo)
{
	int ret 								= 0;
	struct_take_photo_msg take_photo_msg 	= {0};
	
	while(1)
	{
		ret = recv_message_from_queue(msg_id_for_take_photo, MESSAGE_QUEUE_TAKE_PHOTO_OVER_TYPE, &take_photo_msg, sizeof(struct_take_photo_msg), IPC_NOWAIT);
		if(ret == no_message_in_queue)
		{
			return OK;
		}
		else if(OK == ret)
		{
			ret = send_take_photo_cmd_echo(cmd_socketfd, take_photo_msg.photo_name, take_photo_msg.succeed, take_photo_msg.photo_size);
			if(OK != ret)
			{
#ifdef DEBUG
                echo_error_prompt(send_take_photo_cmd_echo_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(send_take_photo_cmd_echo_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
                return send_take_photo_cmd_echo_fail;				
			}
		}
		else
		{
#ifdef DEBUG
            echo_error_prompt(recv_message_from_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(recv_message_from_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
            return recv_message_from_queue_fail;				
		}
	}
	
	return OK;
}

/***************************************************************************
* �������ƣ� send_send_photo_cmd_echo
* ���������� ���ֻ�����ͼƬ������
* �� ����    cmd_socketfd            ��������socket
* �� ����    photo_name            	 �ļ���       
* �� ����    is_take_photo_succeed   �Ƿ���ɹ�
* �� ����    photo_size              ��Ƭ��С
* ����ֵ��    
* ����˵����    
* �޸�����        �޸���            �޸�����
* --------------------------------------------------------------------------
* 2016-01-11      �ų�              ����
****************************************************************************/
int send_send_photo_cmd_echo(int cmd_socketfd, char *photo_path_name, bool is_trans_succeed, unsigned int photo_size)
{
	int ret 					= 0;
	photo_send photo_send_echo 	= {0};
	
	photo_send_echo.CMD = ECHO_SEND_PHOTO_CMD;
	photo_send_echo.is_succeed = is_trans_succeed;
	photo_send_echo.photo_size = photo_size;
	memcpy(photo_send_echo.photo_path_name, photo_path_name, MAX_PHOTO_PATH_NAME_LENGTH);
	
	ret = tcp_send_data(cmd_socketfd, &photo_send_echo, sizeof(photo_send));
	if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return tcp_send_data_fail;
    }
    
    return OK;
}

int check_send_photo_results(int cmd_socketfd, int msg_id_for_send_photo)
{
	int ret = 0;
	struct_send_photo_msg send_photo_msg = {0};
	
	while(1)
	{
		ret = recv_message_from_queue(msg_id_for_send_photo, MESSAGE_QUEUE_TRANS_PHOTO_OVER_TYPE, &send_photo_msg, sizeof(struct_send_photo_msg), IPC_NOWAIT);
		if(no_message_in_queue == ret)
		{
			return OK;
		}
		else if(OK == ret)
		{
			ret = send_send_photo_cmd_echo(cmd_socketfd, send_photo_msg.photo_path_name, send_photo_msg.succeed, send_photo_msg.photo_size);
			if(ret != OK)
            {
#ifdef DEBUG
                echo_error_prompt(send_send_photo_cmd_echo_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(send_send_photo_cmd_echo_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
                return send_send_photo_cmd_echo_fail;
            }	
		}
		else
		{
#ifdef DEBUG
            echo_error_prompt(recv_message_from_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(recv_message_from_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
            return recv_message_from_queue_fail;			
		}
	}
	
	return OK;
}

/***************************************************************************
* @�������ƣ� cmd_server
* @���������� ������������Խ�����ʽ���ڡ��Է�������ʽ���ܡ�     		
* @����ֵ��   �ɹ�:
*					OK
*			  ʧ��:        
* @����˵����
* @�޸�����        �޸���            �޸�����
* --------------------------------------------------------------------------
  2016-01-08       �ų�             ����
****************************************************************************/
int cmd_server()
{
	int ret 						= 0;
	key_t take_photo_key 			= 0;
	key_t file_send_key 			= 0; 
	key_t client_addr_key 			= 0;
	in_addr_t *global_addr 			= NULL;
	int shm_id 						= 0;
	int msg_id_for_take_photo 		= 0;
	int msg_id_for_send_file 		= 0;
	int listen_socketfd 			= 0;
	int cmd_socketfd 				= 0;
	struct sockaddr_in client_addr;
	int client_addr_size 			= 0;
	char cmd[MAX_CMD_LENGTH] 		= {0};
	int cmd_len 					= 0;
	int cmd_number 					= 0;
	
	signal(SIGCLD, SIG_IGN);		//����Զ˹ر�socket���߳��յ��ź�Ĭ��kill������
	
	ret = get_ipc_key(&client_addr_key, IPC_KEY_SEEK_FOR_CLIENT_ADDR_SHM);
	if(OK != ret)
	{
#ifdef DEBUG
        echo_error_prompt(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return get_ipc_key_fail;		
	}
	shm_id = shmget(client_addr_key, sizeof(in_addr_t), IPC_CREAT|0600);		//�����ͻ��˵�ַ�Ĺ����ڴ� 
	if(-1 == shm_id)
	{
#ifdef DEBUG
        echo_error_prompt(get_addr_shm_id_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(get_addr_shm_id_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return get_addr_shm_id_fail;		
	}
	global_addr = (in_addr_t *)shmat(shm_id, NULL, 0);		//���ͻ��˵�ַӳ�䵽������ 
	if(NULL == global_addr)
	{
#ifdef DEBUG
        echo_error_prompt(map_addr_share_mem_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(map_addr_share_mem_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return map_addr_share_mem_fail;		
	}
	
	//����������Ϣ����
	ret = get_ipc_key(&take_photo_key, IPC_KEY_SEED_FOR_TAKE_PHOTO_MSG);
	if(OK != ret)
	{
#ifdef DEBUG
        echo_error_prompt(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return get_ipc_key_fail;		
	}
	ret = create_message_queue(take_photo_key, &msg_id_for_take_photo, true);
 	if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        msgctl(msg_id_for_send_file,IPC_RMID,NULL);
        return create_message_queue_fail;
    } 
    
    //����������Ƭ����Ϣ����
	ret = get_ipc_key(&file_send_key, IPC_KEY_SEED_FOR_SEND_PHOTO_MSG);
	if(OK != ret)
	{
#ifdef DEBUG
        echo_error_prompt(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return get_ipc_key_fail;		
	} 
	ret = create_message_queue(file_send_key, &msg_id_for_send_file, true);
	if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        msgctl(msg_id_for_send_file,IPC_RMID,NULL);
        return create_message_queue_fail;
    } 
    
    ret = sem_post(sem);		//�����ź���������֪ͨ���������������������� 
    if(OK != ret)
    {
#ifdef DEBUG          
        echo_error_prompt(sem_post_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(sem_post_fail, __FILE__, __FUNCTION__, __LINE__);
#endif  
		msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
		msgctl(msg_id_for_send_file, IPC_RMID, NULL);
		shmdt(global_addr);		//���ͻ��˵�ַ�����ڴ��뱾���̷���
		shmctl(shm_id, IPC_RMID, NULL);		//ɾ�������ڴ�� 
		return sem_post_fail;    	
    }
    
    ret = init_tcp_server_socket(&listen_socketfd, CMD_SOCKET_PORT);
    if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(init_cmd_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(init_cmd_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#endif 
		msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
		msgctl(msg_id_for_send_file, IPC_RMID, NULL);
		shmdt(global_addr);
		shmctl(shm_id, IPC_RMID, NULL);
		return init_cmd_socket_fail;   	
    }
    
    while(1)
    {
    	client_addr_size = sizeof(struct sockaddr_in);
    	if((cmd_socketfd = accept(listen_socketfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size)) == -1)
    	{
#ifdef DEBUG
            echo_error_prompt(accept_cmd_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(accept_cmd_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#endif    
			close(listen_socketfd);
			msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
			msgctl(msg_id_for_send_file, IPC_RMID, NULL);
			shmdt(global_addr);
			shmctl(shm_id, IPC_RMID, NULL);
			return 	accept_cmd_socket_fail;    	
	    }
	    
	    *global_addr = client_addr.sin_addr.s_addr;		//���������ӵĿͻ���IPд�빲���ڴ�
#ifdef DEBUG
		fprintf(stdout, "[cmd_server] global_addr = %s\n", inet_ntoa(client_addr.sin_addr)); 
#endif

    	while(1)
    	{
	    	memset(cmd, 0, MAX_CMD_LENGTH);
	    	cmd_number = 0;
	    	ret = recv_cmd(cmd_socketfd, cmd, &cmd_len);

	    	if(ret != no_data_to_recv)
	    	{
	    		if(OK != ret)		//���յ��������� 
	    		{
#ifdef DEBUG
                    echo_error_prompt(recv_cmd_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(recv_cmd_fail, __FILE__, __FUNCTION__, __LINE__);
#endif		    
					if(ret == tcp_connectiong_has_been_closed)
					{
						close(cmd_socketfd);
						break;
					}
					else		//�����쳣 
					{
						close(listen_socketfd);
						close(cmd_socketfd);
						msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
						msgctl(msg_id_for_send_file, IPC_RMID, NULL);
						shmdt(global_addr);
						shmctl(shm_id, IPC_RMID, NULL);
						return recv_cmd_fail;		
					}			
		    	}
		    	
		    	memcpy((char *)(&cmd_number), cmd, sizeof(unsigned int));
#ifdef DEBUG
				fprintf(stdout, "[cmd_server] cmd_number = %d\n", cmd_number); 
#endif
	
				switch(cmd_number)
				{
					case SYN_TIME_CMD:
						ret = dispose_syn_time_cmd(cmd, cmd_socketfd);
						if(OK != ret) {
#ifdef DEBUG
                            echo_error_prompt(dispose_syn_time_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                            write_log(dispose_syn_time_fail, __FILE__, __FUNCTION__, __LINE__);
#endif     
                            close(listen_socketfd);
                            close(cmd_socketfd);
							msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
							msgctl(msg_id_for_send_file, IPC_RMID, NULL);
							shmdt(global_addr);
							shmctl(shm_id, IPC_RMID, NULL);	
							return 	dispose_syn_time_fail;							
						}
						break;
						
					case TAKE_PHOTO_CMD:
						ret = dispose_take_photo_cmd(cmd, msg_id_for_take_photo);
						if(OK != ret)
						{
#ifdef DEBUG
                            echo_error_prompt(dispose_take_photo_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                            write_log(dispose_take_photo_fail, __FILE__, __FUNCTION__, __LINE__);
#endif     
                            close(listen_socketfd);
                            close(cmd_socketfd);
							msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
							msgctl(msg_id_for_send_file, IPC_RMID, NULL);
							shmdt(global_addr);
							shmctl(shm_id, IPC_RMID, NULL);	
							return 	dispose_take_photo_fail;					
						}
						break;
						
					default:
#ifdef DEBUG
                        echo_error_prompt(recv_unknow_cmd, __FILE__, __FUNCTION__, __LINE__);
#else
                        write_log(recv_unknow_cmd, __FILE__, __FUNCTION__, __LINE__);
#endif 
						close(listen_socketfd);
      					close(cmd_socketfd);
						msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
						msgctl(msg_id_for_send_file, IPC_RMID, NULL);
						shmdt(global_addr);
						shmctl(shm_id, IPC_RMID, NULL);
						return 	recv_unknow_cmd;						
				}
	    	}
	    	
	    	ret = check_take_photo_results(cmd_socketfd, msg_id_for_take_photo);
	    	if(OK != ret)
	    	{
#ifdef DEBUG
				echo_error_prompt(check_take_photo_results_fail, __FILE__, __FUNCTION__, __LINE__);
#else
               	write_log(check_take_photo_results_fail, __FILE__, __FUNCTION__, __LINE__);
#endif 
				close(listen_socketfd);
				close(cmd_socketfd);
				msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
				msgctl(msg_id_for_send_file, IPC_RMID, NULL);
				shmdt(global_addr);
				shmctl(shm_id, IPC_RMID, NULL);
				return check_take_photo_results_fail;					    		
	    	}
	    }
	}
    
    close(listen_socketfd);
	close(cmd_socketfd);
	msgctl(msg_id_for_take_photo, IPC_RMID, NULL);
	msgctl(msg_id_for_send_file, IPC_RMID, NULL);
	shmdt(global_addr);
	shmctl(shm_id, IPC_RMID, NULL);
	
	return OK; 
}