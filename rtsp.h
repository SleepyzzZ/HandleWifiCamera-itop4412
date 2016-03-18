/*
 * rtsp_server.h
 *
 *  Created on: 2014-7-21
 *      Author: hexinjiang
 */

#ifndef _RTSP_SERVER_H__
#define _RTSP_SERVER_H__

#define RTSP_CMD_BIT_MASK_OPTIONS  			0b00000001
#define RTSP_CMD_BIT_MASK_DESCRIBE 			0b00000010
#define RTSP_CMD_BIT_MASK_SETUP    			0b00000100
#define RTSP_CMD_BIT_MASK_PLAY     			0b00001000
#define RTSP_CMD_BIT_MASK_TEARDOWN 			0b00010000

int 	Handle_OPTIONS( char *send_buf, char *recv_buf_rtsp );

int		Handle_DESCRIBE( char* send_buf, char *recv_buf_rtsp, char *sps, unsigned int length_of_sps, char *pps, unsigned int length_of_pps );

int 	Handle_SETUP( char *send_buf, char *recv_buf_rtsp, char *session_id );

int 	Handle_PLAY( char *send_buf, char *recv_buf_rtsp, char *session_id );

int		Handle_TEARDOWN ( char *send_buf,char *recv_buf_rtsp,char *session_id );

int		get_port( char *recv_buf_rtsp, unsigned short *client_port );

void 	Get_Time( char *time_str);

int 	get_rtsp_uri( char *recv_buf_rtsp, char *url );

int 	get_transport_info( char *recv_buf_rtsp, char *transport_info );

int 	Get_NTP( void );

int 	get_server_ip( char *rtsp_info, char *server_ip );

void 	Create_Session_ID( char *session_id );

#endif /* RTSP_SERVER_H_ */
