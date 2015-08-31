#ifndef _GPS_H
#define _GPS_H
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
void gps_parse(char *line,GPS_INFO *GPS);
void show_gps(GPS_INFO *GPS);
#endif
