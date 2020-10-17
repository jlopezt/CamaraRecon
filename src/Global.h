/*
 * Definiciones comunes para todos los modulos
 */
 
/***************************** Defines *****************************/
#ifndef _GLOBAL_
#define _GLOBAL_

//Defines generales
#define NOMBRE_FAMILIA   "Camara reconocimiento facial"
#define VERSION          "1.3.3 (ESP32 2.0|Migracion C++|OTA|MQTT|FicherosWeb|FTP)"
#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
//#define OK                0

//Ficheros de configuracion
#define GLOBAL_CONFIG_FILE               "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE           "/Config.json.bak"
#define ENTRADAS_CONFIG_FILE             "/EntradasConfig.json"
#define ENTRADAS_CONFIG_BAK_FILE         "/EntradasConfig.json.bak"
#define SALIDAS_CONFIG_FILE              "/SalidasConfig.json"
#define SALIDAS_CONFIG_BAK_FILE          "/SalidasConfig.json.bak"
#define WIFI_CONFIG_FILE                 "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE             "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE                 "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE             "/MQTTConfig.json.bak"
#define FTP_CONFIG_FILE                  "/FTPConfig.json"
#define FTP_CONFIG_BAK_FILE              "/FTPConfig.json.bak"
#define SECUENCIADOR_CONFIG_FILE         "/SecuenciadorConfig.json"
#define SECUENCIADOR_CONFIG_BAK_FILE     "/SecuenciadorConfig.json.bak"
#define FACE_RECON_CONFIG_FILE           "/FaceReconConfig.json"
#define FACE_RECON_CONFIG_BAK_FILE       "/FaceReconConfig.json.bak"
#define FACE_RECON_FACES_FILE            "/FaceReconConfig.bin"
#define FACE_RECON_FACES_BAK_FILE        "/FaceReconConfig.bin.bak"

//Definicion de pines
#define MAX_PINES        7 //numero de pines disponibles para entradas y salidas
#define MAX_ENTRADAS     4 //numero maximo de reles soportado
#define MAX_RELES        MAX_PINES-MAX_ENTRADAS //numero maximo de salidas

#define ANCHO_PULSO 1*1000 //Ancho del pulso en milisegundos

#ifndef NO_CONFIGURADO 
#define NO_CONFIGURADO -1
#endif

#ifndef CONFIGURADO 
#define CONFIGURADO     1
#endif
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
#include <FtpServer.h>
#include <SistemaFicheros.h>
#include <SistemaFicherosSD.h>
#include <ArduinoJson.h>
#include <cacharro.h>
#include <SNTP.h>
#include <Salidas.h>
#include <Entradas.h>
#include <Secuenciador.h>
#include <Wifi_MQTT.h>
#include <ArduinoOTA.h>
#include <RedWifi.h>
/***************************** Includes *****************************/
#include <WiFiClientSecure.h>
#include <WiFi.h>
        
extern int debugGlobal;
extern boolean trazaMemoria;

#endif