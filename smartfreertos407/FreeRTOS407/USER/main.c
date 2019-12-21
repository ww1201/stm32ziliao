#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "24l01.h" 
#include "gps.h" 
#include "usart3.h"
#include "usart2.h"
#include "beep.h"
#include "esp8266.h"
#include "string.h"
#include "key.h"
/************************************************
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��4-1
 FreeRTOS���񴴽���ɾ��(��̬����)-�⺯���汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/


//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define NRF_TASK_PRIO		3
//�����ջ��С	
#define NRF_TASK_STK_SIZE 		50  
//������
TaskHandle_t NRF_Task_Handler;
//������
void NRF_task_t(void *pvParameters);

//�������ȼ�
#define GPS_TASK_PRIO		2
//�����ջ��С	
#define GPS_TASK_STK_SIZE 		50  
//������
TaskHandle_t GPSTask_Handler;
//������
void GPS_task_t(void *pvParameters);


//�������ȼ�
#define MQTT_TEST_PRIO		1
//�����ջ��С	
#define MQTT_TEST_STK_SIZE 		500 
//������
TaskHandle_t MQTTTask_Handler;
//������
void MQTT_TEST_t(void *pvParameters);
//
#define SSID    "ZWDEHONOR"
#define PASSWORD  "zw981201"
//
#define MQTTSEVERID   "192.168.43.195"
#define MQTTPORTNUM   "1883"
#define MQTTUSERNAME  "username"
#define MQTTPASSWORD  "password"
//
u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//����1,���ͻ�����
nmea_msg gpsx; 											//GPS��Ϣ
__align(4) u8 dtbuf[50];   								//��ӡ������
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode�ַ��� 
u16 i,rxlen;
u8 upload=0;
//


char str5[30],str6[30];
void Gps_Msg_Show(void)
{
 	float tp;		   
	POINT_COLOR=BLUE;  	 
	tp=gpsx.longitude;	   
	sprintf((char *)dtbuf,"%.5f",tp/=100000,gpsx.ewhemi);	//�õ������ַ���
 	LCD_ShowString(30,200,200,16,16,dtbuf);	 
  strcpy(str5,(char *)dtbuf);	
	//MQTT_Publish("longitude",str5,0);
	tp=gpsx.latitude;	   
	sprintf((char *)dtbuf,"%.5f",tp/=100000,gpsx.nshemi);	//�õ�γ���ַ���
 	LCD_ShowString(30,220,200,16,16,dtbuf);
  strcpy(str6,(char *)dtbuf);		
	//MQTT_Publish("latitude",str6,0);
//	tp=gpsx.altitude;	   
// 	sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//�õ��߶��ַ���
// 	LCD_ShowString(30,170,200,16,16,dtbuf);	 			   
//	tp=gpsx.speed;	   
// 	sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//�õ��ٶ��ַ���	 
// 	LCD_ShowString(30,190,200,16,16,dtbuf);	 				    
//	if(gpsx.fixmode<=3)														//��λ״̬
//	{  
//		sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);	
//	  	LCD_ShowString(30,210,200,16,16,dtbuf);			   
//	}	 	   
//	sprintf((char *)dtbuf,"Valid satellite:%02d",gpsx.posslnum);	 		//���ڶ�λ��������
// 	LCD_ShowString(30,230,200,16,16,dtbuf);	    
//	sprintf((char *)dtbuf,"Visible satellite:%02d",gpsx.svnum%100);	 		//�ɼ�������
// 	LCD_ShowString(30,250,200,16,16,dtbuf);		 
	sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//��ʾUTC����
	//printf("year2:%d\r\n",gpsx.utc.year);
	LCD_ShowString(30,240,200,16,16,dtbuf);		    
	sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//��ʾUTCʱ��
  	LCD_ShowString(30,260,200,16,16,dtbuf);		  
}	 



int main(void)
{ 
	u16 i,rxlen;
	u16 lenx;
  u8 key=0XFF;
	u8 upload=0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	usart2_init(38400);          //��ʼ������2
	usart3_init(115200);       //��ʼ������3������Ϊ115200
	//usmart_dev.init(72); 		//��ʼ��USMART 
	LED_Init();		        			//��ʼ��LED�˿�
	LCD_Init();							//��ʼ��LCD
	NRF24L01_Init();    		//��ʼ��NRF24L01 
	
	POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"ATK STM32F103/F407");	
	LCD_ShowString(30,30,200,16,16,"FreeRTOS smartxiaocar");
	LCD_ShowString(30,50,200,16,16,"KEY0:Upload NMEA Data SW");   	 										   	   
  LCD_ShowString(30,70,200,16,16,"NMEA Data Upload:OFF"); 
  while(ESP8266_init()!=0)
  {
		LCD_ShowString(30,130,200,16,16,"esp8266 Error");
		delay_ms(200);
		LCD_Fill(30,130,239,130+16,WHITE);
 		delay_ms(200);
	}	
//	while(NRF24L01_Check())
//	{
//		LCD_ShowString(30,130,200,16,16,"NRF24L01 Error");
//		delay_ms(200);
//		LCD_Fill(30,130,239,130+16,WHITE);
// 		delay_ms(200);
//	}
	if(Ublox_Cfg_Rate(1000,1)!=0)	//���ö�λ��Ϣ�����ٶ�Ϊ1000ms,˳���ж�GPSģ���Ƿ���λ. 
	{
   		LCD_ShowString(30,120,200,16,16,"NEO-6M Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//�����ж�,ֱ�����Լ�鵽NEO-6M,�����ݱ���ɹ�
		{
			usart2_init(9600);				//��ʼ������3������Ϊ9600(EEPROMû�б������ݵ�ʱ��,������Ϊ9600.)
	  		Ublox_Cfg_Prt(38400);			//��������ģ��Ĳ�����Ϊ38400
			usart2_init(38400);				//��ʼ������3������Ϊ38400
			Ublox_Cfg_Tp(1000000,100000,1);	//����PPSΪ1�������1��,������Ϊ100ms	    
			key=Ublox_Cfg_Cfg_Save();		//��������  
		}	  					 
	   	LCD_ShowString(30,120,200,16,16,"NEO-6M Set Done!!");
		delay_ms(500);
		LCD_Fill(30,120,30+200,120+16,WHITE);//�����ʾ 
	}

	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
   taskENTER_CRITICAL();           //�����ٽ���
		//����gps����
    xTaskCreate((TaskFunction_t )GPS_task_t,     	
                (const char*    )"GPS_task_t",   	
                (uint16_t       )GPS_TASK_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )GPS_TASK_PRIO,	
                (TaskHandle_t*  )&GPSTask_Handler);
    //mqtt��������								
    xTaskCreate((TaskFunction_t )MQTT_TEST_t,     	
                (const char*    )"MQTT_TEST_t",   	
                (uint16_t       )MQTT_TEST_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )MQTT_TEST_PRIO,	
                (TaskHandle_t*  )&MQTTTask_Handler);								
    //����nrf��������								
		xTaskCreate((TaskFunction_t) NRF_task_t,
							 (const char *  ) "NRF_task_t",
							 (uint16_t      ) NRF_TASK_STK_SIZE,
							 (void *        ) NULL,
							 (UBaseType_t   ) NRF_TASK_PRIO,
							(TaskHandle_t*) &NRF_Task_Handler); 
								
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}


void beep_start()
{
	BEEP=!BEEP;
	vTaskDelay(100);
	BEEP=!BEEP;
	
}
//gps������
u32 gpsda[10];
void GPS_task_t(void *pvParameters)
{
	 	float tp;		   
	  POINT_COLOR=BLUE;  	 

	  while(1)
		{
			  vTaskDelay(100);
				if(USART2_RX_STA&0X8000)		//���յ�һ��������
		    {
			    rxlen=USART2_RX_STA&0X7FFF;	//�õ����ݳ���
			    for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART2_RX_BUF[i];	   
 			    USART2_RX_STA=0;		   	//������һ�ν���
			    USART1_TX_BUF[i]=0;			//�Զ���ӽ�����
			    GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//�����ַ���
			    Gps_Msg_Show();				//��ʾ��Ϣ	
			    if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//���ͽ��յ������ݵ�����1
 		   }
	   }
}
char perstr[100];
u8 tmp_buf[33];
void NRF_task_t(void *pvParameters)
{
	  u8 len;
		u16 t=0;			 	
	  u16 ALvalue,SMvalue;
	  u8 temperature,humidity;
	  u8 pernum;
   	NRF24L01_RX_Mode();		
    LCD_ShowString(30,110,200,16,16,"SMvalue "); 
	  LCD_ShowString(30,130,200,16,16,"ALvalue ");
	  LCD_ShowString(30,150,200,16,16,"temperature ");
	  LCD_ShowString(30,170,200,16,16,"humidity ");
		while(1)
		{	  				
			if(NRF24L01_RxPacket(tmp_buf)==0)//һ�����յ���Ϣ,����ʾ����.
			{
				  len=strlen((char *)tmp_buf);
				  if(len>10)
					{
						tmp_buf[32]=0;//�����ַ���������
						SMvalue=(tmp_buf[0]-'0')*1000+(tmp_buf[1]-'0')*100+(tmp_buf[2]-'0')*10+(tmp_buf[3]-'0');
						ALvalue=(tmp_buf[4]-'0')*1000+(tmp_buf[5]-'0')*100+(tmp_buf[6]-'0')*10+(tmp_buf[7]-'0');
						temperature=(tmp_buf[8]-'0')*100+(tmp_buf[9]-'0')*10+(tmp_buf[10]-'0');
						humidity=(tmp_buf[11]-'0')*100+(tmp_buf[12]-'0')*10+(tmp_buf[13]-'0');
						SMvalue=(SMvalue-200)/3;
						ALvalue=(ALvalue-200)/3;
						tmp_buf[0]=SMvalue/1000+'0';
						tmp_buf[1]=(SMvalue%1000)/100+'0';
						tmp_buf[2]=(SMvalue%100)/10+'0';
						tmp_buf[3]=SMvalue%10+'0';
						tmp_buf[4]=ALvalue/1000+'0';
						tmp_buf[5]=(ALvalue%1000)/100+'0';
						tmp_buf[6]=(ALvalue%100)/10+'0';
						tmp_buf[7]=ALvalue%10+'0';
						if(SMvalue>500||ALvalue>400||temperature>30||humidity<10)//����������
							 beep_start();
						LCD_ShowNum(80+90,110,SMvalue,4,16);		
						LCD_ShowNum(80+90,130,ALvalue,4,16);
						LCD_ShowNum(80+90,150,temperature,3,16);
						LCD_ShowNum(80+90,170,humidity,3,16);
			    }
          else
					{
						perstr[0]=tmp_buf[0];
						perstr[1]=tmp_buf[1];
						perstr[2]=0;
						pernum=(tmp_buf[0]-'0')*10+(tmp_buf[1]-'0');
						LCD_ShowNum(80+90,200,pernum,2,16);
					}						
			}
			else{
				vTaskDelay(100);
			}
		}
}

char str1[5],str2[5],str3[5],str4[5];
void package(void)
{
	str1[0]=tmp_buf[0];
	str1[1]=tmp_buf[1];
	str1[2]=tmp_buf[2];
	str1[3]=tmp_buf[3];
	str1[4]=0;
	str2[0]=tmp_buf[4];
	str2[1]=tmp_buf[5];
	str2[2]=tmp_buf[6];
	str2[3]=tmp_buf[7];
	str2[4]=0;
	str3[0]=tmp_buf[8];
	str3[1]=tmp_buf[9];
	str3[2]=tmp_buf[10];
	str3[3]=0;
	str4[0]=tmp_buf[11];
	str4[1]=tmp_buf[12];
	str4[2]=tmp_buf[13];
	str4[3]=0;
}
//char strte[]="{\"datastreams\":[{\"id\":\"temp\",\"datapoints\":[{\"at\":\"2019-12-10-19:00\",\"value\":100}]}]}";
char strte[]="{\"id\":45,\"dp\":{\"test\":[{\"v\":30}]}}";
void MQTT_TEST_t(void *pvParameters)
{
	 u8 ack;
	 while(1)
	 {
        package();
	  	  ack=MQTT_Publish("$sys/274153/mqtttest/dp/post/json",strte,0);   //test
		  //ESP8266_SendDatapoint(1,"temp",100.0);
//			ack=MQTT_Publish("air/smog",str1,0);
//		  ack=MQTT_Publish("air/aloco",str2,0);
//		  ack=MQTT_Publish("air/temp",str3,0);
//		  ack=MQTT_Publish("air/humi",str4,0);
//		  ack=MQTT_Publish("pernum",perstr,0);
//		  ack=MQTT_Publish("longitude",str5,0);
//		  ack=MQTT_Publish("latitude",str6,0);
		  if(ack==1)
			{
				LCD_ShowString(30,190,200,16,16,"mqttpublish is successful ");
				LED0=~LED0;
        vTaskDelay(100);
        LED0=~LED0;	
			}
			vTaskDelay(500);
	 }
}


