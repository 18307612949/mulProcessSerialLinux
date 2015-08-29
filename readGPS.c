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

//#define rec_buf_wait_2s 2
//#define buffLen 1024
#define buffLen 24
#define rcvTimeOut 4
#define fileSave "fileSave.txt"
#define fileSaveGPS "fileSaveGPS.txt"

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
		fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY);
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
	newtio.c_cc[VMIN] = 0;
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
	//if ((iSetOpt = setOpt(fdGPS, 9600, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 4800, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 2400, 8, 'N', 1))<0)
	//if ((iSetOpt = setOpt(fdGPS, 115200, 8, 'N', 1))<0)
	if ((iSetOpt = setOpt(fdGPS, 1200, 8, 'N', 1))<0)
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
		ret = read(fdGPS, bufGPS, 1024);
		if (ret > 1)
		{
			if (strstr(bufGPS, "GPGGA") != NULL)
			parseData(bufGPS);
		}
		//write(fdFileSaveGPS, bufGPS, ret);//�����ݱ������ļ�fdFileSave ��"fileSave.txt"
	}
	return 0;
}