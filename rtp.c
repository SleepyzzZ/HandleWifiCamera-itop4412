#include "rtp.h"
#include "net_trans.h"
#include "public.h"

int get_globe_frame_info(struct_rtp_pthread_param *frame_info, char *nalu_buf, unsigned int *nalu_length, unsigned int *time_stamp)
{
    int ret = 0;
    ret = pthread_mutex_lock( &(frame_info->frame_buffer_info->mtx) );
    if(OK != ret)
    {
#ifdef DEBUG          
            echo_error_prompt(get_frame_mutex_lock_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(get_frame_mutex_lock_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return get_frame_mutex_lock_fail;
    }
    pthread_cleanup_push(pthread_mutex_unlock, &(frame_info->frame_buffer_info->mtx));
		 
    while((frame_info->frame_buffer_info)->is_full == false) //如果帧已被取走
    {
        ret = pthread_cond_wait(&(frame_info->frame_buffer_info->cond),&(frame_info->frame_buffer_info->mtx));
        if(0 != ret)
        {
#ifdef DEBUG          
            echo_error_prompt(get_frame_cond_wait_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(get_frame_cond_wait_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return get_frame_cond_wait_fail;
        } 
    }
		
	*time_stamp = frame_info->frame_buffer_info->time_stamp;
	*nalu_length = frame_info->frame_buffer_info->frame_length - H264_START_CODE_LENGTH; 
	memcpy(nalu_buf, frame_info->frame_buffer_info->frame_buffer + H264_START_CODE_LENGTH, *nalu_length);
	  	
	frame_info->frame_buffer_info->is_full = false;
	  	
    pthread_cond_signal(&(frame_info->frame_buffer_info->cond)); 

	pthread_mutex_unlock(&(frame_info->frame_buffer_info->mtx));
    pthread_cleanup_pop(0); 
	
	return OK;
}

void cleanup_rtp_pthread(void *parm)
{
    close(((struct_free_rtp_param*)parm)->socket_fd);
	free(((struct_free_rtp_param*)parm)->nalu_buf);  
}

void *rtp_pthread( void *info )
{
    int                        socket_fd            = 0; 		
    unsigned short             seq_num              = 0; 	
    unsigned int               time_stamp           = 0;		
    char                       *nalu_buf            = NULL;
    char                       *nalu_payload_buf    = NULL;
    unsigned int               nalu_length          = 0;
    unsigned int               nalu_payload_length  = 0;    
    struct_single_RTP_packet   single_RTP_packet    = {0};
    struct_fragment_RTP_packet FU_A_rtp_packet      = {0};
	struct_free_rtp_param      thread_param_to_free = {0};
    unsigned int               next_frame           = 0;
    unsigned int               last_slice_length    = 0; 
    unsigned int               slices_count         = 0;
    int                        ret                  = 0;
    int                        i                    = 0;
    struct sockaddr_in         addr_client;
    struct sockaddr_in         addr_server;


    memset( &addr_client,0,sizeof(struct sockaddr_in));
    addr_client = *(((struct_rtp_pthread_param *)info)->addr_client);
    addr_client.sin_port = htons(((struct_rtp_pthread_param *)info)->client_port);

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == socket_fd)
    {
#ifdef DEBUG          
        echo_error_prompt(create_udp_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(create_udp_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(create_udp_socket_fail);
	}

    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(5004); 
    addr_server.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(addr_server.sin_zero), 8); 
    ret = bind(socket_fd, (struct sockaddr *)&addr_server, sizeof(struct sockaddr_in));
    if(ret <0)
    {
#ifdef DEBUG          
        echo_error_prompt(udp_bind_addr_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(udp_bind_addr_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(udp_bind_addr_fail);
    }

    nalu_buf = (char *)calloc(NALU_BUFFER_LENGTH, sizeof(char));

    thread_param_to_free.socket_fd = socket_fd;
	thread_param_to_free.nalu_buf = nalu_buf;
	pthread_cleanup_push(cleanup_rtp_pthread,&thread_param_to_free);
	
    while(1)       //循环获取帧
	{
       next_frame = 0;

	   ret = get_globe_frame_info((struct_rtp_pthread_param *)info, nalu_buf, &nalu_length, &time_stamp);
       if(0 != ret)
       {
#ifdef DEBUG          
           echo_error_prompt(get_globe_frame_fail, __FILE__, __FUNCTION__, __LINE__);
#else
           write_log(get_globe_frame_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
           pthread_exit(get_globe_frame_fail);
       }

       if( nalu_length <= PAYLOAD_MAX_SIZE )      //单一分包模式
       {
           single_RTP_packet.cc = 0;
           single_RTP_packet.x = 0;
           single_RTP_packet.p = 0;
           single_RTP_packet.version = 2;
           single_RTP_packet.pt = 96;
           single_RTP_packet.m = 1;
           single_RTP_packet.seq = htons(seq_num++);
           single_RTP_packet.ts = htonl(time_stamp);
           single_RTP_packet.ssrc = htonl(1);
           memcpy(single_RTP_packet.payload, nalu_buf, nalu_length);

		   ret = udp_send_data(socket_fd, (char*)(&single_RTP_packet), sizeof(struct_RTP_header)+nalu_length,(struct sockaddr*)&addr_client,sizeof(struct sockaddr_in));
           if(OK != ret)
           {
#ifdef DEBUG          
               echo_error_prompt(udp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
               write_log(udp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
               pthread_exit(udp_send_data_fail);
		   }
	   }
	   else										//分片模式
	   {
            nalu_payload_buf = nalu_buf + 1;        //加1跳过NALU头，如67 
            nalu_payload_length = nalu_length + 1;
            last_slice_length = nalu_payload_length%PAYLOAD_MAX_SIZE; 
	        slices_count = nalu_payload_length/PAYLOAD_MAX_SIZE;  
	        for(i=1; i<=slices_count; i++)
	  	    {	
	            FU_A_rtp_packet.payload.fu_indicator.F = *nalu_buf & 0x80>>7;    //封装 FU indicator
	  	    	FU_A_rtp_packet.payload.fu_indicator.NRI = *nalu_buf & 0x60>>5;  //fu_indicator的F|NRI 是原NALU的F|NRI
	  	    	FU_A_rtp_packet.payload.fu_indicator.Type = 28;
             
                if(1 == i)                                            //封装 FU header
                { 
				    FU_A_rtp_packet.payload.fu_header.s = 1; 
                } 
				else
                { 
				    FU_A_rtp_packet.payload.fu_header.s = 0;
	            } 
                if((0 == last_slice_length)&&(slices_count == i))     //是否为最后一个分片                                
                { 
                    FU_A_rtp_packet.payload.fu_header.e = 1; 
                } 
				else
                { 
				    FU_A_rtp_packet.payload.fu_header.e = 0;
                } 
             
			    FU_A_rtp_packet.payload.fu_header.r = 0;
                FU_A_rtp_packet.payload.fu_header.Type = *nalu_buf & 0x1f;   //fu_header的type是原NALU的type
             
                memcpy(FU_A_rtp_packet.payload.payload_data, nalu_payload_buf+next_frame, PAYLOAD_MAX_SIZE);
                next_frame=next_frame+PAYLOAD_MAX_SIZE;
			 
                FU_A_rtp_packet.cc = 0;
                FU_A_rtp_packet.x = 0;
                FU_A_rtp_packet.p = 0;
                FU_A_rtp_packet.version = 2;
                FU_A_rtp_packet.pt = 96;
                if((0 == last_slice_length)&&(slices_count == i))     //是否为最后一个分片
			    {                           
			        FU_A_rtp_packet.m = 1;    
                }
                else
				{ 
                    FU_A_rtp_packet.m = 0;
                } 
				FU_A_rtp_packet.seq = htons(seq_num++);
                FU_A_rtp_packet.ts = htonl(time_stamp);
                FU_A_rtp_packet.ssrc = htonl(1);
			 
			    ret = udp_send_data(socket_fd, (char*)(&FU_A_rtp_packet), sizeof(struct_RTP_header)+sizeof(FU_header)+sizeof(FU_indicator)+PAYLOAD_MAX_SIZE,(struct sockaddr*)&addr_client,sizeof(struct sockaddr_in)); 
                if(OK != ret)
                {
#ifdef DEBUG          
                    echo_error_prompt(udp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(udp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                    pthread_exit(udp_send_data_fail);
		        }
			}//for
		    
		    if(0 == last_slice_length)   //如果帧长度是1400的整数倍 
			{                            //则上面的for循环已经将一帧数据发送完毕
			    continue;                //中断循环，准备取下一帧数据
            }
            FU_A_rtp_packet.payload.fu_indicator.F = *nalu_buf & 0x80>>7;    //封装 FU indicator
 	    	FU_A_rtp_packet.payload.fu_indicator.NRI = *nalu_buf & 0x60>>5;  //fu_indicator的F|NRI 是原NALU的F|NRI 
 	    	FU_A_rtp_packet.payload.fu_indicator.Type = 28;
             
            FU_A_rtp_packet.payload.fu_header.s = 0;                                
            FU_A_rtp_packet.payload.fu_header.e = 1;
            FU_A_rtp_packet.payload.fu_header.r = 0;
            FU_A_rtp_packet.payload.fu_header.Type = *nalu_buf & 0x1f;
             
            memcpy(FU_A_rtp_packet.payload.payload_data, nalu_payload_buf+next_frame, last_slice_length);
            next_frame=next_frame+last_slice_length;
		     
            FU_A_rtp_packet.cc = 0;
            FU_A_rtp_packet.x = 0;
            FU_A_rtp_packet.p = 0;
            FU_A_rtp_packet.version = 2;
            FU_A_rtp_packet.pt = 96;
            FU_A_rtp_packet.m = 1;
            FU_A_rtp_packet.seq = htons(seq_num++);
            FU_A_rtp_packet.ts = htonl(time_stamp);
            FU_A_rtp_packet.ssrc = htonl(1);
		     
		    ret = udp_send_data(socket_fd, (char*)(&FU_A_rtp_packet), sizeof(struct_RTP_header)+sizeof(FU_header)+sizeof(FU_indicator)+last_slice_length,(struct sockaddr*)&addr_client,sizeof(struct sockaddr_in));
            if(OK != ret)
            {
#ifdef DEBUG          
                echo_error_prompt(udp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(udp_send_data_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                pthread_exit(udp_send_data_fail);
		    }

		}//else
	}//while 

    pthread_cleanup_pop(0);
    free(nalu_buf);
	close(socket_fd);
	pthread_exit(OK);
}


