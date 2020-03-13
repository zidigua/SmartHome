#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "tcp_core.h"
#define BUF_SIZE 4096

char result_buf[BUF_SIZE] = {0};
char table_buf[BUF_SIZE] = {0};
char recv_buf[BUF_SIZE] = {0};
char *send_p = NULL;

int main(int argc, const char *argv[])
{
	FILE * fp_get,*fp_tab,*fp_set;
	int ret=0;

	//检查参数是否正确
	if(argc < 3){
		printf("args error:usage <process> <ip> <port>\n");
		return -1;
	}

	//打开语音解析结果文件
	fp_get = fopen("./result/result.txt","rb");
	if(fp_get == NULL){
		perror("fopen result.txt");
		return -1;
	}

	//打开设备列表文件
	fp_tab = fopen("./result/device_table.txt","rb");
	if(fp_tab == NULL){
		perror("fopen device_table.txt");
		return -1;
	}

	//存放QT返回结果的文件
	fp_set = fopen("./result/current.txt","wb");
	if(fp_set == NULL){
		perror("fopen current.txt");
		return -1;
	}


	//初始化tcp
	ret = init_tcp(argv[1],argv[2]);
	if(ret == -1){
		goto error_stp;
	}

	//读取解析结果
	fread(result_buf,1,BUF_SIZE,fp_get);
	printf("result_buf = %s\n",result_buf);

	//读取设备列表文件
	while(fgets(table_buf,BUF_SIZE,fp_tab) != NULL){
		//开始比较
		if(!strncmp(result_buf, table_buf,strlen(result_buf))){
			//比成功解析数据/发送数据
			send_p = strstr(table_buf,"[");
			if(send_p == NULL){
				perror("device table file error");
				ret = -1;
				goto error_stp;
			}
			if((ret = tcp_send(send_p+1))== -1)
				goto error_stp;

			//接收QT返回值
			if((ret = tcp_recv(recv_buf))==-1){
				goto error_stp;
			}else{
				fwrite(recv_buf,1,BUF_SIZE,fp_set);
			//	memcpy(recv_buf,"厨房灯已打开",BUF_SIZE);
			//	fwrite(recv_buf,1,BUF_SIZE,fp_set);
			}
		}

	}

error_stp:
	fclose(fp_get);
	fclose(fp_tab);
	fclose(fp_set);
	tcp_close();

	return ret;
}
