#include "SsbSipMfcApi.h"
#include "videodev2.h"
#include "videodev2_samsung.h"
#include "mfc_interface.h"
#include "jpeglib.h"
#include "jerror.h"
#include "video.h"
#include "message_queue.h"

int fimc_poll(struct pollfd *events)
{
    int ret;

    ret = poll(events, 1, 5000);
    if (ret < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(poll_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(poll_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return poll_fail;
    }

    if (ret == 0)
	{
#ifdef DEBUG          
        echo_error_prompt(poll_time_out, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(poll_time_out, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return poll_time_out;
    }

    return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_querycap
* ���������� ��ѯ�豸���� 
* �� ����    fd             �򿪵������� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_QUERYCAP_fail
                  no_capture_devices
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_querycap(fd)
{
	struct v4l2_capability cap;
	
	if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_QUERYCAP_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_QUERYCAP_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_QUERYCAP_fail;		
	} 
	
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
#ifdef DEBUG          
        echo_error_prompt(no_capture_devices, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(no_capture_devices, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return no_capture_devices;			
	}
	
	return OK; 
}

/***************************************************************************
* �������ƣ� fimc_v4l2_enuminput
* ���������� ��ѯinput��ƵԴ���� 
* �� ����    fd             �򿪵�������
* �� ����    index          ������ƵԴͨ�� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_ENUMINPUT_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_enuminput(int fd, int index)
{
	struct v4l2_input input;
	
	input.index = index;
	if(ioctl(fd, VIDIOC_ENUMINPUT, &input) != 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_ENUMINPUT_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_ENUMINPUT_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_ENUMINPUT_fail;			
	}
#ifdef DEBUG
	fprintf(stdout, "Name of input channel[%d] is %s\n", input.index, input.name);	
#endif
 	
 	return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_s_input
* ���������� ѡ��input��ƵԴ 
* �� ����    fd             �򿪵�������
* �� ����    index          ������ƵԴͨ�� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_S_INPUT_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_s_input(int fd, int index)
{
    struct v4l2_input input;

    input.index = index;

    if (ioctl(fd, VIDIOC_S_INPUT, &input) < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_S_INPUT_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_S_INPUT_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_S_INPUT_fail;	
    }

    return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_s_fmt
* ���������� ���õ�ǰ��Ƶ֡��ʽ 
* �� ����    fd             �򿪵�������
* �� ����    width          ��Ƶ֡width 
* �� ����    height         ��Ƶ֡height
* �� ����    fmt          	v4l2_buf_type
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_S_FMT_format_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_s_fmt(int fd, int width, int height, unsigned int fmt)
{
    struct v4l2_format v4l2_fmt;
    struct v4l2_pix_format pixfmt;

    memset(&v4l2_fmt, 0, sizeof(struct v4l2_format));
    v4l2_fmt.type = V4L2_BUF_TYPE;

    memset(&pixfmt, 0, sizeof(pixfmt));

    pixfmt.width = width;
    pixfmt.height = height;
    pixfmt.pixelformat = fmt;
    pixfmt.field = V4L2_FIELD_NONE;

    v4l2_fmt.fmt.pix = pixfmt;
    //printf("fimc_v4l2_s_fmt : width(%d) height(%d)\n", width, height);

    if (ioctl(fd, VIDIOC_S_FMT, &v4l2_fmt) < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_S_FMT_format_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_S_FMT_format_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_S_FMT_format_fail;	
    }

    return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_s_fmt_is
* ���������� ���õ�ǰ��Ƶ֡��ʽ-�������� 
* �� ����    fd             �򿪵�������
* �� ����    width          ��Ƶ֡width 
* �� ����    height         ��Ƶ֡height
* �� ����    fmt          	v4l2_buf_type
* �� ����    field          v4l2_field
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_ISP_VIDIOC_S_FMT_format_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_s_fmt_is(int fd, int width, int height, unsigned int fmt, enum v4l2_field field)
{
    struct v4l2_format v4l2_fmt;
    struct v4l2_pix_format pixfmt;

    memset(&pixfmt, 0, sizeof(pixfmt));

    v4l2_fmt.type = V4L2_BUF_TYPE_PRIVATE;

    pixfmt.width = width;
    pixfmt.height = height;
    pixfmt.pixelformat = fmt;
    pixfmt.field = field;

    v4l2_fmt.fmt.pix = pixfmt;

	/* Set up for capture */
    if (ioctl(fd, VIDIOC_S_FMT, &v4l2_fmt) < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_ISP_VIDIOC_S_FMT_format_fail, __FILE__, __FUNCTION__, __LINE__);
        fprintf(stderr, "error in fimc_v4l2_s_fmt_is: %s\n", strerror(errno));
#else
        write_log(ioctl_ISP_VIDIOC_S_FMT_format_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_ISP_VIDIOC_S_FMT_format_fail;	
    }

    return OK;
}


/***************************************************************************
* �������ƣ� fimc_v4l2_enum_fmt
* ���������� ��ѯ��ö��֧�ֵĸ�ʽ 
* �� ����    fd             �򿪵�������
* �� ����    fmt          	v4l2_buf_type
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_ENUM_FMT_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_enum_fmt(int fd, unsigned int fmt)
{
    struct v4l2_fmtdesc fmtdesc;
    int found = 0;

    fmtdesc.type = V4L2_BUF_TYPE;
    fmtdesc.index = 0;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        if (fmtdesc.pixelformat == fmt) {
            fprintf(stdout, "passed fmt = %#x found pixel format[%d]: %s\n", fmt, fmtdesc.index, fmtdesc.description);
            found = 1;
            break;
        }

        fmtdesc.index++;
    }

    if (!found) {
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_ENUM_FMT_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_ENUM_FMT_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_ENUM_FMT_fail;
    }

    return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_reqbufs
* ���������� ���뻺���� 
* �� ����    fd             �򿪵�������
* �� ����    type          	v4l2_buf_type
* �� ����    nr_bufs        ����֡���� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_REQBUFS_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_reqbufs(int fd, enum v4l2_buf_type type, int nr_bufs)
{
    struct v4l2_requestbuffers req;

    req.count = nr_bufs;
    req.type = type;
    req.memory = V4L2_MEMORY_TYPE;		//�ڴ�ӳ�� 

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_REQBUFS_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_REQBUFS_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_REQBUFS_fail;
    }

    return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_querybuf
* ���������� �������� 
* �� ����    fd             �򿪵�������
* �� ����    framebuf       VideoBuffer
* �� ����    type        	v4l2_buf_type 
* �� ����    nr_frames      ����֡���� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_QUERYBUF_fail
                  memory_map_buffers_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_querybuf(int fd, enum v4l2_buf_type type, int nr_frames, VideoBuffer *framebuf)
{
    struct v4l2_buffer v4l2_buf;
    int i, ret;

    for (i = 0; i < nr_frames; i++)
	{
        v4l2_buf.type = type;
        v4l2_buf.memory = V4L2_MEMORY_TYPE;
        v4l2_buf.index = i;

        ret = ioctl(fd, VIDIOC_QUERYBUF, &v4l2_buf);
        if (ret < 0) 
		{
#ifdef DEBUG          
            echo_error_prompt(ioctl_VIDIOC_QUERYBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(ioctl_VIDIOC_QUERYBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return ioctl_VIDIOC_QUERYBUF_fail;
        }

        framebuf[i].length = v4l2_buf.length;
        if ((framebuf[i].start = (char *) mmap(0, v4l2_buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, v4l2_buf.m.offset)) < 0)
		{
#ifdef DEBUG          
            echo_error_prompt(memory_map_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(memory_map_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return memory_map_buffers_fail;
        }

#ifdef DEBUG 
        printf("framebuf[%d].start = %p v4l2_buf.length = %d\n", i, framebuf[i].start, v4l2_buf.length);
#endif

    }

	return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_qbuf
* ���������� ��֡���뻺���� 
* �� ����    fd             �򿪵�������
* �� ����    fd             ����֡id 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_QBUF_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_qbuf(int fd, int index)
{
    struct v4l2_buffer v4l2_buf;
    int ret;

    v4l2_buf.type = V4L2_BUF_TYPE;
    v4l2_buf.memory = V4L2_MEMORY_TYPE;
    v4l2_buf.index = index;

    ret = ioctl(fd, VIDIOC_QBUF, &v4l2_buf);
    if (ret < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_QBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_QBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_QBUF_fail;
    }

    return OK;
}


/***************************************************************************
* �������ƣ� fimc_v4l2_streamon
* ���������� ���� 
* �� ����    fd             �򿪵�������
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_STREAMON_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_streamon(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE;
    int ret;

    ret = ioctl(fd, VIDIOC_STREAMON, &type);
    if (ret < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_STREAMON_fail, __FILE__, __FUNCTION__, __LINE__);
        fprintf(stderr, "error in [video] ioctl VIDIOC_STREAMON = %s\n", strerror(errno));
#else
        write_log(ioctl_VIDIOC_STREAMON_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_STREAMON_fail;
    }

    return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_streamoff
* ���������� �ر��� 
* �� ����    fd             �򿪵�������
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_STREAMON_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_streamoff(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE;
    int ret;

    ret = ioctl(fd, VIDIOC_STREAMOFF, &type);
    if (ret < 0) 
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_STREAMOFF_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_STREAMOFF_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_STREAMOFF_fail;
    }

    return OK;
}

/***************************************************************************
* �������ƣ� fimc_v4l2_dqbuf
* ���������� �ӻ�����ȡ�� 
* �� ����    fd             �򿪵�������
* �� ����    v4l2_buf       v4l2_buffer
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:                 
                  ioctl_VIDIOC_DQBUF_fail
* ����˵����		����AndoridԴ��������v4l2��kernel driver���� 		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int fimc_v4l2_dqbuf(int fd, struct v4l2_buffer *v4l2_buf)
{
    int ret;

    v4l2_buf->type = V4L2_BUF_TYPE;
    v4l2_buf->memory = V4L2_MEMORY_TYPE;

    ret = ioctl(fd, VIDIOC_DQBUF, v4l2_buf);
    if (ret < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_DQBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_DQBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_DQBUF_fail;
    }

    return v4l2_buf->index;
}

int fimc_v4l2_s_ctrl(int fp, unsigned int id, unsigned int value)
{
    struct v4l2_control ctrl;
    int ret;

    ctrl.id = id;
    ctrl.value = value;

    ret = ioctl(fp, VIDIOC_S_CTRL, &ctrl);
    if (ret < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_S_CTRL_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_S_CTRL_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_S_CTRL_fail;
    }

    return OK;
}

/***************************************************************************
* �������ƣ� clear_v4l2_mmap
* ���������� �����ں���Ƶ������û�����֮����ڴ�ӳ��
* �� ����    fd           ������豸���
* �� ����    framebuf     �ں���Ƶ������Ӧ�ó����ڵ�ӳ���ַ����С

* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:               
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int clear_v4l2_mmap(int fd, VideoBuffer *framebuf)
{
	int i = 0;
	
	for(i=0; i<MAX_NR_BUFFERS; i++)
	{
		munmap(framebuf[i].start, framebuf[i].length);
	}
	
	/* 
	ret = fimc_v4l2_reqbufs(fd, V4L2_BUF_TYPE, 0);     //�ͷ��ں��еĻ���
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_require_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_require_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_require_buffers_fail;
    } 
*/
	return OK;
}

/***************************************************************************
* �������ƣ� clear_v4l2_mmap_camfd
* ���������� �����ں���Ƶ������û�����֮����ڴ�ӳ�䡢����ͷ�豸���
* �� ����    fd           ������豸���
* �� ����    framebuf     �ں���Ƶ������Ӧ�ó����ڵ�ӳ���ַ����С

* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:               
                  clear_mmap_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int clear_v4l2_mmap_camfd(int fd, VideoBuffer *framebuf)
{
    int ret = 0; 
	
    ret = clear_v4l2_mmap(fd, framebuf);
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(clear_v4l2_mmap_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(clear_v4l2_mmap_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return clear_v4l2_mmap_fail;
    } 
    close(fd);

    return OK;
}

/***************************************************************************
* �������ƣ� clear_v4l2_stream_mmap
* ���������� �ر�v4l2���������ں���Ƶ������û�����֮����ڴ�ӳ��
* �� ����    fd           ������豸���
* �� ����    framebuf     �ں���Ƶ������Ӧ�ó����ڵ�ӳ���ַ����С

* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:               
                  v4l2_stream_off_fail
                  clear_mmap_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int clear_v4l2_stream_mmap(int fd, VideoBuffer *framebuf)
{
	int ret = 0;
	
	ret = fimc_v4l2_streamoff(fd);
	if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_stream_off_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_stream_off_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_stream_off_fail;
    }
	
	ret = clear_v4l2_mmap(fd, framebuf);
	if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(clear_v4l2_mmap_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(clear_v4l2_mmap_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return clear_v4l2_mmap_fail;
    }
	
	return OK;  
}

/***************************************************************************
* �������ƣ� clear_video_pthread_mem
* ���������� ����v4l2�����ں���Ƶ������û�����֮����ڴ�ӳ�䡢����ͷ�豸�����h264������ָ��
* �� ����    fd           ������豸���
* �� ����    hOpen        h264������ָ��
* �� ����    framebuf     �ں���Ƶ������Ӧ�ó����ڵ�ӳ���ַ����С

* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:               
                  clear_v4l2_stream_mmap_fail
                  h264_encode_close_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
int clear_video_pthread_mem(int fd, void *hOpen, VideoBuffer *framebuf)
{
	int ret = 0;
	
	ret = clear_v4l2_stream_mmap(fd, framebuf);
	if(ret != OK) 
	{
#ifdef DEBUG          
            echo_error_prompt(clear_v4l2_stream_mmap_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(clear_v4l2_stream_mmap_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return clear_v4l2_stream_mmap_fail;
    }
    
    close(fd);
    
    ret = SsbSipMfcEncClose(hOpen);
    if(ret != MFC_RET_OK) 
	{
#ifdef DEBUG          
            echo_error_prompt(h264_encode_close_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(h264_encode_close_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return h264_encode_close_fail;
    }
    
    return OK;
}

void cleanup_video_pthread(void *param)
{
	clear_video_pthread_mem(((struct_video_pthread_param *)param)->cam_fd,
							((struct_video_pthread_param *)param)->encode_fp,
							((struct_video_pthread_param *)param)->framebuf);
}

/***************************************************************************
* �������ƣ� init_h264_encoder_param
* ���������� ��ʼ��H264���������� 
* �� ����    pH264Arg             SSBSIP_MFC_ENC_H264_PARAM
* ����ֵ��   �ɹ�:               
             ʧ��:                 
* ����˵����		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-09      �ų�          ����
****************************************************************************/
void init_h264_encoder_param(SSBSIP_MFC_ENC_H264_PARAM *pH264Arg)
{
    pH264Arg->codecType 				= H264_ENC;
    pH264Arg->SourceWidth 				= CAMERA_PREVIEW_WIDTH;
    pH264Arg->SourceHeight 				= CAMERA_PREVIEW_HEIGHT;
    pH264Arg->IDRPeriod 				= 20;
    pH264Arg->SliceMode					= 0;
    pH264Arg->RandomIntraMBRefresh 		= 0;
    pH264Arg->EnableFRMRateControl		= 0;
    pH264Arg->Bitrate					= 1000;
    pH264Arg->FrameQp					= 20;
    pH264Arg->FrameQp_P					= pH264Arg->FrameQp+1;
    pH264Arg->QSCodeMax					= 51;
    pH264Arg->QSCodeMin					= 10;
    pH264Arg->CBRPeriodRf				= 100;
    pH264Arg->PadControlOn				= 0;
    pH264Arg->LumaPadVal				= 0;
    pH264Arg->CbPadVal					= 0;
    pH264Arg->CrPadVal					= 0;
    pH264Arg->FrameMap 					= NV12_LINEAR;
    //pH264Arg->OutputMode				= 0;					/* [IN] Output mode: Frame/Slice */
    
    /* H.264 specific parameters */
    pH264Arg->ProfileIDC				= 1;
    pH264Arg->LevelIDC					= 40;
    pH264Arg->FrameQp_B					= pH264Arg->FrameQp+3;
    pH264Arg->FrameRate					= 35;
    pH264Arg->SliceArgument				= 0;
    pH264Arg->NumberBFrames				= 0;
    pH264Arg->NumberReferenceFrames		= 1;
    pH264Arg->NumberRefForPframes		= 1;
    pH264Arg->LoopFilterDisable			= 1;
    pH264Arg->LoopFilterAlphaC0Offset	= 0;
    pH264Arg->LoopFilterBetaOffset		= 0;	
    pH264Arg->SymbolMode				= 0;
    pH264Arg->PictureInterlace			= 0;
    pH264Arg->Transform8x8Mode			= 0;
    pH264Arg->EnableMBRateControl		= 0;
    pH264Arg->DarkDisable				= 1;
    pH264Arg->SmoothDisable				= 1;
    pH264Arg->StaticDisable				= 1;
    pH264Arg->ActivityDisable			= 1;
}

/***************************************************************************
* �������ƣ� get_sps_and_pps_from_header
* ���������� ��ȡSPS��PPS��Ϣ 
* �� ����    encode_header             ����ͷ 
* �� ����    encode_header_length      ����ͷ����       SSBSIP_MFC_ENC_H264_PARAM
* �� ����    SPS             			SPS 
* �� ����    length_of_SPS             SPSͷ���� 
* �� ����    PPS             			PPS
* �� ����    length_of_PPS             PPSͷ���� 
* ����ֵ��   �ɹ�: 
					OK              
             ʧ��:  
			 		scan_frame_start_code_fail               
* ����˵����		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-10      �ų�          ����
****************************************************************************/
int get_sps_and_pps_from_header(char *encode_header, int encode_header_length, char *SPS, int *length_of_SPS, char *PPS, int *length_of_PPS)
{
	char start_code[4]                = {0x00,0x00,0x00,0x01};
	char start_code_and_pps[5]        = {0x00,0x00,0x00,0x01,0x68};
	int start_code_and_pps_position   = 0;
	
	//�ɹ������Ӵ�ָ�룬ʧ�ܷ��ؿ�ָ�� 
    start_code_and_pps_position = memmem(encode_header + sizeof(start_code), encode_header_length, start_code, sizeof(start_code));
    if(NULL == start_code_and_pps_position)
    {
#ifdef DEBUG
        echo_error_prompt(scan_frame_start_code_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(scan_frame_start_code_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return scan_frame_start_code_fail;
    }

	*length_of_SPS = (char*)start_code_and_pps_position - encode_header;
	memcpy(SPS, encode_header, *length_of_SPS);

	*length_of_PPS = encode_header + encode_header_length - start_code_and_pps_position;
	memcpy(PPS, (char*)start_code_and_pps_position, *length_of_PPS);
	
	return OK;
}

/***************************************************************************
* �������ƣ� update_globe_frame_info
* ���������� ����֡��Ϣ��rtp�߳�ͨ�� 
* �� ����    frame_info             struct_frame_buffer 
* �� ����    frame_buffer      		
* �� ����    frame_length            
* �� ����    time_stamp             ʱ��� 
* ����ֵ��   �ɹ�: 
					OK              
             ʧ��:  
			 		pthread_mutex_lock_fail
					pthread_cond_wait_fail                
* ����˵����		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-10      �ų�          ����
****************************************************************************/
int update_globe_frame_info(struct_frame_buffer *frame_info, char *frame_buffer, unsigned int frame_length, unsigned int time_stamp)
{
    int ret = 0;
    ret = pthread_mutex_lock( &(frame_info->mtx) );
    if(0 != ret)
    {
#ifdef DEBUG          
        echo_error_prompt(pthread_mutex_lock_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(pthread_mutex_lock_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return pthread_mutex_lock_fail;
    }

	pthread_cleanup_push(pthread_mutex_unlock, &(frame_info->mtx));
	
    while(frame_info->is_full == true) //���֡��δ��ȡ�� 
    {
        ret = pthread_cond_wait( &(frame_info->cond), &(frame_info->mtx) );
        if(0 != ret)
        {
#ifdef DEBUG          
        echo_error_prompt(pthread_cond_wait_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(pthread_cond_wait_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return pthread_cond_wait_fail;
        } 
    }
    memcpy(frame_info->frame_buffer, frame_buffer, frame_length);
    frame_info->time_stamp = time_stamp;
 
    frame_info->frame_length = frame_length;
    frame_info->is_full = true;
        
    pthread_cond_signal( &(frame_info->cond) );

	pthread_mutex_unlock( &(frame_info->mtx) );
    pthread_cleanup_pop(0); 
	
    return OK;
}

/***************************************************************************
* �������ƣ� reset_v4l2_stream
* ���������� ����v4l2�� 
* �� ����    fd             		��� 
* �� ����    framebuf      			VideoBuffer
* �� ����    width            		��Ƶ���ߴ�
* �� ����    height             	��Ƶ���ߴ� 
* �� ����    scenario             	v4l2_field  
* ����ֵ��   �ɹ�: 
					OK              
             ʧ��:  
			 		fail                
* ����˵����		
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-10      �ų�          ����
****************************************************************************/
int reset_v4l2_stream(int fd, VideoBuffer *framebuf, int width, int height, enum v4l2_field scenario)
{
    int ret = 0;
    int i   = 0;

    ret = clear_v4l2_stream_mmap(fd, framebuf);
    if(ret != OK) 
	{
#ifdef DEBUG          
            echo_error_prompt(clear_v4l2_stream_mmap_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(clear_v4l2_stream_mmap_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return clear_v4l2_stream_mmap_fail;
    }
   	 
    ret = fimc_v4l2_s_fmt(fd, width, height, VIDEO_PIX_FORMAT);
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_camera_format_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_camera_format_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_set_camera_format_fail;
    } 
    
    ret = fimc_v4l2_s_fmt_is(fd, width, height, VIDEO_PIX_FORMAT, scenario);     //IS_MODE_PREVIEW_VIDEO
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_isp_format_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_isp_format_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_set_isp_format_fail;
    }
    
    ret = fimc_v4l2_s_ctrl(fd, V4L2_CID_IS_S_SCENARIO_MODE, scenario);		//����extnos4����
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_ctrl_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_ctrl_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_set_ctrl_fail;
    }    

    ret = fimc_v4l2_s_ctrl(fd, V4L2_CID_CACHEABLE, 1);		//����extnos4����
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_ctrl_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_ctrl_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_set_ctrl_fail;
    } 

    ret =  fimc_v4l2_reqbufs(fd, V4L2_BUF_TYPE, MAX_NR_BUFFERS);
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_require_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_require_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_require_buffers_fail;
    }      
    
     ret = fimc_v4l2_querybuf(fd, V4L2_BUF_TYPE, MAX_NR_BUFFERS, framebuf);     //������forѭ������ 
     if (ret != OK)
	 {
#ifdef DEBUG          
        echo_error_prompt(v4l2_query_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_query_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_query_buffers_fail;
     }      
    
    for ( i = 0; i < MAX_NR_BUFFERS; i++)
	{
        ret = fimc_v4l2_qbuf(fd, i);
        if (ret != OK) 
		{
#ifdef DEBUG          
            echo_error_prompt(v4l2_queue_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(v4l2_queue_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return v4l2_queue_buffer_fail;
        }    
    }    
    
    ret = fimc_v4l2_streamon(fd);
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_stream_on_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_stream_on_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return v4l2_stream_on_fail;
    }      

    return OK;
}

/***************************************************************************
* �������ƣ� ConvertNV12toIYUV
* ���������� ��nv12����ת��Ϊiyuv 
* �� ����    pbuf     	���� 
* �� ����    imgw      ͼ���� 
* �� ����    imgh      ͼ��߶� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:               
                  calloc_memory_fail
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-10      �ų�          ����
****************************************************************************/
int ConvertNV12toIYUV(char* pbuf, int imgw, int imgh)
{
    char* pPU 		= NULL;
    char* pPV 		= NULL;
    char* pPUV 		= NULL;
    char* pcache 	= NULL;
    int  i      	= 0;

    if (pbuf == NULL)
       return -1;
    pPUV = pbuf + imgw*imgh;
    pcache = (char*)malloc((imgw*imgh)>>1);
    if (pcache == NULL)
    {
#ifdef DEBUG
        echo_error_prompt(calloc_memory_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(calloc_memory_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return calloc_memory_fail;    
    }
    pPV = pcache;
    pPU = pcache + ((imgw*imgh)>>2);
    for (i=0;i<(imgw*imgh)>>1;i++)
    {
        if ((i % 2) == 0)
            *pPV++ = *(pPUV+i);
        else
            *pPU++ = *(pPUV+i);
    }
    memcpy(pPUV,pcache,(imgw*imgh)>>1);
    if (pcache)
        free(pcache);
        
    return OK;
}

/***************************************************************************
* �������ƣ� I420_data_to_JPEG_data
* ���������� ��yuv420����ת��Ϊjpeg��ʽ���洢��ָ���ڴ��ַ�ռ���
* �� ����    I420_Data      jpg�ļ�����
* �� ����    image_width    ͼ����
* �� ����    image_height   ͼ��߶�
* �� ����    quality        ѹ������(1-100)
* �� ����    jpg_data       jpg��ʽ��ͼ�����ݵ�ַ
* �� ����    jpg_data_size  jpg��ʽ��ͼ�����ݴ�С
* ����ֵ��   �ɹ�:               
                  
             ʧ��:               
                  
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2014-12-18      ��ǿ          ����
****************************************************************************/
void I420_data_to_JPEG_data(unsigned char *I420_Data, int image_width, int image_height, int quality, unsigned char *jpg_data, unsigned long *jpg_data_size)
{
    int i, j;  
    
    JSAMPROW y[16],cb[16],cr[16]; // y[2][5] = color sample of row 2 and pixel column 5; (one plane)  
    JSAMPARRAY planes[3]; // t[0][2][5] = color sample 0 of row 2 and column 5  
    
    struct jpeg_compress_struct cinfo;  
    struct jpeg_error_mgr jerr;  
    
    planes[0] = y;  
    planes[1] = cb;  
    planes[2] = cr;  
    
    cinfo.err = jpeg_std_error(&jerr);  // errors get written to stderr   
        
    jpeg_create_compress(&cinfo);  
    cinfo.image_width = image_width;  
    cinfo.image_height = image_height;  
    cinfo.input_components = 3;  
    jpeg_set_defaults (&cinfo);  
    
    jpeg_set_colorspace(&cinfo, JCS_YCbCr);  
    
    cinfo.raw_data_in = TRUE;                  // supply downsampled data  
    cinfo.do_fancy_downsampling = FALSE;       // fix segfaulst with v7  
    cinfo.comp_info[0].h_samp_factor = 2;  
    cinfo.comp_info[0].v_samp_factor = 2;  
    cinfo.comp_info[1].h_samp_factor = 1;  
    cinfo.comp_info[1].v_samp_factor = 1;  
    cinfo.comp_info[2].h_samp_factor = 1;  
    cinfo.comp_info[2].v_samp_factor = 1;  
    
    jpeg_set_quality(&cinfo, quality, TRUE);  
    cinfo.dct_method = JDCT_FASTEST;  
    //configSamplingFactors(&cinfo);   //Ҫ����?
    
    jpeg_mem_dest(&cinfo, &jpg_data, jpg_data_size);    //���иú�����ʵ�ֻ���Ϊ���jpg_data��jpg_data_size
                                                        //���jpg_data��jpg_data_size����ΪNULL��ʹ�ô���������ڴ�ʹ�С
														//��һ��ΪNULL�����Զ�Ϊ��malloc�����ڴ�,�������ڴ��ַ��jpg_data
                                                        //�ʣ����Ѿ������jpg_data�������&jpg_data��д����ȷ�����δ�������д���޷�����
    jpeg_start_compress (&cinfo, TRUE);                 //jpeg_mem_dest�Զ�Ϊjpg_data������ڴ��ַ
    
    for (j = 0; j < image_height; j += 16) {  
        for (i = 0; i < 16; i++) {  
            y[i] = I420_Data + image_width * (i + j);  
            if (i%2 == 0) {  
                cb[i/2] = I420_Data + image_width * image_height + image_width / 2 * ((i + j) / 2);  
                cr[i/2] = I420_Data + image_width * image_height + image_width * image_height / 4 + image_width / 2 * ((i + j) / 2);  
            }  
        }  
        
        jpeg_write_raw_data(&cinfo, planes, 16);  
    }  
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);       
}

int take_pohto(int fd, VideoBuffer *framebuf, char *photo_name, char *photo_path_name, int *photo_size)
{
    int                ret                                  = 0;
    int                y_size                               = 0;
    int                c_size                               = 0;
    int                aligned_y_size                       = 0;
    int                aligned_c_size                       = 0;
	char               *NV12_image                          = NULL;
	time_t             now_time                             = 0;
	struct v4l2_buffer v4l2_buf                             = {0};
	int                index                                = 0; 
	int                jpg_data_size                        = 0;
	char               *jpg_data_buffer                     = NULL; 
	FILE               *outfile                             = NULL ;
	struct stat        stat_info							= {0};
    struct pollfd      m_events_c 							= {0};
    char			   dir_name[MAX_PHOTO_DIR_NAME_LENGTH]	= {0};

    y_size = CAMERA_SNAPSHOT_WIDTH*CAMERA_SNAPSHOT_HEIGHT;
    c_size = CAMERA_SNAPSHOT_WIDTH*CAMERA_SNAPSHOT_HEIGHT/2;
    aligned_y_size = Align(y_size, 64*BUF_L_UNIT);       
    aligned_c_size = Align(c_size, 64*BUF_L_UNIT);

/*#ifdef DEBUG
    fprintf(stdout, "aligned_y_size = %d, aligned_c_size = %d\n", aligned_y_size, aligned_c_size);
#endif*/

    ret = reset_v4l2_stream(fd, framebuf, CAMERA_SNAPSHOT_WIDTH, CAMERA_SNAPSHOT_HEIGHT, (enum v4l2_field)IS_MODE_CAPTURE_STILL);  
    if(ret != OK)
    {
#ifdef DEBUG          
        echo_error_prompt(reset_v4l2_stream_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(reset_v4l2_stream_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return reset_v4l2_stream_fail;
    }

    memset(&m_events_c, 0, sizeof(m_events_c));
    m_events_c.fd = fd;
    m_events_c.events = POLLIN | POLLERR;
    
    ret = fimc_poll(&m_events_c);
    if(ret != OK)
    {
#ifdef DEBUG          
        echo_error_prompt(poll_yuv_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(poll_yuv_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return poll_yuv_buffer_fail;
    }
     
    index = fimc_v4l2_dqbuf(fd, &v4l2_buf);
    if (!(0 <= index && index < MAX_NR_BUFFERS)) 
    {
#ifdef DEBUG          
       echo_error_prompt(v4l2_dequeue_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
       write_log(v4l2_dequeue_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
       return v4l2_dequeue_buffer_fail;
    }

	NV12_image = (char *)calloc(y_size+c_size, sizeof(char));	
    memcpy(NV12_image, framebuf[v4l2_buf.index].start, y_size);
    memcpy(NV12_image+y_size, framebuf[v4l2_buf.index].start+aligned_y_size, c_size);
    
    ret = ConvertNV12toIYUV(NV12_image, CAMERA_SNAPSHOT_WIDTH, CAMERA_SNAPSHOT_HEIGHT);
    if (ret != OK) 
    {
       	free(NV12_image);
#ifdef DEBUG          
        echo_error_prompt(convert_NV12_to_IYUV_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(convert_NV12_to_IYUV_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return convert_NV12_to_IYUV_fail;
    }

    time(&now_time);
	sprintf(dir_name,"%s/",SD_CARD_MNT);
	strftime(dir_name+strlen(dir_name), sizeof(dir_name)-strlen(dir_name), "%Y-%m-%d/", localtime(&now_time));
	
    if(access(dir_name, NULL) != 0 )  
    {  
        if(mkdir(dir_name, 0755)==-1)  
        {   
            free(NV12_image);
#ifdef DEBUG          
            echo_error_prompt(mkdir_fail, __FILE__, __FUNCTION__, __LINE__);
            fprintf(stderr, "error in [video]:mkdir %s\n", strerror(errno));
#else
            write_log(mkdir_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            return mkdir_fail;
        }  
    }  
	sprintf(photo_path_name, "%s/%s", dir_name, photo_name);
 
    jpg_data_size = CAMERA_SNAPSHOT_WIDTH*CAMERA_SNAPSHOT_HEIGHT*1.5;
    jpg_data_buffer = (char*)calloc(jpg_data_size, sizeof(char));	

	I420_data_to_JPEG_data(NV12_image, CAMERA_SNAPSHOT_WIDTH, CAMERA_SNAPSHOT_HEIGHT, 100, jpg_data_buffer, &jpg_data_size);

    free(NV12_image);                           //��ȡyuv420������ɺ��ͷ����û��ռ�Ļ�����
    ret = ioctl(fd, VIDIOC_QBUF, &v4l2_buf);    //�黹�ں˿ռ�Ļ�����
    if (ret < 0) 
    {
        free(jpg_data_buffer);
#ifdef DEBUG          
        echo_error_prompt(ioctl_VIDIOC_QBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(ioctl_VIDIOC_QBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        return ioctl_VIDIOC_QBUF_fail;
    }

    // TODO:�����ļ������е����ݵ��ļ�
    outfile = fopen(photo_path_name, "wb");
    if( NULL == outfile )
    {
        free(jpg_data_buffer);
#ifdef DEBUG
        echo_error_prompt(open_picture_file_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(open_picture_file_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return open_picture_file_fail;        
    }
	
 	if(fwrite(jpg_data_buffer, jpg_data_size, 1, outfile) != 1) 
  	{
   		free(jpg_data_buffer);
   		fclose(outfile);
   		remove(photo_path_name);		   
#ifdef DEBUG
		echo_error_prompt(fwrite_fail, __FILE__, __FUNCTION__, __LINE__);
#else
		write_log(fwrite_fail, __FILE__, __FUNCTION__, __LINE__);
#endif

		return fwrite_fail;  		   
	} 
    fclose(outfile);
    free(jpg_data_buffer);          //�����ļ����ͷ��ļ����ڴ��еĻ���
	
    ret = stat(photo_path_name, &stat_info);
    if (ret < 0) 
    {
#ifdef DEBUG
            echo_error_prompt(lstat_photo_file_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(lstat_photo_file_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
         
            return lstat_photo_file_fail;	
    }
	*photo_size = stat_info.st_size;

	return OK;
}

/***************************************************************************
* �������ƣ� return_take_photo_result_to_message_queue
* ���������� �������ս������Ϣ���� 
* �� ����    msg_id           			��Ϣ���б�־ 
* �� ����    photo_name     			ͼƬ�� 
* �� ����    photo_size     			ͼƬ��С 
* �� ����    is_take_photo_succeed     	�����Ƿ�ɹ� 
* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:               
                  ...
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2016-01-08      �ų�          ����
****************************************************************************/
int return_take_photo_result_to_message_queue(int msg_id, char *photo_name, int photo_size, bool is_take_photo_succeed)
{
    int                   ret             = 0;
    struct_take_photo_msg take_photo_msg  = {0};

	take_photo_msg.type = MESSAGE_QUEUE_TAKE_PHOTO_OVER_TYPE;
	take_photo_msg.succeed = is_take_photo_succeed;
	memcpy(take_photo_msg.photo_name, photo_name, MAX_PHOTO_NAME_LENGTH);
	take_photo_msg.photo_size = photo_size;

    ret = send_message_to_queue(msg_id, &take_photo_msg, sizeof(struct_take_photo_msg));   //��Ϣ��Сָ���ǣ�����Ϣtype�����Զ������ݵĴ�С
    if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(send_message_to_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(send_message_to_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return send_message_to_queue_fail;
    }

    return OK;
}

/***************************************************************************
* �������ƣ� dispose_send_file
* ���������� ���պ����ͷ���ͼƬ���������Ͷ��� 
* �� ����    file_name    ͼƬ����(����·��) 
* �� ����    msg_id       ͼƬ���Ͷ���id

* ����ֵ��   �ɹ�:               
                  OK
             ʧ��:               
                  send_message_to_queue_fail    		����ʧ�� 
* ����˵���� 
* �޸�����        �޸���        �޸�����
* --------------------------------------------------------------------------
  2015-12-16      �ų�          ����
  2016-01-08	  �ų� 			���msg_buf_send_photo����ͼƬ����·��д����� 
  2016-01-11	  �ų�			�����β�photo_size��Ҫ���͵�ͼƬ��С 
****************************************************************************/
int dispose_send_file(char *photo_path_name, int msg_id, unsigned int photo_size)
{
	int ret 				   						= 0;
	struct_send_photo_msg send_photo_msg 			= {0};
	
	send_photo_msg.type = MESSAGE_QUEUE_TRANS_PHOTO_TYPE;
	send_photo_msg.succeed = false;
	strcpy(send_photo_msg.photo_path_name, photo_path_name);
	send_photo_msg.photo_size = photo_size;
	
	ret = send_message_to_queue(msg_id, &send_photo_msg, sizeof(struct_send_photo_msg));
	if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(send_message_to_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(send_message_to_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        return send_message_to_queue_fail;
	}
	
	return OK; 
}

void *video_pthread(void *arg)
{
	int ret 											= 0;
	key_t take_photo_key 								= 0;
	key_t photo_send_key 								= 0;
	int msg_id_for_take_photo 							= 0;
	int msg_id_for_send_photo 							= 0;
	int fd 												= 0;
	VideoBuffer framebuf[MAX_NR_BUFFERS] 				= {0};
	struct pollfd  m_events_c;
	SSBSIP_MFC_ENC_H264_PARAM h264_encode_param;
	void *hOpen;
	struct_video_pthread_param video_pthread_param;
	SSBSIP_MFC_ENC_INPUT_INFO mfc_enc_input_info;
	SSBSIP_MFC_ENC_OUTPUT_INFO mfc_enc_output_info;
	struct timeval pre_tv 								= {0};
	struct timeval now_tv 								= {0};
	int y_size 											= 0;
	int c_size 											= 0;
	int aligned_y_size 									= 0;
	int aligned_c_size 									= 0; 
	struct v4l2_buffer v4l2_buf                         = {0};
	int index 											= 0;
	unsigned int time_stamp                          	= 0;
	unsigned int time_stamp_increse 					= 0;
	struct_take_photo_msg take_photo_msg 				= {0};
	int photo_file_size 								= 0;
	char photo_path_name[MAX_PHOTO_PATH_NAME_LENGTH] 	= {0};
	int i 												= 0;
	
	//����������Ϣ����
	ret = get_ipc_key(&take_photo_key, IPC_KEY_SEED_FOR_TAKE_PHOTO_MSG);
	if(OK != ret)
	{
#ifdef DEBUG
        echo_error_prompt(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        pthread_exit(get_ipc_key_fail);		
	}
	ret = create_message_queue(take_photo_key, &msg_id_for_take_photo, false);
	if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        pthread_exit(create_message_queue_fail);
    }
	
	//����������Ƭ��Ϣ����
	ret = get_ipc_key(&photo_send_key, IPC_KEY_SEED_FOR_SEND_PHOTO_MSG);
	if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(get_ipc_key_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        pthread_exit(get_ipc_key_fail);
    }
	ret = create_message_queue(photo_send_key, &msg_id_for_send_photo, false);
	if(OK != ret)
    {
#ifdef DEBUG
        echo_error_prompt(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(create_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
        pthread_exit(create_message_queue_fail);
    }
	
	fd = open(CAMERA_DEVICE, O_RDWR);
	if(fd < 0)
	{
#ifdef DEBUG          
        echo_error_prompt(open_camera_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(open_camera_fail, __FILE__, __FUNCTION__, __LINE__);
#endif  	
		pthread_exit(open_camera_fail);	
	}
	
#ifdef DEBUG
	ret = fimc_v4l2_querycap(fd);
	if(OK != ret)
	{
		echo_error_prompt(v4l2_query_capacity_fail, __FILE__, __FUNCTION__, __LINE__);
		pthread_exit(v4l2_query_capacity_fail);
	}
	
	ret = fimc_v4l2_enuminput(fd, 0);
	if(ret != OK)
	{
 		echo_error_prompt(v4l2_enum_input_fail, __FILE__, __FUNCTION__, __LINE__);
        pthread_exit(v4l2_enum_input_fail);
	}
#endif 

	ret = fimc_v4l2_s_input(fd, 0);
	if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_input_channel_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_input_channel_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(v4l2_set_input_channel_fail);
    }
    
#ifdef DEBUG
	ret = fimc_v4l2_enum_fmt(fd, VIDEO_PIX_FORMAT);
	if (ret != OK)
	{
        echo_error_prompt(v4l2_enum_format_fail, __FILE__, __FUNCTION__, __LINE__);
        pthread_exit(v4l2_enum_format_fail);
    } 
#endif 

	ret = fimc_v4l2_s_fmt(fd, CAMERA_PREVIEW_WIDTH, CAMERA_PREVIEW_HEIGHT, VIDEO_PIX_FORMAT);
	if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_camera_format_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_camera_format_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(v4l2_set_camera_format_fail);
    }
	
	ret = fimc_v4l2_s_fmt_is(fd, CAMERA_PREVIEW_WIDTH, CAMERA_PREVIEW_HEIGHT, VIDEO_PIX_FORMAT, (enum v4l2_field)IS_MODE_PREVIEW_VIDEO);  
	if(ret != OK)
    {
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_isp_format_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_isp_format_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(v4l2_set_isp_format_fail);    	
    }
    
    ret = fimc_v4l2_s_ctrl(fd, V4L2_CID_IS_S_SCENARIO_MODE, IS_MODE_PREVIEW_VIDEO);		//����exynos4���� 
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_ctrl_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_ctrl_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(v4l2_set_ctrl_fail);
	}
	
	ret = fimc_v4l2_s_ctrl(fd, V4L2_CID_CACHEABLE, 1);                                 //����exynos4����
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_set_ctrl_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_set_ctrl_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(v4l2_set_ctrl_fail);
    }
    
    ret =  fimc_v4l2_reqbufs(fd, V4L2_BUF_TYPE, MAX_NR_BUFFERS);
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_require_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_require_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(v4l2_require_buffers_fail);
    } 
    
    ret = fimc_v4l2_querybuf(fd, V4L2_BUF_TYPE, MAX_NR_BUFFERS, framebuf);     //������forѭ������ mmap
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_query_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_query_buffers_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(v4l2_query_buffers_fail);
    }
    
    for ( i = 0; i < MAX_NR_BUFFERS; i++)
	{
        ret = fimc_v4l2_qbuf(fd, i);
        if (ret != OK) 
		{
#ifdef DEBUG          
            echo_error_prompt(v4l2_queue_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(v4l2_queue_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   			
            pthread_exit(v4l2_queue_buffer_fail);
        }    
    }
    
    memset(&m_events_c, 0, sizeof(m_events_c));
    m_events_c.fd = fd;
    m_events_c.events = POLLIN | POLLERR;
    
    ret = fimc_v4l2_streamon(fd);
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(v4l2_stream_on_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(v4l2_stream_on_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
		pthread_exit(v4l2_stream_on_fail);
    }
	
	init_h264_encoder_param(&h264_encode_param);
	
	hOpen = SsbSipMfcEncOpen();
	if(hOpen == NULL)
    {
#ifdef DEBUG          
        echo_error_prompt(open_h264_encode_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(open_h264_encode_fail, __FILE__, __FUNCTION__, __LINE__);
#endif  		
        pthread_exit(open_h264_encode_fail);
    }
	video_pthread_param.cam_fd = fd;
	video_pthread_param.encode_fp = hOpen;
	video_pthread_param.framebuf = framebuf;
	pthread_cleanup_push(cleanup_video_pthread, &video_pthread_param);		//��pthread_cleanup_pop�ɶԳ��֣��м������ֹʱ��ִ��cleanup_video_pthread((void *)&video_pthread_param)
	
	if(SsbSipMfcEncInit(hOpen, &h264_encode_param) != MFC_RET_OK)
	{
#ifdef DEBUG          
        echo_error_prompt(init_h264_encode_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(init_h264_encode_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(init_h264_encode_fail);		
	}
	
	if(SsbSipMfcEncGetInBuf(hOpen, &mfc_enc_input_info) != MFC_RET_OK)
	{
#ifdef DEBUG          
        echo_error_prompt(h264_encode_get_in_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(h264_encode_get_in_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(h264_encode_get_in_buffer_fail);		
	}
	
	gettimeofday(&pre_tv, NULL);		//ʱ���Ϊ������������Ƶ֡��һ���ֽڵ�ʱ��
	if(SsbSipMfcEncGetOutBuf(hOpen, &mfc_enc_output_info) != MFC_RET_OK)
	{
#ifdef DEBUG          
        echo_error_prompt(h264_encode_get_out_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(h264_encode_get_out_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(h264_encode_get_out_buffer_fail);		
	}
#ifdef DEBUG
	fprintf(stdout, "[video_pthread] SsbSipMfcEncGetOutBuf\n");
#endif	
	
	ret = get_sps_and_pps_from_header(mfc_enc_output_info.StrmVirAddr, mfc_enc_output_info.headerSize, ((struct_frame_buffer *)arg)->SPS, &(((struct_frame_buffer *)arg)->length_of_SPS), ((struct_frame_buffer *)arg)->PPS, &(((struct_frame_buffer *)arg)->length_of_PPS));
	if(ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(get_sps_and_pps_from_header_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(get_sps_and_pps_from_header_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(get_sps_and_pps_from_header_fail);
	}
#ifdef DEBUG
	fprintf(stdout, "[video_pthread] sem_post\n");
#endif
	
	ret = sem_post(&(((struct_frame_buffer *)arg)->sem));
	if(OK != ret)
    {
#ifdef DEBUG          
        echo_error_prompt(video_sem_post_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(video_sem_post_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(video_sem_post_fail);
	}
	
	//����SPS��PPS 
	ret = update_globe_frame_info((struct_frame_buffer *)arg, ((struct_frame_buffer *)arg)->SPS, ((struct_frame_buffer *)arg)->length_of_SPS,0);
    if(ret != OK)
    {
#ifdef DEBUG          
        echo_error_prompt(update_globe_frame_info_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(update_globe_frame_info_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(update_globe_frame_info_fail);
    }
    ret = update_globe_frame_info((struct_frame_buffer *)arg, ((struct_frame_buffer *)arg)->PPS, ((struct_frame_buffer *)arg)->length_of_PPS,0);
    if(ret != OK)
    {
#ifdef DEBUG          
        echo_error_prompt(update_globe_frame_info_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(update_globe_frame_info_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(update_globe_frame_info_fail);
    }
	
	y_size = CAMERA_PREVIEW_WIDTH*CAMERA_PREVIEW_HEIGHT;
    c_size = CAMERA_PREVIEW_WIDTH*CAMERA_PREVIEW_HEIGHT/2;
    aligned_y_size = Align(y_size, 64 * BUF_L_UNIT);         //H.264����������Ƶ�ڴ��ʽΪ64k����
    aligned_c_size = Align(c_size, 64 * BUF_L_UNIT);
	
	while(1)
	{
		ret = fimc_poll(&m_events_c);
	 	if(ret != OK)
		{
#ifdef DEBUG          
            echo_error_prompt(poll_yuv_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(poll_yuv_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            pthread_exit(poll_yuv_buffer_fail);
        }
        
        index = fimc_v4l2_dqbuf(fd, &v4l2_buf);
        if (!(0 <= index && index < MAX_NR_BUFFERS)) 
        {
#ifdef DEBUG          
            echo_error_prompt(v4l2_dequeue_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(v4l2_dequeue_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            pthread_exit(v4l2_dequeue_buffer_fail);
        }
        
        memcpy (mfc_enc_input_info.YVirAddr,framebuf[v4l2_buf.index].start, aligned_y_size);
        memcpy (mfc_enc_input_info.CVirAddr,framebuf[v4l2_buf.index].start + aligned_y_size,aligned_c_size);
		
		ret = ioctl(fd, VIDIOC_QBUF, &v4l2_buf);
        if (ret < 0) 
        {
#ifdef DEBUG          
            echo_error_prompt(ioctl_VIDIOC_QBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(ioctl_VIDIOC_QBUF_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            pthread_exit(ioctl_VIDIOC_QBUF_fail);
        }
        
        ret = SsbSipMfcEncSetInBuf(hOpen,&mfc_enc_input_info);
        if(ret != MFC_RET_OK)
		{
#ifdef DEBUG          
            echo_error_prompt(h264_encode_set_in_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(h264_encode_set_in_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            pthread_exit(h264_encode_set_in_buffer_fail);
        }
        
        ret = SsbSipMfcEncExe(hOpen);
        if(ret != MFC_RET_OK)
		{
#ifdef DEBUG          
            echo_error_prompt(h264_encode_encode_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(h264_encode_encode_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            pthread_exit(h264_encode_encode_buffer_fail);
        }
        
        memset(&mfc_enc_output_info,0,sizeof(mfc_enc_output_info));                 //�ڱ�����������Ƶ֡��һ���ֽ�ʱ����ʱ���
        gettimeofday(&now_tv , NULL);
        time_stamp_increse = (unsigned int)(90000.0 / (1000.0 / ((now_tv.tv_sec - pre_tv.tv_sec) * 1000.0 + (now_tv.tv_usec - pre_tv.tv_usec) / 1000.0)));
        time_stamp = time_stamp + time_stamp_increse;
        memcpy(&pre_tv, &now_tv, sizeof(struct timeval));
        
        ret = SsbSipMfcEncGetOutBuf(hOpen,&mfc_enc_output_info);
        if(ret != MFC_RET_OK)
		{
#ifdef DEBUG          
            echo_error_prompt(h264_encode_get_out_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(h264_encode_get_out_buffer_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            pthread_exit(h264_encode_get_out_buffer_fail);
        }
        
        ret = update_globe_frame_info((struct_frame_buffer *)arg, mfc_enc_output_info.StrmVirAddr, mfc_enc_output_info.dataSize, time_stamp);
        if(ret != OK)
        {
#ifdef DEBUG          
            echo_error_prompt(update_globe_frame_info_fail, __FILE__, __FUNCTION__, __LINE__);
#else
            write_log(update_globe_frame_info_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
            pthread_exit(update_globe_frame_info_fail);
        }
        
        ret = recv_message_from_queue(msg_id_for_take_photo, MESSAGE_QUEUE_TAKE_PHOTO_TYPE, &take_photo_msg, sizeof(struct_take_photo_msg), IPC_NOWAIT);
		if(ret != no_message_in_queue)
		{
			if(OK == ret)
			{
				ret = take_pohto(fd, framebuf, take_photo_msg.photo_name, photo_path_name, &photo_file_size);
				if(OK != ret)
				{
#ifdef DEBUG          
                    echo_error_prompt(take_pohto_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(take_pohto_fail, __FILE__, __FUNCTION__, __LINE__);
#endif 				
					ret = return_take_photo_result_to_message_queue(msg_id_for_take_photo, take_photo_msg.photo_name, photo_file_size, false);
					if(OK != ret)
                    {
#ifdef DEBUG
                        echo_error_prompt(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                        write_log(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
                        pthread_exit(return_trans_result_to_message_queue_fail);    //��Ϣ����Ͷ�ͣ�����������ֹ�̣߳����ش���
                    } 		
				}
				else
				{
					ret = return_take_photo_result_to_message_queue(msg_id_for_take_photo, take_photo_msg.photo_name, photo_file_size, true);
					if(OK != ret)
                    {
#ifdef DEBUG
                        echo_error_prompt(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                        write_log(return_trans_result_to_message_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
                        pthread_exit(return_trans_result_to_message_queue_fail);    //��Ϣ����Ͷ�ͣ�����������ֹ�̣߳����ش���
                    }
					
					ret = dispose_send_file(photo_path_name, msg_id_for_send_photo, photo_file_size);
					if(OK != ret)
                    {
#ifdef DEBUG
						echo_error_prompt(dispose_send_file_fail, __FILE__, __FUNCTION__, __LINE__);
#else
						write_log(dispose_send_file_fail, __FILE__, __FUNCTION__, __LINE__);
#endif
						return dispose_send_file_fail;               	
                	} 
				}
				
				ret = reset_v4l2_stream(fd, framebuf, CAMERA_PREVIEW_WIDTH, CAMERA_PREVIEW_HEIGHT, IS_MODE_PREVIEW_VIDEO);
				if(ret != OK)
                {
#ifdef DEBUG          
                    echo_error_prompt(reset_v4l2_stream_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                    write_log(reset_v4l2_stream_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
                    pthread_exit(reset_v4l2_stream_fail);
                }
			}
			else
			{
#ifdef DEBUG
                echo_error_prompt(recv_message_from_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#else
                write_log(recv_message_from_queue_fail, __FILE__, __FUNCTION__, __LINE__);
#endif 
                pthread_exit(recv_message_from_queue_fail);				
			}
		}
	} 
	
	pthread_cleanup_pop(0);
	ret = clear_video_pthread_mem(fd, hOpen, framebuf);
    if (ret != OK)
	{
#ifdef DEBUG          
        echo_error_prompt(clear_video_pthread_mem_fail, __FILE__, __FUNCTION__, __LINE__);
#else
        write_log(clear_video_pthread_mem_fail, __FILE__, __FUNCTION__, __LINE__);
#endif   
        pthread_exit(clear_video_pthread_mem_fail);
    } 	
	
    pthread_exit(OK);
}
