#include "public.h"
#include "net_trans.h"

/***************************************************************************
* �������ƣ� init_tcp_server_socket
* ���������� ��ʼ��tcp�׽��� 
* �� ����    socket_fd         	�׽��־��
* �� ����    port               �󶨶˿ں� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  create_tcp_socket_fail
                  set_socket_reuse_addr_fail
                  set_socket_send_timeout_fail
                  socket_bind_addr_fail
                  listen_socket_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
****************************************************************************/
int init_tcp_server_socket(int *socket_fd, unsigned short port)
{
	char reuse_flag 	= 1;		//��ַ����
	char nodelay_flag 	= 1;		//Nagle�㷨
	
	struct timeval timeout = {10, 0};
	struct sockaddr_in server_addr;
	
	if((*socket_fd = (socket(AF_INET, SOCK_STREAM, 0))) == -1)
	{
#ifdef DEBUG          
        echo_error_prompt(create_tcp_server_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(create_tcp_server_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return create_tcp_server_socket_fail;
	}
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family 		= AF_INET;
	server_addr.sin_port		= htons(port);
	server_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	
 	if(setsockopt(*socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse_flag, sizeof(int)) < 0)
 	{
#ifdef DEBUG          
        echo_error_prompt( set_socket_reuse_addr_fail, __FILE__, __FUNCTION__, __LINE__ );
#else
        write_log( set_socket_reuse_addr_fail, __FILE__, __FUNCTION__, __LINE__ );
#endif   
	    close(*socket_fd);
        return set_socket_reuse_addr_fail;		
 	}
 	
 	/*if(*socket_fd��SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(struct timeval)) < 0)
 	{
#ifdef DEBUG          
        echo_error_prompt(set_socket_send_timeout_fail);
#else
        write_log(set_socket_send_timeout_fail);
#endif   
	    close(*socket_fd);
        return set_socket_send_timeout_fail;	 	
	}*/
	
 	if(setsockopt(*socket_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay_flag, sizeof(int)) < 0)
    {
#ifdef DEBUG          
        echo_error_prompt( set_socket_nodelay_fail, __FILE__, __FUNCTION__, __LINE__ );
#else
        write_log( set_socket_nodelay_fail, __FILE__, __FUNCTION__, __LINE__ );
#endif   
	    close(*socket_fd);
        return set_socket_nodelay_fail;
    }
    
    if((bind(*socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1)
    {
#ifdef DEBUG          
        echo_error_prompt( socket_bind_addr_fail, __FILE__, __FUNCTION__, __LINE__ );
#else
        write_log( socket_bind_addr_fail, __FILE__, __FUNCTION__, __LINE__ );
#endif   
	    close(*socket_fd);
        return socket_bind_addr_fail;    	
    }
    
    if((listen(*socket_fd, 20)) == -1)
	{
#ifdef DEBUG          
        echo_error_prompt( listen_socket_fail, __FILE__, __FUNCTION__, __LINE__ );
#else
        write_log( listen_socket_fail, __FILE__, __FUNCTION__, __LINE__ );
#endif   
	    close(*socket_fd);
        return listen_socket_fail;
	}
	
	return OK;
}

/***************************************************************************
* �������ƣ� init_tcp_client_socket
* ���������� ��ʼ��tcp�ͻ����׽��� 
* �� ����    socket_fd         	�׽��־��
* �� ����    server_ip_addr     �������˵�ַ 
* �� ����    port               �󶨶˿ں� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ...
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-10      �ų�          ����
****************************************************************************/
int init_tcp_client_socket(int *socket_fd, in_addr_t server_ip_addr, unsigned short port)
{
	int ret       = 0;
    struct sockaddr_in server_addr;

    if( -1 == (*socket_fd=(socket(AF_INET,SOCK_STREAM,0))) )
    {
#ifdef DEBUG
        echo_error_prompt(create_tcp_client_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(create_tcp_client_socket_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return create_tcp_client_socket_fail;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = server_ip_addr;
    memset(&(server_addr.sin_zero), 0, 8);

    if( 0 != connect(*socket_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
#ifdef DEBUG
        echo_error_prompt(connect_server_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(connect_server_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        close(*socket_fd);
        return connect_server_fail;
    }

    return OK;
}

/***************************************************************************
* �������ƣ� tcp_send_data
* ���������� tcp�������� 
* �� ����    socket_fd         	�׽��־��
* �� ����    buff               ��Ϣ����
* �� ����    buff_len           ��Ϣ���� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  tcp_socket_send_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
****************************************************************************/
int tcp_send_data(int socket_fd, char *buff, int buff_len)
{
	int ret 			= 0;
	int send_len 		= 0;
	int have_send_len 	= 0; 
	
	while(0 != buff_len)
	{
		send_len = send(socket_fd, buff+have_send_len, buff_len, 0);
		if(0 > send_len)
		{
			if(errno == EINTR||errno == EAGAIN)
			{
				continue;
			} else {
#ifdef DEBUG          
                echo_error_prompt( tcp_socket_send_fail, __FILE__, __FUNCTION__, __LINE__ );
#else
                write_log( tcp_socket_send_fail, __FILE__, __FUNCTION__, __LINE__ );
#endif   
                return tcp_socket_send_fail;				
			}	
		}
		
		have_send_len += send_len;
		buff_len -= send_len;
	}
	
	return OK;
}

/***************************************************************************
* �������ƣ� udp_send_data
* ���������� udp�������� 
* �� ����    socket_fd         	�׽��־��
* �� ����    buff               ��Ϣ����
* �� ����    buff_len           ��Ϣ���� 
* �� ����    addr_client        �ͻ��˵�ַ 
* �� ����    client_addr_len    
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  udp_socket_send_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
****************************************************************************/
int udp_send_data(int socket_fd, char *buff, int buff_len, struct sockaddr* addr_client, int client_addr_len)
{
	int ret 			= 0;
	int send_len 		= 0;
	int have_send_len 	= 0;
	
	while(0 != buff_len)
	{
		send_len = sendto(socket_fd, buff+have_send_len, buff_len, 0, addr_client, client_addr_len);
		if(0 > send_len)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				continue;
			} else {
#ifdef DEBUG          
                echo_error_prompt( udp_socket_send_fail, __FILE__, __FUNCTION__, __LINE__ );
#else
                write_log( udp_socket_send_fail, __FILE__, __FUNCTION__, __LINE__ );
#endif   
                return udp_socket_send_fail;				
			}
		}
		
		have_send_len += send_len;
		buff_len -= send_len;
	}
	
	return OK;
}

/***************************************************************************
* �������ƣ� tcp_recv_data
* ���������� ͨ��tcp socket��������
* �� ����    send_sockfd         �������ݵ�socket���
* �� ����    buff                �������ݵĻ���
* �� ����    buff_len            �������ݵĳ���
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  socket_recv_fail
                
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
****************************************************************************/
int tcp_recv_data(int socket_fd, char *buff, int buff_len, int flag)
{
	int ret 			= 0;
	int recv_len 		= 0;
	int have_recv_len 	= 0;
	
	while(0 != buff_len)
	{
		if(MSG_DONTWAIT == flag)		//����������
		{
			recv_len =  recv(socket_fd, buff+have_recv_len, buff_len, flag);
			if(0 > recv_len)
			{
				if((errno == EAGAIN) && (0 == have_recv_len))
				{
					return no_data_to_recv;
				}
				else if(((errno == EAGAIN) && (0 != have_recv_len)) || (errno == EINTR))
				{
					continue;
				}
				else
				{
#ifdef DEBUG          
                    echo_error_prompt(tcp_socket_recv_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(tcp_socket_recv_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                    return tcp_socket_recv_fail;					
				}
			}
			else if(0 == recv_len)
			{
#ifdef DEBUG          
                echo_error_prompt(tcp_connectiong_has_been_closed, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(tcp_connectiong_has_been_closed, __FILE__, __FUNCTION__, __LINE__);
#endif   
                return tcp_connectiong_has_been_closed;				
			}
		} 
		
		if(0 == flag)		//��������
		{
			recv_len =  recv(socket_fd, buff+have_recv_len, buff_len, flag);
		 	if(0 > recv_len)
            {    
			    if(errno == EINTR || errno == EAGAIN)   
                {
                    continue;
                }
                else
                {
#ifdef DEBUG          
                    echo_error_prompt(tcp_socket_recv_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(tcp_socket_recv_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                    return tcp_socket_recv_fail;
                }
            }
            else if(0 == recv_len)
            {
#ifdef DEBUG          
                echo_error_prompt(tcp_connectiong_has_been_closed, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(tcp_connectiong_has_been_closed, __FILE__, __FUNCTION__, __LINE__);
#endif   
                return tcp_connectiong_has_been_closed;
            }			
		} 
		have_recv_len += recv_len;
		buff_len -= recv_len;
	}
	
	return OK;
}