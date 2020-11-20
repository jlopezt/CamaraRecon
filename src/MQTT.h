/**********************************************/
/*                                            */
/*  Gestion de la conexion MQTT               */
/*  Incluye la conexion al bus y la           */
/*  definicion del callback de suscripcion    */
/*                                            */
/* Librria de sooprte del protocolo MQTT      */
/* para arduino/ESP8266/ESP32                 */
/*                                            */
/* https://pubsubclient.knolleary.net/api.html*/
/**********************************************/

/***************************** Defines *****************************/
#ifndef _MQTT_
#define _MQTT_
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
/***************************** Includes *****************************/

//funcion de callback para gestion de mensajes. No puede ser miembro de la clase
void callbackMQTT(char* topic, byte* payload, unsigned int length);

class miMQTTClass
{
private:
//Definicion de variables privadas
PubSubClient clienteMQTT;

String caCert; //Contenedor para el certificado de CA

String modoMQTT;  //Tipo de seguridad (MQTT vs MQTTS)
IPAddress IPBroker; //IP del bus MQTT
String BrokerDir; //IP o URL del broker
uint16_t puertoBroker; //Puerto del bus MQTT
uint16_t timeReconnectMQTT; //Tiempo de espera en la reconexion al bus
String usuarioMQTT; //usuario par ala conxion al broker
String passwordMQTT; //password parala conexion al broker
String topicRoot; //raiz del topic a publicar. Util para separar mensajes de produccion y prepropduccion
String ID_MQTT; //ID del modulo en su conexion al broker
String topicKeepAlive; //Topic para los mensajes de keepAlive
int8_t publicarEntradas; //Flag para determinar si se envia el json con los valores de las entradas
int8_t publicarSalidas; //Flag para determinar si se envia el json con los valores de las salidas
//Definicion de funciones privadas
boolean recuperaDatosMQTT(boolean debug);
boolean parseaConfiguracionMQTT(String contenido);
boolean conectaMQTT(void);
String generaJSONPing(boolean debug);

public:
WiFiClient espClient;
WiFiClientSecure espClientSSL;

void inicializaMQTT(void);
boolean enviarMQTT(String topic, String payload);
void atiendeMQTT(boolean debug);
void enviaDatos(boolean debug);
String stateTexto(void);

String gettopicRoot(void) {return topicRoot;};
String getID_MQTT(void) {return ID_MQTT;};
IPAddress getIPBroker(void) {return IPBroker;};
String getBrokerDir(void){return BrokerDir;}
String getBroker(void){
  if (BrokerDir!="") return BrokerDir;
  
  return IPBroker.toString();
}
uint16_t getPuertoBroker(void) {return puertoBroker;};
String getUsuarioMQTT(void) {return usuarioMQTT;};
String getPasswordMQTT(void) {return passwordMQTT;};
String getWillTopic(void);
String getWillMessage(void);
uint8_t getCleanSession(void);

int8_t getPublicarEntradas(void);
void setPublicarEntradas(int8_t pubEnt);

int8_t getPublicarSalidas(void);
void setPublicarSalidas(int8_t pubSal);

void respondePingMQTT(char* topic, byte* payload, unsigned int length);
};

extern miMQTTClass miMQTT;

#endif