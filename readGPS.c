#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <termios.h> //set baud rate

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include<sys/stat.h>

//#define rec_buf_wait_2s 2
//#define buffLen 1024
#define buffLen 1024
#define rcvTimeOut 4
#define fileSave "fileSave.txt"
#define fileSaveGPS "fileSaveGPS.txt"

/*************************     gps  ******************************/
typedef struct{
	int year;    //��
	int month;   //��
	int day;     //��
	int hour;    //ʱ
	int minute;  //��
	int second;  //��
}date_time;  //ʱ��ṹ��
typedef struct{
	date_time D;//ʱ��
	double latitude;   //γ��
	double longitude;  //����
	char NS;           //�ϱ���
	char EW;           //����
}GPS_INFO;
//void gps_parse(char *line, GPS_INFO *GPS);
//void show_gps(GPS_INFO *GPS);

//static int GetComma(int num, char* str);       //���str�ַ�ָ���Ӧ�ַ����е�num�����ŵ�λ��
//static void UTC2BTC(date_time *GPS);          //��GPS��õ�UTCʱ��ת���ɱ���ʱ��
//static double get_double_number(char *s);     //������������м�ĸ���������ת��doubleֵ����

void gps_parse(char *line, GPS_INFO *GPS);
void show_gps(GPS_INFO *GPS);

int GetComma(int num, char* str);       //���str�ַ�ָ���Ӧ�ַ����е�num�����ŵ�λ��
void UTC2BTC(date_time *GPS);          //��GPS��õ�UTCʱ��ת���ɱ���ʱ��
double get_double_number(char *s);     //������������м�ĸ���������ת��doubleֵ����

GPS_INFO gps_info;                 //��ŷ�����gps���ݣ���һ���ṹ�壬��gps.h�ж���
char GPS_BUF[1024];

void show_gps(GPS_INFO *GPS)//���GPS����ԭʼ���ݺ�Ŀ������ݣ�����ʱ�估��γ�ȣ�
{
	printf("DATE     : %d-%02d-%02d \n", GPS->D.year, GPS->D.month, GPS->D.day);
	printf("TIME     :  %02d:%02d:%02d \n", GPS->D.hour, GPS->D.minute, GPS->D.second);
	printf("Latitude : %10.4f %c\n", GPS->latitude, GPS->NS);
	printf("Longitude: %10.4f %c\n", GPS->longitude, GPS->EW);
}
////////////////////////////////////////////////////////////////////////////////
//����gps����������
//0      7  0   4 6   0     6 8 0        90         0  3      0        9   
//$GPRMC,091400,A,3958.9870,N,11620.3278,E,000.0,000.0,120302,005.6,W*62  
void gps_parse(char *line, GPS_INFO *GPS)
{
	int tmp;
	char c;
	char* buf = line;
	c = buf[5];
	if (c == 'C'){//"GPRMC"
		GPS->D.hour = (buf[7] - '0') * 10 + (buf[8] - '0');    //ԭʼ���ݴ��е�7λ����0����Ϊʱ�ӵ�ʮλ����8Ϊ��λ
		GPS->D.minute = (buf[9] - '0') * 10 + (buf[10] - '0');    //              9               ����          10
		GPS->D.second = (buf[11] - '0') * 10 + (buf[12] - '0');    //              11              ����          12
		tmp = GetComma(9, buf);              //���buf�е�9�����ŵ���ʼλ�ã���������buf�е��±�
		GPS->D.day = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0');
		GPS->D.month = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
		GPS->D.year = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0') + 2000;
		GPS->latitude = get_double_number(&buf[GetComma(3, buf)]);      //γ��
		GPS->NS = buf[GetComma(4, buf)];                          //��s��n
		GPS->longitude = get_double_number(&buf[GetComma(5, buf)]);      //����
		GPS->EW = buf[GetComma(6, buf)];                          //��e��w
		UTC2BTC(&GPS->D);
	}
}
//������ַ���s�е�һ������ǰ����ֵ����ת��Ϊ����������
//static double get_double_number(char *s)
double get_double_number(char *s)
{
	char buf[128];
	int i;
	double rev;
	i = GetComma(1, s);
	strncpy(buf, s, i);
	buf[i] = 0;
	rev = atof(buf);
	return rev;
}
//����str�е�num�����ŵ���һλ��(��0��)
//static int GetComma(int num, char *str)
int GetComma(int num, char *str)
{
	int i, j = 0;
	int len = strlen(str);
	for (i = 0; i<len; i++)
	{
		if (str[i] == ',')j++;
		if (j == num)return i + 1;
	}
	return 0;
}
//������ʱת��Ϊ����ʱ
//static void UTC2BTC(date_time *GPS)
void UTC2BTC(date_time *GPS)
{
	//�������ȳ�,�ٳ�ʱ������,��ʱ������+1��
	GPS->second++; //��һ��
	if (GPS->second>59){
		GPS->second = 0;
		GPS->minute++;
		if (GPS->minute>59){
			GPS->minute = 0;
			GPS->hour++;
		}
	}
	GPS->hour += 8;   //����ʱ��������ʱ�����8��ʱ���������8����
	if (GPS->hour>23)
	{
		GPS->hour -= 24;
		GPS->day += 1;
		if (GPS->month == 2 || GPS->month == 4 || GPS->month == 6 || GPS->month == 9 || GPS->month == 11){
			if (GPS->day>30){
				GPS->day = 1;
				GPS->month++;
			}
		}
		else{
			if (GPS->day>31){
				GPS->day = 1;
				GPS->month++;
			}
		}
		if ((GPS->year % 4 == 0) && (GPS->year % 400 == 0 || GPS->year % 100 != 0)){  //�ж�����
			if (GPS->day > 29 && GPS->month == 2){   //������±�ƽ����¶�һ��
				GPS->day = 1;
				GPS->month++;
			}
		}
		else{
			if (GPS->day>28 && GPS->month == 2){
				GPS->day = 1;
				GPS->month++;
			}
		}
		if (GPS->month>12){
			GPS->month -= 12;
			GPS->year++;
		}
	}
}



