#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>  
#include "gps.h"
#include <string.h>
#define BAUDRATE B9600      //���ڲ����ʣ���GPSͨ��COM1���������ݴ�����
#define COM1 "/dev/ttyS1"   //������GPS���ӵĴ���1��ʵ��λ�ã���λ�������2410���ϵ�ʵ��������
GPS_INFO gps_info;                 //��ŷ�����gps���ݣ���һ���ṹ�壬��gps.h�ж���
char GPS_BUF[1024];                //��ŴӴ��ڶ�����GPSԭʼ����
static int baud=BAUDRATE;
volatile int fd;                   //�򿪴��ں󱣴���豸������
void* receive() //���յ�һ������ʱ���Իس�����Ϊ�ֽ���ʶ��һ���ַ�������ԭ����������ַ��е�6����Cʱ�����ʾ��������Ҫ�����ݴ�����$GPRMC��ͷ������Ըô����з����ʱ�估��γ�Ȳ������������ֻ��ԭ�������
{
   int i=0;
   char c;     
   char buf[1024];
   printf("read gps\n");
   while (1)
   {
     read(fd,&c,1);    //�Ӵ򿪵Ĵ���COM1����ΪGPSģ��ӵ���COM1����ȡ1���ַ�
	 //printf("read a char");
	 if (i < 10){
		 buf[i++] = c;
		 printf("read a char %d\n",c);
		 printf("i de zhi is %d", i);
	 }
	 //buf[11] = '\0';
	 //printf("%s", buf);
  //   if(c == '\n'){
  //      strncpy(GPS_BUF,buf,i); //��buf�и���i���ַ���GPS_BUF��
  //      i=0;                    //�������ݵ���ʼλ��
  //      printf("%s",GPS_BUF);   //ԭ�����
  //      if(buf[5]=='C'){
  //          gps_parse(GPS_BUF,&gps_info);  //����ԭʼ����
  //          show_gps(&gps_info);           //��ʾ����������
  //     }
  //    }
   }
   printf("exit from reading gps\n");
   //return NULL;
}
int main()
{
    struct termios options;         //�������õĽṹ
    fd=open(COM1,O_RDWR | O_NONBLOCK);
    if (fd <0) {
       perror(COM1);
       exit(-1);
   }
   tcgetattr(fd,&options);
  
    options.c_cflag = baud | CRTSCTS | CS8 | CLOCAL | CREAD;    
    options.c_iflag = IGNPAR;    //������żУ��
    options.c_oflag = 0;     //Raw ģʽ���  //��������ԭʼ�������
    options.c_lflag = 0; //������ԭʼ��������  
    options.c_cc[VMIN]=100;     // ������ֱ����ȡ��һ���ַ�
    options.c_cc[VTIME]=0;   // ��ʹ���ַ���ļ�ʱ�� 
    
    //��������ߣ������µĴ�������
    tcflush(fd, TCIFLUSH);   //ˢ���������
    tcsetattr(fd,TCSANOW,&options);            //������������Ч
	printf("set options\n");
    if(receive()==NULL) 
		exit(-1);//���ý��պ�����ȡGPS�񵽵�����
    exit(0);
}
