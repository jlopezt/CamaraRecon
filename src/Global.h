/*
 * Definiciones comunes para todos los modulos
 */
 
/***************************** Defines *****************************/
#ifndef _GLOBAL_
#define _GLOBAL_

//Defines generales
#define NOMBRE_FAMILIA   "Camara reconocimiento facial"
#define VERSION          "1.6.0"// (ESP32 2.0|Migracion C++|OTA|MQTTS|FicherosWeb|FTP|cargador|Nueva Web)"
#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
//#define OK                0

//Definicion de pines
#define MAX_PINES        7 //numero de pines disponibles para entradas y salidas
#define MAX_ENTRADAS     4 //numero maximo de reles soportado
#define MAX_SALIDAS      MAX_PINES-MAX_ENTRADAS //numero maximo de salidas

#define ANCHO_PULSO 1*1000 //Ancho del pulso en milisegundos

#ifndef NO_CONFIGURADO 
#define NO_CONFIGURADO -1
#endif

#ifndef CONFIGURADO 
#define CONFIGURADO     1
#endif

//Sistema de ficheros
//#define SISTEMA_FICHEROS_SD SD

#ifndef SISTEMA_FICHEROS_SD
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
#else
    #define GLOBAL_CONFIG_FILE               "/config/Config.json"
    #define GLOBAL_CONFIG_BAK_FILE           "/config/Config.json.bak"
    #define ENTRADAS_CONFIG_FILE             "/config/EntradasConfig.json"
    #define ENTRADAS_CONFIG_BAK_FILE         "/config/EntradasConfig.json.bak"
    #define SALIDAS_CONFIG_FILE              "/config/SalidasConfig.json"
    #define SALIDAS_CONFIG_BAK_FILE          "/config/SalidasConfig.json.bak"
    #define WIFI_CONFIG_FILE                 "/config/WiFiConfig.json"
    #define WIFI_CONFIG_BAK_FILE             "/config/WiFiConfig.json.bak"
    #define MQTT_CONFIG_FILE                 "/config/MQTTConfig.json"
    #define MQTT_CONFIG_BAK_FILE             "/config/MQTTConfig.json.bak"
    #define FTP_CONFIG_FILE                  "/config/FTPConfig.json"
    #define FTP_CONFIG_BAK_FILE              "/config/FTPConfig.json.bak"
    #define SECUENCIADOR_CONFIG_FILE         "/config/SecuenciadorConfig.json"
    #define SECUENCIADOR_CONFIG_BAK_FILE     "/config/SecuenciadorConfig.json.bak"
    #define FACE_RECON_CONFIG_FILE           "/config/FaceReconConfig.json"
    #define FACE_RECON_CONFIG_BAK_FILE       "/config/FaceReconConfig.json.bak"
    #define FACE_RECON_FACES_FILE            "/config/FaceReconConfig.bin"
    #define FACE_RECON_FACES_BAK_FILE        "/config/FaceReconConfig.bin.bak"
#endif
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Arduino.h>
#include <memory>

#ifndef SISTEMA_FICHEROS_SD
    #include <SistemaFicheros.h>
#else
    #include <SistemaFicherosSD.h>
#endif

#include <ArduinoJson.h>
#include <cacharro.h>
#include <SNTP.h>

#include <Salidas.h>
#include <Entradas.h>

//#include <ESP32FtpServer.h>
//#include <Secuenciador.h>
#include <MQTT.h>
#include <ArduinoOTA.h>
#include <RedWifi.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
/***************************** Includes *****************************/
        
extern int debugGlobal;
extern boolean trazaMemoria;

#endif