/*********************************************************/

/*************Linux and Serial Port *********************/
/*************Linux and Serial Port *********************/
int openPort(int fd, int comport)
{

	if (comport == 1)
	{
		fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyS0 .....\n");
		}
	}
	else if (comport == 2)
	{
		//fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY);
		//fd = open("/dev/ttyS1", O_RDWR | O_NONBLOCK);
		fd = open("/dev/ttyS1", O_RDWR);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyS1 .....\n");
		}
	}
	else if (comport == 3)
	{
		fd = open("/dev/ttyS2", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyS2 .....\n");
		}
	}
	/*************************************************/
	else if (comport == 4)
	{
		fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyUSB0 .....\n");
		}
	}

	/*************************************************/
	else if (comport == 5)
	{
		//fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
		fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
		//fd = open("/dev/ttyACM1", O_RDWR | O_NOCTTY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyACM0 .....\n");
		}
	}

	if (fcntl(fd, F_SETFL, 0)<0)
	{
		printf("fcntl failed!\n");
	}
	else
	{
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
	}
	if (isatty(STDIN_FILENO) == 0)
	{
		printf("standard input is not a terminal device\n");
	}
	else
	{
		printf("is a tty success!\n");
	}
	printf("fd-open=%d\n", fd);
	return fd;
}

