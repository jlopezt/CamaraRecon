/***********************************************/
/*                                             */
/*  cacharro es el agregador del secuenciador  */
/*                                             */
/***********************************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <cacharro.h>
/***************************** Includes *****************************/

/***************************** Defines *****************************/
#define ESP32CAM

#if defined(ESP32CAM)
//  #define LED_BUILTIN                 33 //GPIO del led de la placa en los ESP32-CAM   
  const int LED_BUILTIN=33;
  const boolean ENCENDIDO=LOW;
  const boolean APAGADO=HIGH;
#else
//  #define LED_BUILTIN                2 //GPIO del led de la placa en los ESP32   
  const int LED_BUILTIN=4;//2;
  const boolean ENCENDIDO=HIGH;
  const boolean APAGADO=LOW;
#endif
/***************************** Defines *****************************/

cacharroClass::cacharroClass() {}

/********************************** Funciones de configuracion global **************************************/
/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo global                           */
/************************************************/
boolean cacharroClass::inicializaConfiguracion(boolean debug)
  {
  String cad="";

  nivelActivo=HIGH; //Indica si el rele se activa con HIGH o LOW. Se activa con HIGH por defecto
  nombre_dispositivo=String(NOMBRE_FAMILIA); //Nombre del dispositivo, por defecto el de la familia
  debugGlobal=0; //por defecto desabilitado  

  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  nombre_dispositivo=String(NOMBRE_FAMILIA); //Nombre del dispositivo, por defecto el de la familia
  nivelActivo=LOW;  
  
  if(!SistemaFicherosSD.leeFichero(GLOBAL_CONFIG_FILE_SD, cad)){
    //if(!SistemaFicheros.leeFichero(GLOBAL_CONFIG_FILE, cad)){
      Serial.printf("No existe fichero de configuracion global\n");
      return false;
      //}
  }

  return parseaConfiguracionGlobal(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio global                       */
/*  auto date = obj.get<char*>("date");      */
/*  auto high = obj.get<int>("high");        */
/*  auto low = obj.get<int>("low");          */
/*  auto text = obj.get<char*>("text");      */
/*********************************************/
boolean cacharroClass::parseaConfiguracionGlobal(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("nombre_dispositivo")) nombre_dispositivo=((const char *)json["nombre_dispositivo"]);    
    if(nombre_dispositivo==NULL) nombre_dispositivo=String(NOMBRE_FAMILIA);
 
    if (json.containsKey("NivelActivo")) 
      {
      if((int)json["NivelActivo"]==0) nivelActivo=LOW;
      else nivelActivo=HIGH;
      }
    
    Serial.printf("Configuracion leida:\nNombre dispositivo: %s\nNivelActivo: %i\n",nombre_dispositivo.c_str(),nivelActivo);
//************************************************************************************************
    return true;
    }
  return false;  
  }

/**********************************************************************/
/* Salva la configuracion general en formato json                     */
/**********************************************************************/  
String cacharroClass::generaJsonConfiguracionNivelActivo(String configActual, int nivelAct)
  {
  String salida="";

  if(configActual=="")  
    {
    Serial.println("No existe el fichero. Se genera uno nuevo");
    return "{\"nombre_dispositivo\": \"Nombre dispositivo\", \"NivelActivo\": \"" + String(nivelAct) + "\"}";
    }
    
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());
  json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");          

//******************************Parte especifica del json a modificar*****************************
    json["NivelActivo"]=nivelAct;
//************************************************************************************************

    json.printTo(salida);//pinto el json que he creado
    Serial.printf("json creado:\n#%s#\n",salida.c_str());
    }//la de parsear el json

  return salida;  
  }  


/********************************** Funciones de LED **************************************/
void cacharroClass::configuraLed(void){pinMode(LED_BUILTIN, OUTPUT);}
void cacharroClass::enciendeLed(void){digitalWrite(LED_BUILTIN, ENCENDIDO);}
void cacharroClass::apagaLed(void){digitalWrite(LED_BUILTIN, APAGADO);}
void cacharroClass::parpadeaLed(uint8_t veces, uint16_t delayed)
  {
  for(uint8_t i=0;i<2*veces;i++)
    {  
    delay(delayed);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
void cacharroClass::parpadeaLed(uint8_t veces)
  {
  parpadeaLed(veces,100);
  }
/********************************** Funciones de LED **************************************/


cacharroClass cacharro;