#ifndef _RTP_H__
#define _RTP_H__


#include <stdio.h>
#include <stdlib.h>
#include "video.h"



#define		PAYLOAD_MAX_SIZE	1400

#pragma pack(1)

typedef struct RTP_HEADER
{
    unsigned char cc:4; /* CSRC count */
    unsigned char x:1; /* header extension flag */
    unsigned char p:1; /* padding flag */
    unsigned char version:2;    /*protocol version*/
    unsigned char pt:7; /* payload type */
    unsigned char m:1; /* marker bit */
    short int seq; /* sequence number */
    unsigned int ts; /* timestamp */
    unsigned int ssrc; /* synchronization source */
}struct_RTP_header;

typedef struct my_fu_indicator
{
	unsigned int Type:5;
	unsigned int NRI:2;
	unsigned int F:1;
}FU_indicator;

typedef struct my_fu_header
{
	unsigned int Type:5;
	unsigned int r:1;
	unsigned int e:1;
	unsigned int s:1;
}FU_header;

typedef struct my_payload
{
    FU_indicator fu_indicator;
    FU_header fu_header;  
    char payload_data[1400]; 
}Fragment_Payload;

typedef struct RTP_SINGLE_packet
{
    unsigned char cc:4; /* CSRC count */
    unsigned char x:1; /* header extension flag */
    unsigned char p:1; /* padding flag */
    unsigned char version:2;    /*protocol version*/
    unsigned char pt:7; /* payload type */
    unsigned char m:1; /* marker bit */
    short int seq; /* sequence number */
    unsigned int ts; /* timestamp */
    unsigned int ssrc; /* synchronization source */
    char payload[1400]; 
}struct_single_RTP_packet;


typedef struct RTP_FU_A_packet
{
    unsigned char cc:4; /* CSRC count */
    unsigned char x:1; /* header extension flag */
    unsigned char p:1; /* padding flag */
    unsigned char version:2;    /*protocol version*/
    unsigned char pt:7; /* payload type */
    unsigned char m:1; /* marker bit */
    short int seq; /* sequence number */
    unsigned int ts; /* timestamp */
    unsigned int ssrc; /* synchronization source */
    Fragment_Payload payload;
}struct_fragment_RTP_packet;

#pragma pack()

typedef struct RTP_PTHREAD_PARAM
{
	struct sockaddr_in *addr_client;
	unsigned short client_port;
	struct_frame_buffer *frame_buffer_info;
}struct_rtp_pthread_param;

typedef struct free_rtp_param
{
    int  socket_fd;
    char *nalu_buf;
}struct_free_rtp_param;

void *rtp_pthread( void *info );

#endif