int setOpt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;
	if (tcgetattr(fd, &oldtio) != 0)
	{
		perror("SetupSerial 1");
		return -1;
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch (nBits)
	{
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch (nEvent)
	{
	case 'O':                     //��У��
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':                     //żУ��
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':                    //��У��
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch (nSpeed)
	{
	case 1200:
		cfsetispeed(&newtio, B1200);
		cfsetospeed(&newtio, B1200);
		break;
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if (nStop == 1)
	{
		newtio.c_cflag &= ~CSTOPB;
	}
	else if (nStop == 2)
	{
		newtio.c_cflag |= CSTOPB;
	}
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 100;//��Ȼ����Ҫ��û�����GPS�����ݾͶ�������ѽ
	//newtio.c_cc[VTIME] = 0;//��Ҫ
	//newtio.c_cc[VMIN] = 100;//���ص���Сֵ  ��Ҫ
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
	{
		perror("com set error");
		return -1;
	}
	printf("set done!\n");
	return 0;
}

int readDataTty(int fd, char *rcv_buf, int TimeOut, int Len)
{
	int retval;
	fd_set rfds;
	struct timeval tv;
	int ret, pos;
	tv.tv_sec = TimeOut / 1000;  //set the rcv wait time  
	tv.tv_usec = TimeOut % 1000 * 1000;  //100000us = 0.1s  

	pos = 0;
	while (1)
	{
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		retval = select(fd + 1, &rfds, NULL, NULL, &tv);//��ֵ��ĳЩ�ļ��ɶ�д����� 
		//retval = select(fd + 1, &rfds, NULL, NULL, NULL);
		if (retval == -1)
		{
			perror("select()");
			break;
		}
		else if (retval)
		{
			ret = read(fd, rcv_buf + pos, 1);
			if (-1 == ret)
			{
				break;
			}

			pos++;
			if (Len <= pos)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return pos;
}

int sendDataTty(int fd, char *send_buf, int Len)
{
	ssize_t ret;

	ret = write(fd, send_buf, Len);
	if (ret == -1)
	{
		printf("write device error\n");
		return -1;
	}

	return 1;
}

int serialSubProcess(int num,int max)
{
	pid_t pid;
	int iSetOpt = 0;
	int fdSerial = 0;

	int iSetOpt1 = 0;
	int fdSerial1 = 0;

	unsigned int readDataNum = 0;
	char buffRcvData[buffLen] = { 0 };
	
	int printNum = 0;
	//�򿪱����ļ�
	int fdFileSave = 0;

	if ((fdFileSave = open(fileSave, O_WRONLY | O_CREAT | O_APPEND)) == -1)
	{
		printf("Open %s Error\n", fileSave);
		exit(1);
	}

	if (num >= max)return;
	pid = fork();
	if (pid<0)
	{
		perror("fork error!\n");
		exit(1);
	}
	//�ӽ���   
	else if (pid == 0)
	{
		//sleep(3);
		printf("Son Process id=%d,Parent Process id=%d\n", getpid(), getppid());
		switch (num){
		case 0:
			//openPort
			if ((fdSerial = openPort(fdSerial, 5))<0)//1--"/dev/ttyS0",2--"/dev/ttyS1",3--"/dev/ttyS2",4--"/dev/ttyUSB0" С��������2--"/dev/ttyS1"
			{
				perror("open_port error");
				return -1;
			}
			//setOpt(fdSerial, 9600, 8, 'N', 1)
			if ((iSetOpt = setOpt(fdSerial, 9600, 8, 'N', 1))<0)
			{
				perror("set_opt error");
				//return -1;
			}
			printf("Serial fdSerial=%d\n", fdSerial);

			tcflush(fdSerial, TCIOFLUSH);//������ڻ���
			fcntl(fdSerial, F_SETFL, 0);//���������ΪĬ�ϵ�����ģʽ��

			/*readDataNum = readDataTty(fdSerial, buffRcvData, rcvTimeOut, buffLen);
			while (1){
				printNum++;
				printf("%d\n", printNum);
			}*/
			
			
			while (1){
				readDataNum = readDataTty(fdSerial, buffRcvData, rcvTimeOut, buffLen);
				write(fdFileSave, buffRcvData, readDataNum);//�����ݱ������ļ�fdFileSave ��"fileSave.txt"
			}

			break;
			
		case 1:
			printf("**********************************************\n");
			printf("this is the 1 serial subProcess\n");
			printf("**********************************************\n");
			////openPort
			//if ((fdSerial1 = openPort(fdSerial1, 0))<0)//1--"/dev/ttyS0",2--"/dev/ttyS1",3--"/dev/ttyS2",4--"/dev/ttyUSB0" С��������2--"/dev/ttyS1"
			//{
			//	perror("open_port1 error");
			//	return -1;
			//}
			////setOpt(fdSerial, 9600, 8, 'N', 1)
			//if ((iSetOpt1 = setOpt(fdSerial1, 9600, 8, 'N', 1))<0)
			//{
			//	perror("set_opt1 error");
			//	return -1;
			//}
			//printf("Serial fdSerial1=%d\n", fdSerial1);

			//tcflush(fdSerial1, TCIOFLUSH);//������ڻ���
			//fcntl(fdSerial1, F_SETFL, 0);//���������ΪĬ�ϵ�����ģʽ��
			break;
			
		default:
			break;			
		}
	}
	//������   
	else
	{
		num++;
		if (num == 1)printf("Parent Process id=%d\n", getpid());
		if (num<max)serialSubProcess(num,max);
		//�˴���sleep��Ϊ�˷�ֹ���������˳�,�Ӷ������쳣   
		//sleep(5);
		sleep(5);
	}
}

void parseData(char *buf)
{
	int ret, nQ, nN, nB, nC;

	char cX, cY, cM1, cM2;

	float fTime, fX, fY, fP, fH, fB, fD;



	if (buf == NULL)

		return;

	ret = sscanf(buf,

		"$GPGGA,%f,%f,%c,%f,%c,%d,%02d,%f,%f,%c,%f,%c,%f,%04d%02x",

		&fTime, &fX, &cX, &fY, &cY, &nQ, &nN, &fP, &fH, &cM1, &fB,

		&cM2, &fD, &nB, &nC);

	printf("x: %c %f, y: %c %f, h %f, satellite: %d\n",

		cX, fX, cY, fY, fH, nN);

}

int main(int argc, char** argv)
{
	char bufGPS[1024] ="$GPGGA,064746.000,4925.4895,N,00103.99255,E,1,05,2.1,-68.0,M,47.1,M,,0000*4F\r\n"; // �˴���ֵ���ڲ���

	pid_t pid;
	int fdGPS, i, ret;
	int iSetOpt;

	//gps
	int j, startI, endI;
	char buf[1024];
	char c;

	//�򿪱����ļ�
	int fdFileSaveGPS = 0;

	if ((fdFileSaveGPS = open(fileSaveGPS, O_WRONLY | O_CREAT | O_APPEND)) == -1)
	{
		printf("Open %s Error\n", fileSaveGPS);
		exit(1);
	}

	//openPort
	if ((fdGPS = openPort(fdGPS, 2))<0)//1--"/dev/ttyS0",2--"/dev/ttyS1",3--"/dev/ttyS2",4--"/dev/ttyUSB0" С��������2--"/dev/ttyS1"
	{
		perror("open_port error");
		return -1;
	}
	//setOpt(fdSerial, 9600, 8, 'N', 1)
	if ((iSetOpt = setOpt(fdGPS, 9600, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 4800, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 2400, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 115200, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 1200, 8, 'N', 1))<0)
	{
		perror("set_opt error");
		return -1;
	}

	printf("Serial fdSerial=%d\n", fdGPS);

	tcflush(fdGPS, TCIOFLUSH);//������ڻ���
	fcntl(fdGPS, F_SETFL, 0);//���������ΪĬ�ϵ�����ģʽ��	

	//for (i = 0; i < 100; i++)
	//{
	//	ret = read(fdGPS, bufGPS, 1024);
	//	write(fdFileSaveGPS, bufGPS, ret);
	//	/*if (ret > 1)
	//	{
	//		if (strstr(bufGPS, "GPGGA") != NULL)
	//		parseData(bufGPS);
	//	}*/
	//}
	while (1){
		read(fdGPS, &c, 1);    //�Ӵ򿪵Ĵ���COM1����ΪGPSģ��ӵ���COM1����ȡ1���ַ�
		buf[i++] = c;
		if (c == '\n'){
			strncpy(GPS_BUF, buf, i); //��buf�и���i���ַ���GPS_BUF��
			i = 0;                    //�������ݵ���ʼλ��
			printf("%s", GPS_BUF);   //ԭ�����
			if (buf[5] == 'C'){
				gps_parse(GPS_BUF, &gps_info);  //����ԭʼ����
				show_gps(&gps_info);           //��ʾ����������
			}
		}
		/*********************************************************************************/
		//ret = read(fdGPS, bufGPS, 1024);
		//if (ret > 1)
		//{
		//	/*printf("\n GPS DATALen=%d\n", ret);
		//	bufGPS[ret] = '\0';*/

		//	/*for (i = 0; i < ret; i++){
		//		printf("%c", bufGPS[i]);
		//	}*/

		//	//wangbo�Լ�д��
		//	for (i = 0; i < ret; i++){
		//		if ('$'==bufGPS[i] ){
		//			startI = i;
		//		}
		//		else if ('\n' == bufGPS[i]){
		//			endI = i;
		//			break;
		//		}
		//	}
		//	printf("%d    %d\n", startI, endI);
		//	for (j = 0, i = startI; i < endI; i++,j++){
		//		buf[j] = bufGPS[i];
		//	}
		//	strncpy(GPS_BUF, buf, endI-startI);
		//	if (buf[5] == 'C'){
		//		          gps_parse(GPS_BUF,&gps_info);  //����ԭʼ����
		//		          show_gps(&gps_info);           //��ʾ����������
		//	}




		//	//�����2����Ҫ
		//	//printf("GPS %s\n", bufGPS); //�����������
		//	//if (strstr(bufGPS, "GPGGA") != NULL)
		//	//parseData(bufGPS);

		//}
		/*********************************************************************************/

		//write(fdFileSaveGPS, bufGPS, ret);//�����ݱ������ļ�fdFileSave ��"fileSave.txt"
		sleep(2);
	}
	close(fdGPS);
	return 0;
}
