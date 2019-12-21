#include "esp8266.h"
#include "delay.h"
#include "usart3.h"
#include "lcd.h"
#include "string.h"
#include "mqtt.h"

// OneNet产品ID、鉴权信息及设备ID
//#define PROID		"291489"
//#define AUTH_INFO	"STM32F407Key"
#define DEVID		"553641200"

int ESP8266_SendCmd(char* cmd, char* reply, int wait)
{
	 usart3_send_buff((u8*)cmd,strlen(cmd));
	 delay_ms(wait);
	 usart3_rece_buff();
	 if(strcmp(reply,(char*)USART3_RX_BUF)==0)
	 {
		 return 1;
	 }
	 else
		 return 0;
	 
} 
//可参考论文上的 
int ESP8266_init()
{
	 int ret = 0;
   ret = ESP8266_SendCmd("AT+GAPSTATUS","AT+INFO=failed", 1000);
   ESP8266_SendCmd("AT+RST","", 1000);
   if (!ret)
		return 0;
	 else
    return ret;
}

int ESP8266_joinwifi(char* ssid,char* psd)
{
	  int ret = 0;
    char ssid_psd[120] = {0};
	  sprintf(ssid_psd,"AT+AP=%s,%s",ssid, psd);
    ret = ESP8266_SendCmd(ssid_psd,"AT+INFO=ok", 2000);
    return ret;
}

int ESP8266_JoinMqttSrv(char* serverip, int port,char* username,char* password)
{
    int ret = 0;
    char temp_cmd[120] = {0}; 
	  sprintf(temp_cmd, "AT+MQTTCLOUD=%s,%d,%s,%s\r\n", serverip, port,username,password);
    ret = ESP8266_SendCmd(temp_cmd, "OK", 1000);//reply要改
    if (ret)
        return 1;
    else 
        return 0;
}

int MQTT_Publish(char* topic, char* payload, int qos)
{
    int ret = 0;
    char pub_src[180] ={0};   
    sprintf(pub_src, "AT+PUB=%s,%s,%d\r\n", topic, payload, qos);
    ret = ESP8266_SendCmd(pub_src, "AT+INFO=Publish OK", 200);
    memset(pub_src,0,sizeof(pub_src));    
    if (ret)
	{
		printf("publish success to:%s\r\n", topic);
		return 1;
	}
	printf("Failed publishing to:%s\r\n", topic);    
    return ret;
}

int MQTT_Publish1(char *payload)
{
    int ret = 0;
    char pub_src[180] ={0};   
    sprintf(pub_src, "AT+PUB=%s\r\n",payload);
    ret = ESP8266_SendCmd(pub_src, "AT+INFO=Publish OK", 200);
    memset(pub_src,0,sizeof(pub_src));    
    if (ret)
	 {
		//printf("publish success to:%s\r\n", topic);
		return 1;
	 }
	//printf("Failed publishing to:%s\r\n", topic);    
    return ret;
}

int MQTT_Subscribe(const char* topic, int qos)
{
	  int ret = 0;
    char sub_str[120] = {0};
    sprintf(sub_str, "AT+SUB=%s,%d",topic, qos);   
    ret = ESP8266_SendCmd(sub_str, " AT+INFO=Subscribe OK", 1000);
	  if (ret)
	  {
		  return ret;
	  }
		else
	    return 0;
}



void ESP8266_SendDatapoint(int type, const char *data_id, float data)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};		// 协议包
	char buf[128];
	char text[128];
	short body_len = 0, i;
	printf("Tips:	OneNet_SendDatapoints-MQTT\r\n");
	memset(buf, 0, sizeof(buf));
	// 封装上报的数据体
	if(1 == type)		// JSON格式1字符串
	{
		strcpy(buf, "{\"datastreams\":[");
		memset(text, 0, sizeof(text));
		sprintf(text, "{\"id\":\"%s\",\"datapoints\":[{\"value\":%.2f}]}]},", data_id, data);
	}
	else if(5 == type)	// 自定义分隔符
	{
		strcpy(buf, ",;");
		memset(text, 0, sizeof(text));
		sprintf(text, "%s,%.2f;", data_id, data);
	}
	strcat(buf, text);
	body_len = strlen(buf);
	if(body_len)
	{
		if(MQTT_PacketSaveData(DEVID, body_len, NULL, type, &mqttPacket) == 0)// 封包
		{
			for(i = 0; i < body_len; i++)
			{
				mqttPacket._data[mqttPacket._len++] = buf[i];
			}
			//ESP8266_SendData(mqttPacket._data, mqttPacket._len);// 上传数据到平台
			MQTT_Publish1(mqttPacket._data);
			printf("Send %d Bytes\r\n", mqttPacket._len);
			MQTT_DeleteBuffer(&mqttPacket);						// 删包
		}
		else
		{
			printf("WARN:	SendDatapoints Failed\r\n");
		}
	}
	delay_ms(500);
}





