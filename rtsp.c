/*
 * rtsp_server.c
 *
 *  Created on: 2014-7-21
 *      Author: hexinjiang
 */
#include <regex.h>
#include "public.h"
#include "rtsp.h"
#include "video.h"

char *RTSP_INFO[]=
{
	"RTSP/1.0 200 OK \r\n",
	"Server: COPYRIGHT@hxj\r\n",
	"CSeq: ",
	"Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n\r\n",
	"Date: ",
	"Content-Base: ",
	"Content-Type: application/sdp\r\n",
	"Content-Length: ",
	"User-Agent: ",
	"Transport: ",
	"Session: ",
	"Range: ",
	"Expries: ",
	"RTP-Info: url="
};

static inline int base64_encoded_len ( int raw_len ) 
{
    return ( ( ( raw_len + 3 - 1 ) / 3 ) * 4 );
}

void base64_encode ( unsigned char *raw, int len, char *encoded ) 
{
	char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char *raw_bytes = ( ( unsigned char * ) raw );
    unsigned char *encoded_bytes = ( ( unsigned char * ) encoded );
    int raw_bit_len = ( 8 * len );
    unsigned int bit;
    unsigned int byte;
    unsigned int shift;
    unsigned int tmp;

    for ( bit = 0 ; bit < raw_bit_len ; bit += 6 ) 
    {
        byte = ( bit / 8 );
        shift = ( bit % 8 );
        tmp = ( raw_bytes[byte] << shift );
        if ( ( byte + 1 ) < len )
            tmp |= ( raw_bytes[ byte + 1 ] >> ( 8 - shift ) );
        tmp = ( ( tmp >> 2 ) & 0x3f );
        *(encoded_bytes++) = base64[tmp];
    }
    for ( ; ( bit % 8 ) != 0 ; bit += 6 )
        *(encoded_bytes++) = '=';
    *(encoded_bytes++) = '\0';

}

static inline int base16_encoded_len ( size_t raw_len ) 
{
    return ( 2 * raw_len );
}

void base16_encode ( unsigned char *raw, size_t len, char *encoded ) 
{
    unsigned char *raw_bytes = raw;
    char *encoded_bytes = encoded;
    int remaining = len;
 
    /* Encode each byte */
    for ( ; remaining-- ; encoded_bytes += 2 ) 
	{
        sprintf ( encoded_bytes, "%02x", *(raw_bytes++) );
    }
 
    /* Ensure terminating NUL exists even if length was zero */
    *encoded_bytes = '\0';
    
}

