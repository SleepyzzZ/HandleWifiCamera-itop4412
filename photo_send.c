#include "public.h"
#include "message_queue.h"
#include "net_trans.h"
#include "photo_send.h"

/***************************************************************************
* 函数名称： return_trans_photo_result_to_message_queue
* 功能描述： 返回图片发送消息队列 
* 参 数：    msg_id             	消息队列id 
* 参 数：    photo_path_name      	图片绝对路径 
* 参 数：    is_trans_succeed       发送成功与否     
* 返回值：   成功: 
					OK              
             失败:  
			 		...              
* 其它说明：		
* 修改日期        修改人        修改内容
* --------------------------------------------------------------------------
  2016-01-10      张超          创建
****************************************************************************/
int return_trans_photo_result_to_message_queue(int msg_id, char *photo_path_name, bool is_trans_succeed)
{
    int        ret                  		= 0;
    struct_send_photo_msg send_photo_msg  	= {0};
	
	send_photo_msg.type = MESSAGE_QUEUE_TRANS_PHOTO_OVER_TYPE;
	send_photo_msg.succeed = is_trans_succeed;
	memcpy(send_photo_msg.photo_path_name, photo_path_name, MAX_PHOTO_PATH_NAME_LENGTH);

    ret = send_message_to_queue(msg_id, &send_photo_msg, sizeof(struct_send_photo_msg));
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
* 函数名称： send_photo_data
* 功能描述： 发送图片数据-先发送图片数据大小 
* 参 数：    socket_fd             	套接字句柄 
* 参 数：    photo_path_name      	图片绝对路径 
* 参 数：    photo_size       		图片大小     
* 返回值：   成功: 
					OK              
             失败:  
			 		...              
* 其它说明：		
* 修改日期        修改人        修改内容
* --------------------------------------------------------------------------
  2016-01-10      张超          创建
****************************************************************************/
int send_photo_data(int socket_fd, char *photo_path_name, unsigned int photo_size)
{
    int          ret                                         	= 0;
    int          read_length                                 	= 0;
    int          file_fd                                     	= 0;
    char         send_buffer[MAX_SEND_PHOTO_SOCKET_BUFFER_SIZE] = {0};
    struct stat  file_stat                                   	= {0};
    unsigned int file_length_to_send                         	= 0;
    unsigned int need_read_to_buffer_length                  	= 0;
    unsigned int have_read_length                            	= 0;
    unsigned int need_to_send                                	= 0;
    int cmd_len												 	= 0;

    file_length_to_send = photo_size;
    file_fd = open(photo_path_name, O_RDONLY);
    if( 0 > file_fd )
    {
#ifdef DEBUG
        echo_error_prompt(open_picture_file_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(open_picture_file_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return open_picture_file_fail;        
    }
    
    if(0 != file_length_to_send) {
    	cmd_len = htonl(file_length_to_send);
    	ret = tcp_send_data(socket_fd, (char *)(&cmd_len), sizeof(int));
    	if(OK != ret)
    	{
#ifdef DEBUG
            echo_error_prompt(send_photo_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(send_photo_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif 
            close(file_fd);
            return send_photo_data_fail;	    	
	    }
    }

    
    while( 0 != file_length_to_send )
    {
        if(file_length_to_send > MAX_SEND_PHOTO_SOCKET_BUFFER_SIZE)
        {
            need_read_to_buffer_length = MAX_SEND_PHOTO_SOCKET_BUFFER_SIZE;
        }
        else
        {
            need_read_to_buffer_length = file_length_to_send;
        }
        need_to_send = need_read_to_buffer_length;
        while( 0 != need_read_to_buffer_length )
        {
            read_length = read(file_fd, send_buffer + have_read_length, need_read_to_buffer_length);
            if( 0 > read_length )
            {
               if( errno == EINTR || errno == EAGAIN )
               {
                   continue;
               }
               else
               {
#ifdef DEBUG
                   echo_error_prompt(read_photo_file_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                   write_log(read_photo_file_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
                   close(file_fd);
                   return read_photo_file_fail;         
               }
            }
            
            have_read_length = have_read_length + read_length;
            need_read_to_buffer_length = need_read_to_buffer_length - read_length;
        }
        
        ret = tcp_send_data(socket_fd, send_buffer, need_to_send);
        if(OK != ret)
        {
#ifdef DEBUG
            echo_error_prompt(send_photo_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(send_photo_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif 
            close(file_fd);
            return send_photo_data_fail;
        }

        file_length_to_send = file_length_to_send - need_to_send;	
		have_read_length = 0;
		memset(send_buffer, 0, MAX_SEND_PHOTO_SOCKET_BUFFER_SIZE);
    }

    close(file_fd);
    return OK;
}

int photo_trans()
{
    int               socket_fd                                   = 0;
    int               ret                                         = 0;
    struct_send_photo_msg  send_photo_msg                         = {0};
	key_t             file_send_key                               = 0;
	int               msg_id_for_send_photo                       = 0;
	char              photo_path_name[MAX_PHOTO_PATH_NAME_LENGTH] = {0}; 
	key_t    		  client_addr_key							  = 0;
	in_addr_t	      *global_addr                                = NULL;
	int 			  shm_id									  = 0;
	
	//客户端地址共享内存 
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
	shm_id = shmget(client_addr_key, sizeof(in_addr_t), IPC_CREAT|0600);
	if(-1 == shm_id)
	{
#ifdef DEBUG
        echo_error_prompt(get_addr_shm_id_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(get_addr_shm_id_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return get_addr_shm_id_fail;
	}
	global_addr = (in_addr_t *)shmat(shm_id, NULL, 0);
	if(NULL == global_addr)
	{
#ifdef DEBUG
        echo_error_prompt(map_addr_share_mem_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(map_addr_share_mem_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return map_addr_share_mem_fail;		
	}
	
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
	
	//创建图片发送消息队列 
    ret = create_message_queue(file_send_key, &msg_id_for_send_photo, false);
    if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return create_message_queue_fail;
    }	

    while(1)
    {
        memset(&send_photo_msg, 0, sizeof(struct_send_photo_msg));
        ret = recv_message_from_queue(msg_id_for_send_photo, MESSAGE_QUEUE_TRANS_PHOTO_TYPE, &send_photo_msg, sizeof(struct_send_photo_msg), 0);
        if(OK != ret)
        {
#ifdef DEBUG
            echo_error_prompt(recv_message_from_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(recv_message_from_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
            return recv_message_from_queue_fail;
        }
        
        if(access(send_photo_msg.photo_path_name, F_OK) < 0)
        {
#ifdef DEBUG
            echo_error_prompt(can_not_find_image_file, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(can_not_find_image_file, __FILE__, __FUNCTION__, __LINE__);
#endif
            ret = return_trans_photo_result_to_message_queue(msg_id_for_send_photo, send_photo_msg.photo_path_name, false);
            if(OK != ret)
            {
#ifdef DEBUG
                echo_error_prompt(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
                return return_trans_result_to_message_queue_fail;
            }

            continue;
        }

        ret = init_tcp_client_socket(&socket_fd, *global_addr, PHOTO_TRANS_SOCKET_PORT);
        if(OK != ret)
        {
#ifdef DEBUG
            echo_error_prompt(create_tcp_client_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(create_tcp_client_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
            return create_tcp_client_socket_fail;
        }   

        ret = send_photo_data(socket_fd, send_photo_msg.photo_path_name, send_photo_msg.photo_size);
        if(OK != ret)
        {
#ifdef DEBUG
            echo_error_prompt(send_photo_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(send_photo_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
            ret = return_trans_photo_result_to_message_queue(msg_id_for_send_photo, send_photo_msg.photo_path_name, false);
            if(OK != ret)
            {
#ifdef DEBUG
                echo_error_prompt(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
                close(socket_fd);
                return return_trans_result_to_message_queue_fail;
            }        
            close(socket_fd);
            continue;
        }

        ret = return_trans_photo_result_to_message_queue(msg_id_for_send_photo, send_photo_msg.photo_path_name, true);
        if(OK != ret)
        {
#ifdef DEBUG
            echo_error_prompt(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
            close(socket_fd);
            return return_trans_result_to_message_queue_fail;
        }   

        close(socket_fd);
    }

    return OK;
}