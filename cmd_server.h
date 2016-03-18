#ifndef _CMD_SERVER_H__
#define _CMD_SERVER_H__

#include "public.h"

extern sem_t *sem;

#define MAX_CMD_LENGTH				1200

#pragma pack(1)

typedef struct photo_snapshot_info
{
	int 			CMD;
	bool 			is_succeeed;
	char 			photo_name[MAX_PHOTO_NAME_LENGTH];
	unsigned int 	photo_size;
} photo_snapshot;

typedef struct photo_snapshot_outfo
{
	unsigned int	packet_size;
	int				CMD;
	bool			is_succeed;
	char 			photo_name[MAX_PHOTO_NAME_LENGTH];
	unsigned int 	photo_size;
} photo_snapshot_echo; 

typedef struct syn_time_info
{
	int			CMD;
	bool		is_succeed;
	time_t		time;
} syn_time;

typedef struct syn_time_outfo
{
	unsigned int			packet_size;
	int						CMD;
	bool					is_succeed;
} syn_time_echo;

typedef struct photo_send_info
{
	int 			CMD;
	bool 			is_succeed;
	char 			photo_path_name[MAX_PHOTO_PATH_NAME_LENGTH];
	unsigned int 	photo_size;
} photo_send; 

#pragma pack()

/*******************√¸¡Ó∫≈**********************/
#define SYN_TIME_CMD         		 0x01
#define ECHO_SYN_TIME_CMD        	 0x02

#define SET_SYSTEM_TIME_CMD          0x03
#define ECHO_SET_SYSTEM_TIME_CMD     0x04

#define GET_PHOTO_NAME_LIST_CMD      0x05
#define ECHO_GET_PHOTO_NAME_LIST_CMD 0x06

#define TAKE_PHOTO_CMD               0x07 
#define ECHO_TAKE_PHOTO_CMD          0x08

#define DEL_IMAGE_CMD                0x09
#define ECHO_DEL_IMAGE_CMD           0x0A

#define SEND_PHOTO_CMD                0x0B
#define ECHO_SEND_PHOTO_CMD           0x0C

#define FORMAT_SD_CARD_CMD           0x0D
#define ECHO_FORMAT_SD_CARD_CMD      0x0E

int cmd_server();

#endif