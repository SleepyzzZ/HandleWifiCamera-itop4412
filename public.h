#ifndef _PUBLIC_H__
#define _PUBLIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <syslog.h>
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <pthread.h>
#include <sys/wait.h>
#include <assert.h>
#include <getopt.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <asm/types.h>

#define RTSP_SOCKET_PORT								554			//RTSP端口 
#define RTP_SOCKET_PORT									5004		//RTP端口 
#define CMD_SOCKET_PORT									8080		//命令服务器端口 
#define PHOTO_TRANS_SOCKET_PORT                       	8000		//图片传输端口 

#define MAX_PHOTO_NAME_LENGTH							25			//图片名 
#define MAX_PHOTO_DIR_NAME_LENGTH					  	23			//存放图像的目录名
#define MAX_PHOTO_PATH_NAME_LENGTH						48			//存放图像的目录名+图像名

#define IPC_KEY_SEEK_FOR_CLIENT_ADDR_SHM				0x01		//客户端共享内存key 
#define IPC_KEY_SEED_FOR_TAKE_PHOTO_MSG					0x02		//拍照key 
#define IPC_KEY_SEED_FOR_SEND_PHOTO_MSG					0x03		//图片传输key 
#define SYN_SEM_NAME_FOR_CMD_SERVER_AND_PHOTO_CLIENT  	"syn_sem_for_cmd_and_photo_trans"    //命令服务器和照片发送客户端同步所用posix信号量的名称

#define SD_CARD_MNT						  				"/mnt/sdcard"				//SD卡挂载位置(Android4.4.4)
#define CAMERA_DEVICE									"/dev/video0"				//摄像头设备 
#define CAMERA_PREVIEW_WIDTH              				640                         //预览分辨率
#define CAMERA_PREVIEW_HEIGHT             				480    
#define CAMERA_SNAPSHOT_WIDTH			  				1600						//拍照分辨率 
#define CAMERA_SNAPSHOT_HEIGHT			  				1200

#define LIKELY(x)	                    				__builtin_expect(!!(x), 1)
#define UNLIKELY(x)	                  					__builtin_expect(!!(x), 0)	   

//错误码 
enum error_code_list {
	OK 							= 0,
	
	ftok_ipc_key_fail			= 1,
	get_ipc_key_fail			= 2,
	
	create_tcp_server_socket_fail,
	create_tcp_client_socket_fail,
	connect_server_fail,
	set_socket_reuse_addr_fail,
	set_socket_send_timeout_fail,
	set_socket_nodelay_fail,
	socket_bind_addr_fail,
	listen_socket_fail,
	tcp_socket_send_fail,
	udp_socket_send_fail,
	no_data_to_recv,
	tcp_socket_recv_fail,
	tcp_connectiong_has_been_closed,
	
	msgget_fail,
	msgctl_fail,
	msgsnd_fail,
	msgrcv_fail,
	no_message_in_queue,
	
	scan_option_tail_fail,
	scan_option_header_fail,
	get_rtsp_uri_fail,
	get_server_ip_fail,
	scan_describe_tail_fail,
	scan_describe_header_fail,
	scan_transport_info_tail_fail,
	scan_transport_info_header_fail,
	get_transport_info_fail,
	scan_setup_tail_fail,
	scan_setup_header_fail,
	scan_play_tail_fail,
	scan_play_header_fail,
	scan_teardown_tail_fail,
	scan_teardown_header_fail,
	compile_regex_fail,
	regex_match_fail,
	sscanf_client_port_fail,
	match_server_ip_fail,
	match_rtsp_uri_fail, 
	
	get_frame_mutex_lock_fail,
	get_frame_cond_wait_fail,
	create_udp_socket_fail,
	udp_bind_addr_fail,
	get_globe_frame_fail,
	udp_send_data_fail,
	
	sem_open_fail,
	fork_cmd_server_fail,
	cmd_server_running_fail,
	sem_wait_fail,
	sem_close_fail,
	sem_unlink_fail,
	fork_video_server_fail,
	video_server_running_fail,
	fork_photo_trans_client_fail,
	photo_trans_client_running_fail, 
	
