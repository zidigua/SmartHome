#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <alsa/asoundlib.h>  
#include "../../include/msp_cmn.h"
#include "../../include/qivw.h"
#include "../../include/msp_errors.h"

#define IVW_AUDIO_FILE_NAME "audio/awake.pcm"
#define FRAME_LEN	640 //16k采样率的16bit音频，一帧的大小为640B, 时长20ms
#define ALSA_PCM_NEW_HW_PARAMS_API  

int success_flags = 0;
long loops;  
int rc;  
int size;  
snd_pcm_t *handle;  
snd_pcm_hw_params_t *params;  
unsigned int val;  
int dir;  
snd_pcm_uframes_t frames;  
char *buffer;  

void sleep_ms(int ms)
{
	usleep(ms * 1000);
}
	
void init_sound_capture()
{
	rc = snd_pcm_open(&handle, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0);  
	if (rc < 0) {  
		fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(rc));  
		exit(1);  
	}  

	/* Allocate a hardware parameters object. */  
	snd_pcm_hw_params_alloca(&params);  

	/* Fill it in with default values. */  
	snd_pcm_hw_params_any(handle, params);  

	/* Set the desired hardware parameters. */  

	/* Interleaved mode */  
	snd_pcm_hw_params_set_access(handle, params,  
			SND_PCM_ACCESS_RW_INTERLEAVED);  

	/* Signed 16-bit little-endian format */  
	snd_pcm_hw_params_set_format(handle, params,  
			SND_PCM_FORMAT_S16_LE);  

	/* One channels (stereo) */  
	snd_pcm_hw_params_set_channels(handle, params, 1);  

	/* 16000 bits/second sampling rate (CD quality) */  
	val = 16000;  
	snd_pcm_hw_params_set_rate_near(handle, params,  
			&val, &dir);  

	/* Set period size to 32 frames. */  
	frames = 32;  
	snd_pcm_hw_params_set_period_size_near(handle,  
			params, &frames, &dir);  

	/* Write the parameters to the driver */  
	rc = snd_pcm_hw_params(handle, params);  
	if (rc < 0) {  
		fprintf(stderr,  
				"unable to set hw parameters: %s\n",  
				snd_strerror(rc));  
		exit(1);  
	}  

	/* Use a buffer large enough to hold one period */  
	snd_pcm_hw_params_get_period_size(params,  
			&frames, &dir);  
	size = frames * 2; /* 2 bytes/sample, 2 channels */  
	buffer = (char *) malloc(size);  

	/* We want to loop for 2 seconds */  
	snd_pcm_hw_params_get_period_time(params,  
			&val, &dir);  
}

void sound_capture_save_file()
{
	int fd;
	fd = open(IVW_AUDIO_FILE_NAME,O_CREAT|O_WRONLY|O_TRUNC,0664);
	if(fd == -1){
		printf("open file error\n");
		exit(1);
	}

	loops = 2000000 / val;  

	while (loops > 0) {  
		loops--;  
		rc = snd_pcm_readi(handle, buffer, frames);  
		if (rc == -EPIPE) {  
			/* EPIPE means overrun */  
			fprintf(stderr, "overrun occurred\n");  
			snd_pcm_prepare(handle);  
		} else if (rc < 0) {  
			fprintf(stderr,  
					"error from read: %s\n",  
					snd_strerror(rc));  
		} else if (rc != (int)frames) {  
			fprintf(stderr, "short read, read %d frames\n", rc);  
		}  
		rc = write(fd, buffer, size);  
		if (rc != size)  
			fprintf(stderr,  
					"short write: wrote %d bytes\n", rc);  
	}  
	
	close(fd);

}

int cb_ivw_msg_proc( const char *sessionID, int msg, int param1, int param2, const void *info, void *userData )
{
	if (MSP_IVW_MSG_ERROR == msg) //唤醒出错消息
	{
		printf("\n\nMSP_IVW_MSG_ERROR errCode = %d\n\n", param1);
	}
	else if (MSP_IVW_MSG_WAKEUP == msg) //唤醒成功消息
	{
		success_flags = 1;
		printf("唤醒成功\n");
		//printf(msg"\n\nMSP_IVW_MSG_WAKEUP result = %s\n\n", info);
	}
	return 0;
}

