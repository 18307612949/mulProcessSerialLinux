#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "gps.h"
static int GetComma(int num,char* str);       //���str�ַ�ָ���Ӧ�ַ����е�num�����ŵ�λ��
static void UTC2BTC(date_time *GPS);          //��GPS��õ�UTCʱ��ת���ɱ���ʱ��
static double get_double_number(char *s);     //������������м�ĸ���������ת��doubleֵ����
void show_gps(GPS_INFO *GPS)//���GPS����ԭʼ���ݺ�Ŀ������ݣ�����ʱ�估��γ�ȣ�
{
   printf("DATE     : %d-%02d-%02d \n",GPS->D.year,GPS->D.month,GPS->D.day);
   printf("TIME     :  %02d:%02d:%02d \n",GPS->D.hour,GPS->D.minute,GPS->D.second);
   printf("Latitude : %10.4f %c\n",GPS->latitude,GPS->NS); 
   printf("Longitude: %10.4f %c\n",GPS->longitude,GPS->EW); 
}
////////////////////////////////////////////////////////////////////////////////
//����gps����������
//0      7  0   4 6   0     6 8 0        90         0  3      0        9   
//$GPRMC,091400,A,3958.9870,N,11620.3278,E,000.0,000.0,120302,005.6,W*62  
void gps_parse(char *line,GPS_INFO *GPS)
{
   int tmp;
   char c;
   char* buf=line;
   c=buf[5];
   if(c=='C'){//"GPRMC"
      GPS->D.hour   =(buf[ 7]-'0')*10+(buf[ 8]-'0');    //ԭʼ���ݴ��е�7λ����0����Ϊʱ�ӵ�ʮλ����8Ϊ��λ
      GPS->D.minute =(buf[ 9]-'0')*10+(buf[10]-'0');    //              9               ����          10
      GPS->D.second =(buf[11]-'0')*10+(buf[12]-'0');    //              11              ����          12
      tmp = GetComma(9,buf);              //���buf�е�9�����ŵ���ʼλ�ã���������buf�е��±�
      GPS->D.day    =(buf[tmp+0]-'0')*10+(buf[tmp+1]-'0');        
      GPS->D.month  =(buf[tmp+2]-'0')*10+(buf[tmp+3]-'0');
      GPS->D.year   =(buf[tmp+4]-'0')*10+(buf[tmp+5]-'0')+2000;
      GPS->latitude =get_double_number(&buf[GetComma(3,buf)]);      //γ��
      GPS->NS       =buf[GetComma(4,buf)];                          //��s��n
      GPS->longitude=get_double_number(&buf[GetComma(5,buf)]);      //����
      GPS->EW       =buf[GetComma(6,buf)];                          //��e��w
      UTC2BTC(&GPS->D);
   }
}
//������ַ���s�е�һ������ǰ����ֵ����ת��Ϊ����������
static double get_double_number(char *s)
{
   char buf[128];
   int i;
   double rev;
   i=GetComma(1,s);  
   strncpy(buf,s,i);
   buf[i]=0;
   rev=atof(buf);
   return rev; 
}
//����str�е�num�����ŵ���һλ��(��0��)
static int GetComma(int num,char *str)
{
  int i,j=0;
  int len=strlen(str);
  for(i=0;i<len;i++)
  {
     if(str[i]==',')j++;
     if(j==num)return i+1; 
  }
  return 0; 
}
//������ʱת��Ϊ����ʱ
static void UTC2BTC(date_time *GPS)
{
   //�������ȳ�,�ٳ�ʱ������,��ʱ������+1��
   GPS->second++; //��һ��
   if(GPS->second>59){
      GPS->second=0;
      GPS->minute++;
      if(GPS->minute>59){
        GPS->minute=0;
        GPS->hour++;
      }
  } 
  GPS->hour+=8;   //����ʱ��������ʱ�����8��ʱ���������8����
  if(GPS->hour>23)
  {
     GPS->hour-=24;
     GPS->day+=1;
     if(GPS->month==2 || GPS->month==4 || GPS->month==6 || GPS->month==9 || GPS->month==11 ){
        if(GPS->day>30){
           GPS->day=1;
           GPS->month++;
        }
     }
     else{
        if(GPS->day>31){
           GPS->day=1;
           GPS->month++;
        }
   }
   if((GPS->year % 4 ==0) && (GPS->year % 400 == 0 || GPS->year % 100 != 0)){  //�ж�����
       if(GPS->day > 29 && GPS->month ==2){   //������±�ƽ����¶�һ��
          GPS->day=1;
          GPS->month++;
       }
   }
   else{
       if(GPS->day>28 &&GPS->month ==2){
         GPS->day=1;
         GPS->month++;
       }
   }
   if(GPS->month>12){
      GPS->month-=12;
      GPS->year++;
   }  
  }
}
