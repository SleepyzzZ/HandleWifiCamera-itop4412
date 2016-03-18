#ifndef _MESSAGE_QUEUE_h__
#define _MESSAGE_QUEUE_h__

#define MESSAGE_QUEUE_TAKE_PHOTO_TYPE          			1
#define MESSAGE_QUEUE_TAKE_PHOTO_OVER_TYPE     			2
#define MESSAGE_QUEUE_TRANS_PHOTO_TYPE					3
#define MESSAGE_QUEUE_TRANS_PHOTO_OVER_TYPE     		4

typedef struct msg_buf_take_photo
{
	long type;
	bool succeed;
	char photo_name[MAX_PHOTO_NAME_LENGTH];
	unsigned int photo_size;
}struct_take_photo_msg;

typedef struct msg_buf_send_photo
{
	long type;
	bool succeed;
	char photo_path_name[MAX_PHOTO_PATH_NAME_LENGTH];
	unsigned int photo_size;
} struct_send_photo_msg;

int create_message_queue(int key, int *msg_id, bool is_create_flag);

int del_message_queue(int msg_id);

int send_message_to_queue(int msg_id, void *msg_buf, int msg_buf_length);

int recv_message_from_queue(int msg_id, long msg_type, void *msg_buf, int msg_buf_length, int flag);

#endif