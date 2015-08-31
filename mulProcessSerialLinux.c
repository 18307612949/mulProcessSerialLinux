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
#define buffLen 1024
#define rcvTimeOut 4
#define fileSave "fileSave.txt"
#define fileSave1 "fileSave1.txt"


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

	/*************************************************/
	else if (comport == 6)
	{
		//fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
		//fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
		fd = open("/dev/ttyACM1", O_RDWR | O_NOCTTY);
		if (-1 == fd)
		{
			perror("Can't Open Serial Port");
			return(-1);
		}
		else
		{
			printf("open ttyACM1 .....\n");
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
//���ǲ��������ȡ�����˰ɣ�ֱ��ʹ��read()����,��Ȼ�ˣ��������ȡ��ʵҲûʲô����
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
int dataSave(int fd, int fdFileSave, int portNum, int baudRate,int setOptRet){

	unsigned int readDataNum = 0;
	char buffRcvData[buffLen] = { 0 };

	if ((fd = openPort(fd, portNum))<0)
	{
		perror("open_port error");
		return -1;
	}
	//setOpt(fdSerial, 9600, 8, 'N', 1)
	if ((setOptRet = setOpt(fd, baudRate, 8, 'N', 1))<0)
	{
		perror("set_opt error");
		return -1;
	}
	printf("Serial fdSerial=%d\n", fd);

	tcflush(fd, TCIOFLUSH);//������ڻ���
	fcntl(fd, F_SETFL, 0);//���������ΪĬ�ϵ�����ģʽ��

	while (1){
		readDataNum = read(fd, buffRcvData, buffLen);
		write(fdFileSave, buffRcvData, readDataNum);//�����ݱ������ļ�fdFileSave ��"fileSave.txt"
	}
}
int serialSubProcess(int num,int max)
{
	//�ӽ���PID
	pid_t pid;

	//�����ӽ��̹��õķ��ض�ȡ���ݸ���readDataNum���Լ���ȡ����ʱ�Ĵ洢��������buffRcvData[buffLen]
	unsigned int readDataNum = 0;
	char buffRcvData[buffLen] = { 0 };

	//����ֵ----setOpt()����ֵ
	int iSetOpt = 0;
	//int iSetOpt1 = 0;

	//�����ļ�������----openPort(fdSerial, 5))<0)���򿪴����豸�ķ��ص��ļ�������
	int fdSerial = 0;
	int fdSerial1 = 0;

	//���ݱ����txt�ļ����ļ�������----open(fileSave, O_WRONLY | O_CREAT | O_APPEND)) == -1)���򿪱����ļ��ķ��ص��ļ�������
	int fdFileSave = 0;
	int fdFileSave1 = 0;

	if ((fdFileSave = open(fileSave, O_WRONLY | O_CREAT | O_APPEND)) == -1)
	{
		printf("Open %s Error\n", fileSave);
		exit(1);
	}

	if ((fdFileSave1 = open(fileSave1, O_WRONLY | O_CREAT | O_APPEND)) == -1)
	{
		printf("Open %s Error\n", fileSave1);
		exit(1);
	}

	if (num >= max)return;
	pid = fork();

	//������ʧ��
	if (pid<0)
	{
		perror("fork error!\n");
		exit(1);
	}

	//�ӽ���----�����ӽ���   
	else if (pid == 0)
	{
		sleep(3);
		//printf("Son Process id=%d,Parent Process id=%d\n", getpid(), getppid());
		switch (num){
		case 0:
			printf("**********************************************\n");
			printf("Son Process id=%d,Parent Process id=%d\n", getpid(), getppid());

			//int dataSave(int fd, int fdFileSave, int portNum, int baudRate, int setOptRet)
			dataSave(fdSerial, fdFileSave, 5, 9600, iSetOpt);
			printf("the  /dev/ttyACM0 is receving data! \n");

			//openPort
			//1--"/dev/ttyS0",2--"/dev/ttyS1",3--"/dev/ttyS2",4--"/dev/ttyUSB0" С��������2--"/dev/ttyS1"
			//5--"/dev/ttyACM0"�������APM���ӵ�USB������Ubuntu�ϵ��豸��ʾ��,6--"/dev/ttyACM0"

			//test
			//if ((fdSerial = openPort(fdSerial, 5))<0)
			//{
			//	perror("open_port error");
			//	return -1;
			//}
			////setOpt(fdSerial, 9600, 8, 'N', 1)
			//if ((iSetOpt = setOpt(fdSerial, 9600, 8, 'N', 1))<0)
			//{
			//	perror("set_opt error");
			//	return -1;
			//}
			//printf("Serial fdSerial=%d\n", fdSerial);

			//tcflush(fdSerial, TCIOFLUSH);//������ڻ���
			//fcntl(fdSerial, F_SETFL, 0);//���������ΪĬ�ϵ�����ģʽ��

			///*readDataNum = readDataTty(fdSerial, buffRcvData, rcvTimeOut, buffLen);
			//while (1){
			//	printNum++;
			//	printf("%d\n", printNum);
			//}*/
			//
			//while (1){
			//	readDataNum = read(fdSerial, buffRcvData, buffLen);
			//	write(fdFileSave, buffRcvData, readDataNum);//�����ݱ������ļ�fdFileSave ��"fileSave.txt"
			//}
			//test
			break;
			
		case 1:
			printf("**********************************************\n");
			printf("Son Process id=%d,Parent Process id=%d\n", getpid(), getppid());

			//int dataSave(int fd, int fdFileSave, int portNum, int baudRate, int setOptRet)
			dataSave(fdSerial1, fdFileSave1, 6, 9600, iSetOpt);
			printf("the  /dev/ttyACM1 is receving data! \n");

			//openPort
			//1--"/dev/ttyS0",2--"/dev/ttyS1",3--"/dev/ttyS2",4--"/dev/ttyUSB0" С��������2--"/dev/ttyS1"
			//5--"/dev/ttyACM0"�������APM���ӵ�USB������Ubuntu�ϵ��豸��ʾ��,6--"/dev/ttyACM1"

			//test
			//if ((fdSerial1 = openPort(fdSerial1, 6))<0)
			//{
			//	perror("open_port error");
			//	return -1;
			//}
			////setOpt(fdSerial, 9600, 8, 'N', 1)
			//if ((iSetOpt1 = setOpt(fdSerial1, 9600, 8, 'N', 1))<0)
			//{
			//	perror("set_opt error");
			//	//return -1;
			//}
			//printf("Serial fdSerial=%d\n", fdSerial1);

			//tcflush(fdSerial1, TCIOFLUSH);//������ڻ���
			//fcntl(fdSerial1, F_SETFL, 0);//���������ΪĬ�ϵ�����ģʽ��

			///*readDataNum = readDataTty(fdSerial, buffRcvData, rcvTimeOut, buffLen);
			//while (1){
			//printNum++;
			//printf("%d\n", printNum);
			//}*/

			//while (1){
			//	readDataNum = read(fdSerial1, buffRcvData, buffLen);
			//	write(fdFileSave1, buffRcvData, readDataNum);//�����ݱ������ļ�fdFileSave1 ��"fileSave1.txt"
			//}
			//test
			break;
			
		case 2:
			//printf("**********************************************\n");
			//printf("the  /dev/ttyACM0 is receving data! \n");
			break;

		default:
			break;			
		}
	}
	//������----���븸����
	else
	{
		num++;
		if (num == 1)printf("Parent Process id=%d\n", getpid());
		if (num<max)serialSubProcess(num,max);

		//�˴���sleep��Ϊ�˷�ֹ���������˳�,�Ӷ������쳣   
		sleep(5);
	}
}

int main(int argc, char** argv)
{

	int subProcessNum = 0;
	int subProcessMax = 3;

	serialSubProcess(subProcessNum, subProcessMax);

	return 0;

}