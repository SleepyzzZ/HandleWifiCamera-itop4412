#include "public.h"
#include "rtsp.h"
#include "rtp.h"
#include "net_trans.h"
#include "video.h"
#include "video_server.h"

int video_server()
{
	int ret 								= 0;
	char *recv_buf 							= NULL;
	char *send_buf 							= NULL;
	struct_rtp_pthread_param client_info 	= {0};
	struct_frame_buffer frame_buffer_info 	= {0};
	int client_addr_len 					= 0;
	unsigned short client_port 				= 0;
	struct sockaddr_in addr_client;
	char session_id[12] 					= {0};
	int rtsp_sockfd 						= 0;
	int rtsp_conn_sockfd 					= 0;
	unsigned char rtsp_bit_mask 			= 0b00000000;
	void *pthread_cancel_ret 				= NULL;
	pthread_t rtp_pthread_id;
	pthread_t video_pthread_id;
	
	signal(SIGPIPE, SIG_IGN);		//避免被对方中断网络后收到信号，并自动杀死本进程
	
	send_buf = (char *)calloc(1400, sizeof(char));
	recv_buf = (char *)calloc(1400, sizeof(char));
	
	//初始化视频帧共享内存结构体
	frame_buffer_info.frame_buffer = (char *)calloc(NALU_BUFFER_LENGTH, sizeof(char));
	pthread_mutex_init(&(frame_buffer_info.mtx), NULL);
	pthread_cond_init(&(frame_buffer_info.cond), NULL);
	sem_init(&(frame_buffer_info.sem), 0, 0);		//信号量初始化位0，属性为在线程内共享
	frame_buffer_info.is_full = false;
	frame_buffer_info.SPS = (char *)calloc(100, sizeof(char));
	frame_buffer_info.PPS = (char *)calloc(100, sizeof(char));
	
	ret = init_tcp_server_socket(&rtsp_sockfd, RTSP_SOCKET_PORT); 
	if(OK != ret)
	{
#ifdef DEBUG
        echo_error_prompt(init_rtsp_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(init_rtsp_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
		free(send_buf);
		free(recv_buf);
		free(frame_buffer_info.frame_buffer);
        free(frame_buffer_info.SPS);
		free(frame_buffer_info.PPS);
		pthread_mutex_destroy(&(frame_buffer_info.mtx));
		pthread_cond_destroy(&(frame_buffer_info.cond));
		sem_destroy(&(frame_buffer_info.sem));
        
        return init_rtsp_socket_fail; 		
	}
	
	while(1)
	{
		rtsp_bit_mask = 0b00000000;
		
		memset(session_id, 0, 12);
		Create_Session_ID(session_id);
		
		memset(&addr_client, 0, sizeof(struct sockaddr_in));
		client_addr_len = sizeof(struct sockaddr_in);
		rtsp_conn_sockfd = accept(rtsp_sockfd, (struct sockaddr *)&addr_client, &client_addr_len);
		if(rtsp_conn_sockfd < 0)
		{
#ifdef DEBUG
            echo_error_prompt(accept_rtsp_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(accept_rtsp_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
			free(recv_buf);
			free(send_buf);
			free(frame_buffer_info.frame_buffer);
			free(frame_buffer_info.SPS);
			free(frame_buffer_info.PPS);
			pthread_mutex_destroy(&(frame_buffer_info.mtx));
			pthread_cond_destroy(&(frame_buffer_info.cond));
			sem_destroy(&(frame_buffer_info.sem));
			close(rtsp_sockfd);
			return accept_rtsp_socket_fail;			
		}
		
		ret = pthread_create(&video_pthread_id, NULL, video_pthread, (void *)&frame_buffer_info);
		if(OK != ret)
		{
#ifdef DEBUG
            echo_error_prompt(create_video_pthread_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(create_video_pthread_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
			free(recv_buf);
			free(send_buf);
			free(frame_buffer_info.frame_buffer);
			free(frame_buffer_info.SPS);
			free(frame_buffer_info.PPS);
			pthread_mutex_destroy(&(frame_buffer_info.mtx));
			pthread_cond_destroy(&(frame_buffer_info.cond));
			sem_destroy(&(frame_buffer_info.sem));
			close(rtsp_conn_sockfd); 
			close(rtsp_sockfd);	
			return 	create_video_pthread_fail;		
		}
		
		ret = sem_wait(&(frame_buffer_info.sem));
		if(OK != ret)
		{
#ifdef DEBUG
            echo_error_prompt(sem_wait_video_pthread_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(sem_wait_video_pthread_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
			free(recv_buf);
			free(send_buf);
			free(frame_buffer_info.frame_buffer);
			free(frame_buffer_info.SPS);
			free(frame_buffer_info.PPS);
			pthread_mutex_destroy(&(frame_buffer_info.mtx));
			pthread_cond_destroy(&(frame_buffer_info.cond));
			sem_destroy(&(frame_buffer_info.sem));
			close(rtsp_conn_sockfd); 
			close(rtsp_sockfd);
			pthread_cancel(video_pthread_id);
			pthread_join(video_pthread_id, &pthread_cancel_ret);
			return sem_wait_video_pthread_fail;			
		}
		
		while(1)
		{
			memset(send_buf, 0, 1400);
			memset(recv_buf, 0, 1400);
			
			ret = recv(rtsp_conn_sockfd, recv_buf, 1400, 0);
			if(0 >= ret)
			{
#ifdef DEBUG
                echo_error_prompt(recv_rtsp_command_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(recv_rtsp_command_fail, __FILE__, __FUNCTION__, __LINE__);
#endif			
				free(recv_buf);
				free(send_buf);
				free(frame_buffer_info.frame_buffer);
				free(frame_buffer_info.SPS);
				free(frame_buffer_info.PPS);
				pthread_mutex_destroy(&(frame_buffer_info.mtx));
				pthread_cond_destroy(&(frame_buffer_info.cond));
				sem_destroy(&(frame_buffer_info.sem));
				close(rtsp_conn_sockfd); 
				close(rtsp_sockfd);
				pthread_cancel(video_pthread_id);
				pthread_join(video_pthread_id, &pthread_cancel_ret);
				return recv_rtsp_command_fail;
			}
			
			if(NULL != strstr(recv_buf, "OPTIONS")) 
			{
				if((RTSP_CMD_BIT_MASK_OPTIONS == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_OPTIONS)) || (0 != rtsp_bit_mask))
				{
					if( RTSP_CMD_BIT_MASK_PLAY == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_PLAY))
				    {
                        pthread_cancel(rtp_pthread_id);
						pthread_join(rtp_pthread_id, &pthread_cancel_ret);
				    }
				    pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
				}
				
				ret = Handle_OPTIONS( send_buf, recv_buf );
			    if(ret != OK )
			    {
					//close(rtsp_conn_sockfd);
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
                }
							
				ret = tcp_send_data(rtsp_conn_sockfd, send_buf, strlen(send_buf));
			    if(ret != OK )
			    {
#ifdef DEBUG
                    echo_error_prompt(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif	    
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
                }
				
				rtsp_bit_mask = rtsp_bit_mask | RTSP_CMD_BIT_MASK_OPTIONS;  
			}
			
			else if( NULL != strstr(recv_buf, "DESCRIBE") )
            {
                if( (RTSP_CMD_BIT_MASK_DESCRIBE == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_DESCRIBE)) || (RTSP_CMD_BIT_MASK_OPTIONS !=rtsp_bit_mask))
                {
				    if( RTSP_CMD_BIT_MASK_PLAY == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_PLAY))
				    {
                        pthread_cancel(rtp_pthread_id);
						pthread_join(rtp_pthread_id, &pthread_cancel_ret);
				    }
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
				}
				
                ret = Handle_DESCRIBE(send_buf, recv_buf, frame_buffer_info.SPS, frame_buffer_info.length_of_SPS, frame_buffer_info.PPS,frame_buffer_info.length_of_PPS);
			    if(ret != OK )
				{
					//close(rtsp_conn_sockfd);
                    pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
				}

                ret = tcp_send_data(rtsp_conn_sockfd, send_buf, strlen(send_buf));
			    if(ret != OK )
			    {
#ifdef DEBUG
                    echo_error_prompt(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif				    
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
                }   
				
				rtsp_bit_mask = rtsp_bit_mask | RTSP_CMD_BIT_MASK_DESCRIBE;
			}
			
			else if( NULL != strstr(recv_buf, "SETUP") )
			{
                if((RTSP_CMD_BIT_MASK_SETUP == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_SETUP)) || ((RTSP_CMD_BIT_MASK_OPTIONS |
					                                                                             RTSP_CMD_BIT_MASK_DESCRIBE)!=rtsp_bit_mask))
                {
				    if( RTSP_CMD_BIT_MASK_PLAY == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_PLAY))
				    {
                        pthread_cancel(rtp_pthread_id);
						pthread_join(rtp_pthread_id, &pthread_cancel_ret);
				    }
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
				}
				
                ret = get_port(recv_buf, &client_port);
                if(ret != OK )
			    {
#ifdef DEBUG
                    echo_error_prompt(get_client_port_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(get_client_port_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
				}
				ret = Handle_SETUP(send_buf, recv_buf, session_id);
                if(ret != OK )
			    {
					//close(rtsp_conn_sockfd);
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
				}

                ret = tcp_send_data(rtsp_conn_sockfd, send_buf, strlen(send_buf));
			    if(ret != OK )
			    {
#ifdef DEBUG
                    echo_error_prompt(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif			    
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
                }   
				
				rtsp_bit_mask = rtsp_bit_mask | RTSP_CMD_BIT_MASK_SETUP;
			}
			
			else if( NULL != strstr(recv_buf, "PLAY") )
		    {
                if((RTSP_CMD_BIT_MASK_PLAY == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_PLAY)) || ((RTSP_CMD_BIT_MASK_OPTIONS |
					                                                                           RTSP_CMD_BIT_MASK_DESCRIBE| 
					                                                                           RTSP_CMD_BIT_MASK_SETUP)!=rtsp_bit_mask))
                {
				    if( RTSP_CMD_BIT_MASK_PLAY == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_PLAY))
				    {
                        pthread_cancel(rtp_pthread_id);
						pthread_join(rtp_pthread_id, &pthread_cancel_ret);
				    }
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
				}
			
		        ret = Handle_PLAY(send_buf, recv_buf, session_id);
			    if(OK != ret)
                {
					//close(rtsp_conn_sockfd);
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
                }

                ret = tcp_send_data(rtsp_conn_sockfd, send_buf, strlen(send_buf));
			    if(ret != OK )
			    {
#ifdef DEBUG
                    echo_error_prompt(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
                }   
				
                client_info.addr_client = &addr_client;
                client_info.client_port = client_port;
                client_info.frame_buffer_info = &frame_buffer_info;
                ret = pthread_create( &rtp_pthread_id, NULL, rtp_pthread, (void *)&client_info );
                if( OK != ret)
                {
#ifdef DEBUG
                    echo_error_prompt(create_rtp_thread_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(create_rtp_thread_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
                    free(send_buf);
				    free(recv_buf);	
					free(frame_buffer_info.frame_buffer);
                    free(frame_buffer_info.SPS);
				    free(frame_buffer_info.PPS);
		            pthread_mutex_destroy(&(frame_buffer_info.mtx));
		            pthread_cond_destroy(&(frame_buffer_info.cond));
		            sem_destroy(&(frame_buffer_info.sem));
					close(rtsp_conn_sockfd);
					close(rtsp_sockfd);
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    return create_rtp_thread_fail;
                }
				
				rtsp_bit_mask = rtsp_bit_mask | RTSP_CMD_BIT_MASK_PLAY;
				
			}
			
			else if( NULL != strstr(recv_buf, "TEARDOWN ") )
			{	
                if((RTSP_CMD_BIT_MASK_TEARDOWN == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_TEARDOWN)) || ((RTSP_CMD_BIT_MASK_OPTIONS |
					                                                                                   RTSP_CMD_BIT_MASK_DESCRIBE | 
					                                                                                   RTSP_CMD_BIT_MASK_SETUP |
					                                                                                   RTSP_CMD_BIT_MASK_PLAY)!=rtsp_bit_mask))
                {
				    if(RTSP_CMD_BIT_MASK_PLAY == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_PLAY))
				    {
                        pthread_cancel(rtp_pthread_id);
						pthread_join(rtp_pthread_id, &pthread_cancel_ret);
				    }
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    break;
				}

			    ret = Handle_TEARDOWN(send_buf, recv_buf, session_id);
			    if(OK != ret)
                {
					//close(rtsp_conn_sockfd);
					pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    pthread_cancel(rtp_pthread_id);
					pthread_join(rtp_pthread_id, &pthread_cancel_ret);
                    break;
                }

				ret = tcp_send_data(rtsp_conn_sockfd, send_buf, strlen(send_buf));
                if(ret != OK )
			    {
#ifdef DEBUG
                    echo_error_prompt(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(tcp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
			        pthread_cancel(video_pthread_id);
					pthread_join(video_pthread_id, &pthread_cancel_ret);
                    pthread_cancel(rtp_pthread_id);
					pthread_join(rtp_pthread_id, &pthread_cancel_ret);
                    break;
                }   

				pthread_cancel(video_pthread_id);
				pthread_join(video_pthread_id, &pthread_cancel_ret);
                pthread_cancel(rtp_pthread_id);
				pthread_join(rtp_pthread_id, &pthread_cancel_ret);

				rtsp_bit_mask = rtsp_bit_mask | RTSP_CMD_BIT_MASK_TEARDOWN;

				break;
            }
            
            else                           //其他未知命令
			{
				//close(rtsp_conn_sockfd);
				if(RTSP_CMD_BIT_MASK_PLAY == (rtsp_bit_mask & RTSP_CMD_BIT_MASK_PLAY))
				{
				    pthread_cancel(rtp_pthread_id);           //如果是未知命令跳出循环，结束video线程和rtp线程，跳出本次rtsp会话等待手机发起下一次连接
                    pthread_join(rtp_pthread_id, &pthread_cancel_ret);
				}
				pthread_cancel(video_pthread_id);
				pthread_join(video_pthread_id, &pthread_cancel_ret);
			    break;							  
		    }
		}//end while(1)
		
		close(rtsp_conn_sockfd);
	}//end while(1)
	
	close(rtsp_sockfd);
	free(send_buf);
	free(recv_buf);
	free(frame_buffer_info.frame_buffer);
	free(frame_buffer_info.SPS);
	free(frame_buffer_info.PPS);
	pthread_mutex_destroy(&(frame_buffer_info.mtx));
	pthread_cond_destroy(&(frame_buffer_info.cond));
	sem_destroy(&(frame_buffer_info.sem));
	
	return OK;
}