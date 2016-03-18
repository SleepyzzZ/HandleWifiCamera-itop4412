#include "public.h"

char *error_mesg[] = {
	"OK",
	
	"ftok ipc key fail",
	"get ipc key fail",
	
	"create tcp socket fail",
	"create tcp client socket fail",
	"connect server fail", 
	"set socket reuse addr fail",
	"set socket send timeout fail",
	"set socket nodelay fail",
	"socket bind addr fail",
	"listen socket fail",
	"tcp socket send fail",
	"udp socket send fail",
	"no data to recv",
	"tcp socket recv fail", 
	"tcp connectiong has been closed",
	
	"msgget fail",
	"msgctl fail",
	"msgsnd fail",
	"msgrcv fail,",
	"no message in queue",
	
	"scan option tail fail",
	"scan option header fail",
	"get rtsp uri fail",
	"get server ip fail",
	"scan describe tail fail",
	"scan describe header fail",
	"scan transport info tail fail",
	"scan transport info header fail",
	"get transport info fail",
	"scan setup tail fail",
	"scan setup header fail",
	"scan play tail fail",
	"scan play header fail",
	"scan teardown tail fail",
	"scan teardown header fail",
	"compile regex fail",
	"regex match fail",
	"sscanf client port fail",
	"match server ip fail",
	"match rtsp uri fail",
	
	"get frame mutex lock fail",
	"get frame cond wait fail",
	"create udp socket fail",
	"bind udp addr fail", 
	"get globe frame fail",
	"udp send data fail",
	
	"sem open fail",
	"fork cmd server fail",
	"cmd server running fail",
	"sem wait fail",
	"sem close fail",
	"sem unlink fail",
	"fork video server fail",
	"video server running fail",
	"fork photo trans client fail",
	"photo trans client running fail",
	
	"get addr shm id fail",
	"map addr share mem fail",
	"create message queue fail",
	"sem post fail",
	"init cmd socket fail",
	"accept cmd socket fail",
	"recv cmd fail",
	"recv cmd data length fail",
	"recv cmd data fail",
	"send message to queue fail",
	"take syn time fail",
	"send syn time echo fail",
	"dispose syn time fail",
	"dispose take photo fail",
	"recv unknow cmd",
	"tcp send data fail",
	"send take photo cmd echo fail",
	"recv message from queue fail",
	"check take photo results fail",
	"send send photo cmd echo fail",
	
	"init rtsp socket fail",
	"accept rtsp socket fail",
	"get client port fail",
	"create video pthread fail",
	"open camera fail",
	"poll_fail",
	"poll_time_out", 
	"ioctl VIDIOC_QUERYCAP fail",
	"no capture devices", 
	"ioctl VIDIOC_ENUMINPUT fail",
	"ioctl VIDIOC_S_INPUT fail",
	"ioctl VIDIOC_S_FMT_format fail",
	"ioctl ISP_VIDIOC_S_FMT_format fail",
	"ioctl VIDIOC_ENUM_FMT fail",
	"ioctl VIDIOC_REQBUFS fail",
	"ioctl VIDIOC_QUERYBUF fail",
	"memory map buffers fail",
	"ioctl_VIDIOC_QBUF_fail",
	"ioctl VIDIOC_STREAMON fail",
	"ioctl VIDIOC_STREAMOFF fail",
	"ioctl VIDIOC_DQBUF fail",
	"ioctl VIDIOC_S_CTRL fail",
	"v4l2 query_capacity fail",
	"v4l2 enum input fail",
	"v4l2 set input channel fail",
	"v4l2 enum format fail",
	"v4l2 set camera format fail",
	"v4l2 set isp format fail",
	"v4l2 set ctrl fail",
	"v4l2 require buffers fail",
	"v4l2 query buffers fail",
	"v4l2 queue buffer fail",
	"v4l2 stream on fail",
	"v4l2 stream off fail",
	"clear v4l2 mmap fail",
	"clear v4l2 stream mmap fail",
	"h264 encode close fail",
	"open h264 encode fail",
	"init h264 encode fail",
	"h264 encode get in buffer fail",
	"h264 encode get out buffer fail",
	"scan frame start code fail",
	"get sps and pps from header fail",
	"video sem post fail",
	"sem wait video pthread fail",
	"recv rtsp command fail",
	"create rtp thread fail",
	"pthread mutex lock fail",
	"pthread_cond_wait_fail",
	"update globe frame info fail",
	"poll yuv buffer fail",
	"v4l2 dequeue buffer fail",
	"h264 encode set in buffer fail", 
	"h264 encode encode buffer fail",
	"reset v4l2 stream fail",
	"calloc memory fail",
	"convert NV12 to IYUV fail",
	"mkdir fail",
	"open picture file fail",
	"fwrite fail",
	"lstat photo file fail",
	"take pohto fail",
	"return trans result to message queue fail",
	"dispose send file fail",
	"clear video pthread mem fail",
	
	"can_not_find_image_file",
	"send photo data fail",
	"read photo file fail", 
	
	"Unknown error"
};

void echo_error_prompt(unsigned int i_error_code, const char *errro_in_file, const char *error_in_function, int error_in_line)
{
    if( i_error_code < ENUM_ERROR_CODE_LIST_END )
    {
        fprintf(stdout, "In FILE: %s FUNCTION: %s() LINE: %d ERROR: %s, error code is : %d!\n", \
                errro_in_file, error_in_function, error_in_line, error_mesg[i_error_code], i_error_code);    //自报错位置向前移动3行才是出错行
    }
    else
    {
        fprintf(stdout, "Unknown error, encountered a fatal problem !\n");
    }
    return;
}

void write_log(unsigned int i_error_code, const char *errro_in_file, const char *error_in_function, int error_in_line)
{
 
    openlog("WIFI_CAMERA_FOR_CNRRI", LOG_CONS | LOG_PID, 0); 
    syslog(LOG_DEBUG,"In FILE: %s FUNCTION: %s() LINE: %d ERROR: %s, error code is : %d!\n", \
           errro_in_file, error_in_function, error_in_line, error_mesg[i_error_code], i_error_code); 
    closelog(); 
    return;
}

int get_ipc_key(key_t *key, int seed)
{
    *key = ftok( "./", seed );
    if(-1 == *key)
    {
#ifdef DEBUG
        echo_error_prompt(ftok_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ftok_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return ftok_ipc_key_fail; 
    }

    return OK;
}
