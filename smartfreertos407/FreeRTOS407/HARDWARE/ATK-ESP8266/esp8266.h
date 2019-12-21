#ifndef ESP8266_H
#define ESP8266_H
#include "usart3.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
int ESP8266_SendCmd(char* cmd, char* reply, int wait);
int ESP8266_init(void);
int ESP8266_joinwifi(char* ssid,char* psd);
int ESP8266_JoinMqttSrv(char* serverip, int port,char* username,char* password);
int MQTT_Publish(char* topic, char* payload, int qos);
int MQTT_Subscribe(const char* topic, int qos);
int MQTT_Publish1(char *payload);
#endif