int Handle_OPTIONS( char* send_buf, char *recv_buf_rtsp )
{
	strcat( send_buf, RTSP_INFO[0] );	 //"RTSP/1.0 200 ok \r\n"
	strcat( send_buf, RTSP_INFO[1] );	 //server:
	char *ptr_start ;
	char *ptr_end ;
	if( NULL!=(ptr_start=(strstr( recv_buf_rtsp, "CSeq: " ))) )
	{
		if( NULL!=(ptr_end = strstr( ptr_start,"\r\n")) )
		{
			strncat( send_buf, ptr_start, ptr_end-ptr_start );	 //Cseq:
			strcat( send_buf, "\r\n");
			strcat( send_buf, RTSP_INFO[3] );
			return OK;
		}
		else
		{
#ifdef DEBUG          
            echo_error_prompt(scan_option_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(scan_option_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return scan_option_tail_fail;
		}
	}
	else
	{
#ifdef DEBUG          
            echo_error_prompt(scan_option_header_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(scan_option_header_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return scan_option_header_fail;
	}
}

/***************************************************************************
* 函数名称： Handle_DESCRIBE
* 功能描述： 生成DESCRIBE会话的返回消息
* 参 数：    send_buf        发送数据的缓存
* 参 数：    recv_buf_rtsp   接收vlc消息的缓存
* 参 数：    sps             视频的sps数据缓存
* 参 数：    length_of_sps   视频的sps数据大小
* 参 数：    pps             视频的pps数据缓存
* 参 数：    length_of_pps   视频的pps数据大小
* 返回值：   成功:               
                  OK
             失败:               

* 其它说明： 
* 修改日期        修改人        修改内容
* --------------------------------------------------------------------------
  2014-07-21      何新江        创建
  2014-10-15      高强          修改内存泄漏bug，并增加自动计算sprop-parameter-sets等视频属性的代码
  2014-11-03      高强          使用get_rtsp_uri替换get_rtsp_content_base
****************************************************************************/
int Handle_DESCRIBE( char* send_buf, char *recv_buf_rtsp, char *sps, unsigned int length_of_sps, char *pps, unsigned int length_of_pps )
{
	strcat( send_buf, RTSP_INFO[0] );	 //"RTSP/1.0 200 ok \r\n"
	strcat( send_buf, RTSP_INFO[1] );	 //server:	
        char *ptr_start, *ptr_end;
	char server_ip[50]={0};
	char time[50] = {0};
	char sdp_info[1024] = {0};
	char sdp_info_len[50]={0};
	char content_base[50]={0};
	char sdp_media_info[500] = {0};
    char *base64_sps = calloc(base64_encoded_len(length_of_sps - H264_START_CODE_LENGTH)+1, sizeof(char));//加1防止越界
    char *base64_pps = calloc(base64_encoded_len(length_of_pps - H264_START_CODE_LENGTH)+1, sizeof(char));
    char *base16_profile_level_id = calloc(base16_encoded_len(3)+1, sizeof(char));        //取sps数据的前3字节
	
	if( NULL!=(ptr_start=(strstr( recv_buf_rtsp, "CSeq: " ))) )
	{
		if( NULL!=(ptr_end = strstr( ptr_start,"\r\n")) )
		{
			strncat( send_buf, ptr_start, ptr_end-ptr_start );	 //Cseq:
			strcat( send_buf, "\r\n");

			strcat( send_buf, RTSP_INFO[4]);	 //Date:
			Get_Time(time);
			strcat( send_buf, time);

			strcat( send_buf, RTSP_INFO[5] ); //content-base
			if( 0 != get_rtsp_uri( recv_buf_rtsp, content_base ))
			{
				free(base64_sps);
			    free(base64_pps);
				free(base16_profile_level_id);
#ifdef DEBUG          
                echo_error_prompt(get_rtsp_uri_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(get_rtsp_uri_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                return get_rtsp_uri_fail;
			}
			strcat( send_buf, content_base );
			strcat( send_buf, "/\r\n" );

			if( 0 != get_server_ip( recv_buf_rtsp, server_ip ))
			{
			    free(base64_sps);
			    free(base64_pps);
				free(base16_profile_level_id);
#ifdef DEBUG          
                echo_error_prompt(get_server_ip_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(get_server_ip_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                return get_server_ip_fail;
			}
			sprintf(sdp_info,
			"v=0\r\no=NVRStream %d %d IN IP4 %s\r\ns=H.264 video streamed by NVR\r\nt=0 0\r\na=control:*\r\na=range:npt=now-\r\nm=video 0 RTP/AVP 96\r\n"
			,Get_NTP(),Get_NTP(),server_ip);	 //sdp info
			strcat( sdp_info, "c=IN IP4 0.0.0.0\r\n" );
			strcat( sdp_info,"a=rtpmap:96 H264/90000\r\n" );

			//strcat( sdp_info,"a=fmtp:96 profile-level-id=640028;packetization-mode=1;sprop-parameter-sets=MDAwMGg6IDAwIA==,MDAgMA==\r\n");
            base64_encode( sps + H264_START_CODE_LENGTH, length_of_sps - H264_START_CODE_LENGTH, base64_sps );
	        base64_encode( pps + H264_START_CODE_LENGTH, length_of_pps - H264_START_CODE_LENGTH, base64_pps );
	        base16_encode( sps + H264_START_CODE_LENGTH + 1, 3, base16_profile_level_id ); //去掉nalu头的一字节
			sprintf(sdp_media_info,"a=fmtp:96 profile-level-id=%s;packetization-mode=1;sprop-parameter-sets=%s,%s\r\n",base16_profile_level_id,base64_sps,base64_pps);
            strcat( sdp_info,sdp_media_info);
			free(base64_sps);
			free(base64_pps);
			free(base16_profile_level_id);
			
			strcat( sdp_info,"a=control:streamid=0\r\n" );

			strcat( send_buf, RTSP_INFO[6] ); //content-type
			strcat( send_buf, RTSP_INFO[7] ); //content-length
			sprintf(sdp_info_len,"%d",strlen(sdp_info));
			strcat( send_buf, sdp_info_len );
			strcat( send_buf, "\r\n\r\n" );

			strcat( send_buf, sdp_info ); //sdp

			return OK;
			}
			else
			{
#ifdef DEBUG          
                echo_error_prompt(scan_describe_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(scan_describe_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                return scan_describe_tail_fail;
			}

	}
	else
	{
#ifdef DEBUG          
        echo_error_prompt(scan_describe_header_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(scan_describe_header_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return scan_describe_header_fail;
	}

}

int get_transport_info( char *recv_buf_rtsp, char *transport_info )
{
	char* ptr_start;
	char* ptr_end ;

	if( NULL!=(ptr_start=strstr(recv_buf_rtsp,"Transport: ")) )
	{
		if( NULL!=(ptr_end=strstr(ptr_start,"\r\n")) )
		{
			memcpy( transport_info, ptr_start, ptr_end-ptr_start);
			return OK;
		}
		else
		{
#ifdef DEBUG          
            echo_error_prompt(scan_transport_info_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(scan_transport_info_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return scan_transport_info_tail_fail;
		}

	}
	else
	{
#ifdef DEBUG          
        echo_error_prompt(scan_transport_info_header_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(scan_transport_info_header_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return scan_transport_info_header_fail;
	}

}

int Handle_SETUP( char* send_buf, char *recv_buf_rtsp, char *session_id )
{

	strcat( send_buf, RTSP_INFO[0] );	 //"RTSP/1.0 200 ok \r\n"
	strcat( send_buf, RTSP_INFO[1] );	 //server:

	char *ptr_start, *ptr_end;
	char time[50] = {0};
	char transport_info[50]={0};


	if( NULL!=(ptr_start=(strstr( recv_buf_rtsp, "CSeq: " ))) )
	{
		if( NULL!=(ptr_end = strstr( ptr_start,"\r\n")) )
		{
		strncat( send_buf, ptr_start, ptr_end-ptr_start );	 //Cseq:
		strcat( send_buf, "\r\n");

		strcat( send_buf, RTSP_INFO[10] );	 //Session
		strcat( send_buf, session_id );
		strcat( send_buf, "\r\n" );

		strcat( send_buf, RTSP_INFO[4]);	 //Date:
		Get_Time(time);
		strcat( send_buf, time);

		strcat( send_buf, RTSP_INFO[12]);	 //Expries
		strcat( send_buf, time);


		//strcat( send_buf, RTSP_INFO[9] );	 //Transport
		if( 0 != get_transport_info( recv_buf_rtsp, transport_info ) )
		{
#ifdef DEBUG          
            echo_error_prompt(get_transport_info_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(get_transport_info_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return get_transport_info_fail;
		}
		strcat( send_buf, transport_info );
		strcat( send_buf,";server-port=5004-5005\r\n\r\n");

		return OK;
		}
		else
		{
#ifdef DEBUG          
            echo_error_prompt(scan_setup_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(scan_setup_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return scan_setup_tail_fail;
		}
	}
	else
	{
#ifdef DEBUG          
        echo_error_prompt(scan_setup_header_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(scan_setup_header_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return scan_setup_header_fail;
	}
}

int Handle_PLAY( char* send_buf, char *recv_buf_rtsp, char *session_id)
{
	strcat( send_buf, RTSP_INFO[0] );	 //"RTSP/1.0 200 ok \r\n"
	strcat( send_buf, RTSP_INFO[1] );	 //server:
	char *ptr_start, *ptr_end;
	char url[100] ={ 0 };
	if( NULL!=(ptr_start=(strstr( recv_buf_rtsp, "CSeq: " ))) )
	{
		if( NULL!=(ptr_end=strstr(ptr_start,"\r\n")) )
		{
			strncat( send_buf, ptr_start, ptr_end-ptr_start );	 //Cseq:
			strcat( send_buf, "\r\n");

			strcat( send_buf, RTSP_INFO[10] );	 //Session
			strcat( send_buf, session_id );
			strcat( send_buf, "\r\n" );

			strcat( send_buf, RTSP_INFO[13] );
			if(0 != get_rtsp_uri( recv_buf_rtsp, url ))
			{
#ifdef DEBUG          
                echo_error_prompt(get_rtsp_uri_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(get_rtsp_uri_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                return get_rtsp_uri_fail;
			}
			strcat( send_buf, url );
			strcat( send_buf, "/trackID=1;seq=0;rtptime=0\r\n\r\n" );
			return OK;
		}
		else
		{
#ifdef DEBUG          
            echo_error_prompt(scan_play_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(scan_play_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return scan_play_tail_fail;
		}
	}
	else
	{
#ifdef DEBUG          
            echo_error_prompt(scan_play_header_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(scan_play_header_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return scan_play_header_fail;
	}

}

int Handle_TEARDOWN ( char *send_buf, char *recv_buf_rtsp, char *session_id )
{
	strcat( send_buf, RTSP_INFO[0] );	 //"RTSP/1.0 200 ok \r\n"
	strcat( send_buf, RTSP_INFO[1] );	 //server:
	char *ptr_start, *ptr_end;
		if( NULL!=(ptr_start=(strstr( recv_buf_rtsp, "CSeq: " ))) )
		{
            if( NULL!=(ptr_end=strstr(ptr_start,"\r\n")) )
            {
                strncat( send_buf, ptr_start, ptr_end-ptr_start );	 //Cseq:
                strcat( send_buf, "\r\n");

                strcat( send_buf, RTSP_INFO[10] );	 //Session
                strcat( send_buf, session_id );
                strcat( send_buf, "\r\n" );

                strcat( send_buf, "Connection: Close \r\n\r\n" );       //Connection::close

                return OK;
            }
            else
            {
#ifdef DEBUG          
                echo_error_prompt(scan_teardown_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(scan_teardown_tail_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                return scan_teardown_tail_fail;
            }
		}
		else
		{
#ifdef DEBUG          
            echo_error_prompt(scan_teardown_header_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(scan_teardown_header_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return scan_teardown_header_fail;
		}

}

int regex_pattern_match_string( char *source_string, char *dest_string, char *pattern)
{
    int        ret          = 0;
    regex_t    regex;
	regmatch_t string_position;


	ret = regcomp(&regex, pattern, REG_EXTENDED);
	if(ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(compile_regex_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(compile_regex_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        regfree(&regex);
        return compile_regex_fail;	
	}

    ret = regexec(&regex, source_string, 1, &string_position, 0);
	if(ret == REG_NOMATCH)
	{
#ifdef DEBUG          
        echo_error_prompt(regex_match_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(regex_match_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        regfree(&regex);
        return regex_match_fail;	
	}

    memcpy (dest_string, source_string + string_position.rm_so, string_position.rm_eo - string_position.rm_so);

	regfree(&regex);
	return OK;
}

int get_port( char *recv_buf_rtsp, unsigned short *client_port )
{
    int  ret = 0;
	char client_port_string[10]={0};
		
	ret = sscanf(recv_buf_rtsp,"%*[^_]_port=%[^-]-",client_port_string);
    if(ret <= 0)
    {
#ifdef DEBUG          
        echo_error_prompt(sscanf_client_port_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(sscanf_client_port_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return sscanf_client_port_fail;
	}

    *client_port = (unsigned short)strtoul(client_port_string,NULL,10);
	
	return OK;
}

void Get_Time( char * time_str )
{
	time_t	 now;
	struct tm *timenow;

	time(&now);
	timenow = localtime(&now);
	strncat( time_str, asctime(timenow), strlen(asctime(timenow))-1);
	strcat( time_str, " GMT\r\n" );
}

int Get_NTP( void )
{
	time_t	 timenow;
	time(&timenow);
	return timenow;
}

int get_server_ip( char *rtsp_info, char *server_ip )
{
    int ret = 0;

	ret = regex_pattern_match_string( rtsp_info, server_ip, "([0-9]{1,3}[.]){3}[0-9]{1,3}");
    if(ret != OK)
    {
#ifdef DEBUG          
        echo_error_prompt(match_server_ip_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(match_server_ip_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return match_server_ip_fail;
    }
	
	return OK;
}

void Create_Session_ID( char *session_id)
{
	long id=0;
	long Randmax=999999999;
	srand((unsigned)time(NULL));
	id=10000000+rand()%Randmax;
	sprintf(session_id,"%ld",id);
}

int get_rtsp_uri( char *recv_buf_rtsp, char *rtsp_uri )
{
    int ret = 0;

	ret = regex_pattern_match_string( recv_buf_rtsp, rtsp_uri, "rtsp://(([0-9]{1,3}[.]){3}[0-9]{1,3}):([0-9]{1,5})");
    if(ret != OK)
    {
#ifdef DEBUG          
        echo_error_prompt(match_rtsp_uri_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(match_rtsp_uri_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return match_rtsp_uri_fail;
    }
	
	return OK;
}
