#include "public.h"
#include "cmd_server.h"
#include "video_server.h"
#include "photo_send.h"

sem_t *sem = NULL;

int main(int argc, char **argv)
{
	int ret 						= 0;
	pid_t temp_pid 					= 0;
	pid_t cmd_server_pid 			= 0;
	pid_t video_server_pid 			= 0;
	pid_t photo_trans_pid 			= 0;
	int cmd_server_exit_status 		= 0;
	int video_server_exit_status 	= 0;
	int photo_trans_exit_status 	= 0;
	
	sem = sem_open(SYN_SEM_NAME_FOR_CMD_SERVER_AND_PHOTO_CLIENT, O_CREAT, 0644, 0);
	if(sem == SEM_FAILED)
    {
#ifdef DEBUG
        echo_error_prompt(sem_open_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(sem_open_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return sem_open_fail;
	}
	
	/*************************启动命令服务器进程*****************************/
#ifdef DEBUG
	fprintf(stdout, "start cmd server...\n");
#endif

	temp_pid = fork();
	if(UNLIKELY(temp_pid < 0))
	{
#ifdef DEBUG          
        echo_error_prompt( fork_cmd_server_fail, __FILE__, __FUNCTION__, __LINE__ );
#else
        write_log( fork_cmd_server_fail, __FILE__, __FUNCTION__, __LINE__ );
#endif   
        return fork_cmd_server_fail; 		
	} else if(0 == temp_pid) {
		ret = cmd_server();
		if(OK != ret)
		{
#ifdef DEBUG          
            echo_error_prompt( cmd_server_running_fail, __FILE__, __FUNCTION__, __LINE__ ) ;
#else
            write_log( cmd_server_running_fail, __FILE__, __FUNCTION__, __LINE__ );
#endif   
            // TODO: 向父进程发送自己挂掉信号 
            fprintf(stdout, "cmd server fail");
		    exit(cmd_server_running_fail);			
		}
		
		exit(OK);
	} else {
		cmd_server_pid = temp_pid;
	}
	
	ret = sem_wait(sem);		//等待命令服务器启动 
	if(OK != ret)
	{
#ifdef DEBUG          
        echo_error_prompt(sem_wait_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(sem_wait_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return sem_wait_fail; 
	}
	
	ret = sem_close(sem);
	if(OK != ret)
	{
#ifdef DEBUG          
        echo_error_prompt(sem_close_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(sem_close_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return sem_close_fail; 
	}
	
	ret = sem_unlink(SYN_SEM_NAME_FOR_CMD_SERVER_AND_PHOTO_CLIENT);
	if(OK != ret)
	{
#ifdef DEBUG          
        echo_error_prompt(sem_unlink_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(sem_unlink_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return sem_unlink_fail; 
	}
	
	/***********************创建视频预览/拍照服务器进程**************************/
#ifdef DEBUG
	fprintf(stdout, "start video_server...\n");
#endif
	temp_pid = fork();
	if(UNLIKELY(temp_pid < 0))
	{
#ifdef DEBUG          
        echo_error_prompt(fork_video_server_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(fork_video_server_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return fork_video_server_fail;		
	} else if(LIKELY(0 == temp_pid)) {
		ret = video_server();
		if(OK != ret)
		{
#ifdef DEBUG          
            echo_error_prompt(video_server_running_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(video_server_running_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            // TODO: 向父进程发送自己挂掉信号 
		    exit(video_server_running_fail);			
		}
		
		exit(OK);
	} else {
		video_server_pid = temp_pid;
	}
	
	/*************************启动照片发送客户端进程*****************************/
#ifdef DEBUG
	fprintf(stdout, "start photo_send...\n");
#endif
	temp_pid = fork();
	if(UNLIKELY(temp_pid < 0))
	{
#ifdef DEBUG          
        echo_error_prompt(fork_photo_trans_client_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(fork_photo_trans_client_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return fork_photo_trans_client_fail;		
	} else if(LIKELY(0 == temp_pid)) {
		ret = photo_trans();
		if(ret != OK)
		{
#ifdef DEBUG          
            echo_error_prompt(photo_trans_client_running_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(photo_trans_client_running_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            // TODO: 向父进程发送自己挂掉信号 
		    exit(photo_trans_client_running_fail);
		}
		exit(OK);
	} else {
		photo_trans_pid = temp_pid;
	}
	
	waitpid(cmd_server_pid, cmd_server_exit_status, 0);
	waitpid(video_server_pid, video_server_exit_status, 0);
	waitpid(photo_trans_pid, photo_trans_exit_status, 0);
	
	return OK;	
}