	get_addr_shm_id_fail,
	map_addr_share_mem_fail,
	create_message_queue_fail,
	sem_post_fail,
	init_cmd_socket_fail,
	accept_cmd_socket_fail,
	recv_cmd_fail,
	recv_cmd_data_length_fail,
	recv_cmd_data_fail,
	send_message_to_queue_fail,
	take_syn_time_fail,
	send_syn_time_echo_fail,
	dispose_syn_time_fail,
	dispose_take_photo_fail,
	recv_unknow_cmd,
	tcp_send_data_fail,
	send_take_photo_cmd_echo_fail,
	recv_message_from_queue_fail,
	check_take_photo_results_fail,
	send_send_photo_cmd_echo_fail,
	
	init_rtsp_socket_fail,
	accept_rtsp_socket_fail,
	get_client_port_fail,
	create_video_pthread_fail,
	open_camera_fail,
	poll_fail,
	poll_time_out,
	ioctl_VIDIOC_QUERYCAP_fail,
	no_capture_devices,
	ioctl_VIDIOC_ENUMINPUT_fail,
	ioctl_VIDIOC_S_INPUT_fail,
	ioctl_VIDIOC_S_FMT_format_fail,
	ioctl_ISP_VIDIOC_S_FMT_format_fail,
	ioctl_VIDIOC_ENUM_FMT_fail,
	ioctl_VIDIOC_REQBUFS_fail,
	ioctl_VIDIOC_QUERYBUF_fail,
	memory_map_buffers_fail,
	ioctl_VIDIOC_QBUF_fail, 
	ioctl_VIDIOC_STREAMON_fail,
	ioctl_VIDIOC_STREAMOFF_fail,
	ioctl_VIDIOC_DQBUF_fail,
	ioctl_VIDIOC_S_CTRL_fail,
	v4l2_query_capacity_fail,
	v4l2_enum_input_fail,
	v4l2_set_input_channel_fail,
	v4l2_enum_format_fail,
	v4l2_set_camera_format_fail,
	v4l2_set_isp_format_fail,
	v4l2_set_ctrl_fail,
	v4l2_require_buffers_fail,
	v4l2_query_buffers_fail,
	v4l2_queue_buffer_fail, 
	v4l2_stream_on_fail,
	v4l2_stream_off_fail,
	clear_v4l2_mmap_fail,
	clear_v4l2_stream_mmap_fail,
	h264_encode_close_fail,
	open_h264_encode_fail,
	init_h264_encode_fail, 
	h264_encode_get_in_buffer_fail,
	h264_encode_get_out_buffer_fail,
	scan_frame_start_code_fail,
	get_sps_and_pps_from_header_fail,
	video_sem_post_fail,
	sem_wait_video_pthread_fail,
	recv_rtsp_command_fail,
	create_rtp_thread_fail,
	pthread_mutex_lock_fail,
	pthread_cond_wait_fail,
	update_globe_frame_info_fail,
	poll_yuv_buffer_fail,
	v4l2_dequeue_buffer_fail,
	h264_encode_set_in_buffer_fail,
	h264_encode_encode_buffer_fail,
	reset_v4l2_stream_fail,
	calloc_memory_fail,
	convert_NV12_to_IYUV_fail,
	mkdir_fail,
	open_picture_file_fail,
	fwrite_fail,
	lstat_photo_file_fail,
	take_pohto_fail,
	return_trans_result_to_message_queue_fail,
	dispose_send_file_fail, 
	clear_video_pthread_mem_fail,
	
	can_not_find_image_file,
	send_photo_data_fail,
	read_photo_file_fail,
	
	ENUM_ERROR_CODE_LIST_END
};

void echo_error_prompt(unsigned int i_error_code, const char *errro_in_file, const char *error_in_function, int error_in_line);

void write_log(unsigned int i_error_code, const char *errro_in_file, const char *error_in_function, int error_in_line);

int get_ipc_key(key_t *key, int seed);

#endif