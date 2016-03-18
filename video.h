#ifndef _VIDEO_H__
#define _VIDEO_H__

#include "public.h"

#define H264_START_CODE_LENGTH				4
#define NALU_BUFFER_LENGTH					1024*1024
#define MAX_NR_BUFFERS            			8

#define V4L2_BUF_TYPE          				V4L2_BUF_TYPE_VIDEO_CAPTURE
#define V4L2_MEMORY_TYPE       				V4L2_MEMORY_MMAP
#define VIDEO_PIX_FORMAT          	 		V4L2_PIX_FMT_NV12

typedef struct
{
	char            *SPS;
	unsigned int    length_of_SPS;
	char            *PPS;
	unsigned int    length_of_PPS;
    unsigned int    frame_length;
    char            *frame_buffer;
    unsigned int    time_stamp; 
    pthread_mutex_t mtx;
    pthread_cond_t  cond;
	sem_t           sem;
    bool            is_full;
} struct_frame_buffer;

typedef struct VideoBuffer
{
	void *start;
	size_t length;
} VideoBuffer;

typedef struct video_pthread_parm
{
	int cam_fd;
	void *encode_fp;
	VideoBuffer *framebuf;
} struct_video_pthread_param;

void *video_pthread(void *arg);

#endif