void run_ivw(const char *grammar_list, const char* audio_filename ,  const char* session_begin_params)
{
	const char *session_id = NULL;
	int err_code = MSP_SUCCESS;
	FILE *f_aud = NULL;
	long audio_size = 0;
	long real_read = 0;
	long audio_count = 0;
//	int count = 0;
	int audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
	char *audio_buffer=NULL;
	char sse_hints[128];

	session_id=QIVWSessionBegin(grammar_list, session_begin_params, &err_code);
	if (err_code != MSP_SUCCESS)
	{
		printf("QIVWSessionBegin failed! error code:%d\n",err_code);
		goto exit;
	}

	err_code = QIVWRegisterNotify(session_id, cb_ivw_msg_proc,NULL);
	if (err_code != MSP_SUCCESS)
	{
		snprintf(sse_hints, sizeof(sse_hints), "QIVWRegisterNotify errorCode=%d", err_code);
		printf("QIVWRegisterNotify failed! error code:%d\n",err_code);
		goto exit;
	}
	printf("我在听请讲.............\n");
loop:
	sound_capture_save_file();

	if (NULL == audio_filename)
	{
		printf("params error\n");
		return;
	}

	f_aud=fopen(audio_filename, "rb");
	if (NULL == f_aud)
	{
		printf("audio file open failed! \n");
		return;
	}
	fseek(f_aud, 0, SEEK_END);
	audio_size = ftell(f_aud);
	fseek(f_aud, 0, SEEK_SET);
	audio_buffer = (char *)malloc(audio_size);
	if (NULL == audio_buffer)
	{
		printf("malloc failed! \n");
		goto exit;
	}
	real_read = fread((void *)audio_buffer, 1, audio_size, f_aud);
	if (real_read != audio_size)
	{
		printf("read audio file failed!\n");
		goto exit;
	}

	while(1)
	{
		long len = 7*FRAME_LEN; //16k音频，10帧 （时长200ms）
		audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		if(audio_size <= len)
		{
			len = audio_size;
			audio_stat = MSP_AUDIO_SAMPLE_LAST; //最后一块
		}
		if (0 == audio_count)
		{
			audio_stat = MSP_AUDIO_SAMPLE_FIRST;
		}

		//	printf("csid=%s,count=%d,aus=%d\n",session_id, count++, audio_stat);
		err_code = QIVWAudioWrite(session_id, (const void *)&audio_buffer[audio_count], len, audio_stat);
		if (MSP_SUCCESS != err_code)
		{
			printf("QIVWAudioWrite failed! error code:%d\n",err_code);
			snprintf(sse_hints, sizeof(sse_hints), "QIVWAudioWrite errorCode=%d", err_code);
			goto exit;
		}
		if (MSP_AUDIO_SAMPLE_LAST == audio_stat)
		{
			break;
		}
		audio_count += len;
		audio_size -= len;

		sleep_ms(30); //模拟人说话时间间隙，10帧的音频时长为200ms
	}
	if(success_flags == 0) 
	{
		fclose(f_aud);
		success_flags = 0;
		audio_count = 0;
		audio_size = 0;
		//printf("唤醒失败,重新采集音频\n");
		goto loop;
	}
	snprintf(sse_hints, sizeof(sse_hints), "success");

exit:
	if (NULL != session_id)
	{
		QIVWSessionEnd(session_id, sse_hints);
	}
	if (NULL != f_aud)
	{
		fclose(f_aud);
	}
	if (NULL != audio_buffer)
	{
		free(audio_buffer);
	}
}


int main(int argc, char* argv[])
{
	int         ret       = MSP_SUCCESS;
	const char *lgi_param = "appid = 5e438e7a,work_dir = .";
	const char *ssb_param = "ivw_threshold=0:1450,sst=wakeup,ivw_res_path =fo|res/ivw/wakeupresource.jet";
	
	init_sound_capture();

	ret = MSPLogin(NULL, NULL, lgi_param);
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code: %d.\n", ret);
		goto exit ;//登录失败，退出登录
	}
	printf(">>>>>>>>>>>>>>>>>>>>开始唤醒<<<<<<<<<<<<<<<<<<<<<<\n");

	run_ivw(NULL, IVW_AUDIO_FILE_NAME, ssb_param); 
	printf(">>>>>>>>>>>>>>>>>>>>唤醒成功<<<<<<<<<<<<<<<<<<<<<<\n");

exit:

	snd_pcm_drain(handle);  
	snd_pcm_close(handle);  
	free(buffer);  

	//	printf("按任意键退出 ...\n");
	//	getchar();
	MSPLogout(); //退出登录
	return 0;
}
