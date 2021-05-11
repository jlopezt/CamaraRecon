/*****************************************/
/*                                       */
/*  Secuenciador de fases de entradas    */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Secuenciador.h>
/***************************** Includes *****************************/

/************************************** Funciones de configuracion ****************************************/
void secuenciadorClass::inicializaSecuenciador()
  {
  //Valor por defecto de las variables
  secuenciadorActivo=false;
  
  for(int8_t i=0;i<MAX_PLANES;i++)
    {
    for(int8_t j=0;j<12;j++) 
      {
      planes[i].rele=NO_CONFIGURADO;
      planes[i].configurado=NO_CONFIGURADO;  
      planes[i].horas[j]=0;
      }
    } 
        
  //leo la configuracion del fichero
  if(!recuperaDatosSecuenciador(debugGlobal)) Serial.println("Configuracion del secuenciador por defecto");
  else
    { 
    //compruebo si la salida asociada a cada plan esta configurada
    for(int8_t i=0;i<MAX_PLANES;i++)
      {
      if(planConfigurado(i)==CONFIGURADO)
        {  
        if (Salidas.getReleConfigurado(planes[i].rele)==NO_CONFIGURADO)
          {
          Serial.printf("La salida asociada al plan %i no esta configurada\n", planes[i].rele);
          planes[i].configurado=NO_CONFIGURADO;
          }
        //Esta bien configurado  
        else Salidas.asociarSecuenciador(planes[i].rele, i); //Asocio el rele al plan
        }
      else Serial.printf("Plan %i no configurado\n",i);        
      }
    }
  }

boolean secuenciadorClass::recuperaDatosSecuenciador(boolean debug)
  {
  String cad="";

  if (debug) Serial.println("Recupero configuracion de archivo...");
  
  if(!SistemaFicherosSD.leeFichero(SECUENCIADOR_CONFIG_FILE_SD, cad)) {
    //if(!SistemaFicheros.leeFichero(SECUENCIADOR_CONFIG_FILE, cad)) {
      //Confgiguracion por defecto
      Serial.printf("No existe fichero de configuracion del secuenciador\n");
      return false;
    //}      
  }
    
  return parseaConfiguracionSecuenciador(cad);
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de los reles                 */
/*********************************************/
boolean secuenciadorClass::parseaConfiguracionSecuenciador(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  json.printTo(Serial);
  if (!json.success()) return false;
        
  Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************  
  secuenciadorActivo=json["estadoInicial"];
  
  JsonArray& Planes = json["Planes"];  

  int8_t max;
  max=(Planes.size()<MAX_PLANES?Planes.size():MAX_PLANES);
  for(int8_t i=0;i<max;i++)
    { 
    //Plan configurado
    planes[i].configurado=CONFIGURADO;
      
    //Salida asociada
    planes[i].rele=Planes[i]["salida"];

    //Intevalos
    JsonArray& Intervalos = json["Planes"][i]["intervalos"];  
    for(int8_t j=0;j<HORAS_EN_DIA;j++) planes[i].horas[j]=Intervalos[j]["valor"];//el valor es un campo de bit. los primeros 12 son los intervalos de 5 min de cada hora
  
    Serial.printf("Plan %i:\nSalida: %i\n", i, planes[i].rele); 
    for(int8_t j=0;j<HORAS_EN_DIA;j++) Serial.printf("hora %02i: valor: %01i\n",j,planes[i].horas[j]);    
    }
//************************************************************************************************
  return true; 
  }
/**********************************************************Fin configuracion******************************************************************/  

/**********************************************************SALIDAS******************************************************************/    
/*************************************************/
/*Logica del secuenciador                        */
/*Comporueba como debe estar en ese peridodo de  */
/*cinco minutos parea esa hora y actualiza el    */
/*rele correspondiente                           */
/*************************************************/
void secuenciadorClass::actualizaSecuenciador(bool debug)
  {
  if(!estadoSecuenciador()) return;
    
  for(int8_t i=0;i<getNumPlanes();i++)
    {
    if(planConfigurado(i))
      {
      int mascara=1;
      int8_t limite=reloj.minuto()/(int)5;
      mascara<<=limite;//calculo la mascara para leer el bit correspondiente al minuto adecuado

      if(debug) Serial.printf("Hora: %02i:%02i\nMascara: %i | intervalo: %i\n",reloj.hora(),reloj.minuto(),mascara,planes[i].horas[reloj.hora()]);
      
      if(planes[i].horas[reloj.hora()] & mascara) Salidas.conmutaRele(planes[i].rele, cacharro.getNivelActivo(), debugGlobal);
      else Salidas.conmutaRele(planes[i].rele, !cacharro.getNivelActivo(), debugGlobal);
      }  
    }
  }

/**************************************************/
/*                                                */
/* Devuelve el nuemro de planes definido          */
/*                                                */
/**************************************************/
int8_t secuenciadorClass::getNumPlanes()
  {
  int resultado=0;
  
  for(int8_t i=0;i<MAX_PLANES;i++)
    {
    if(planes[i].configurado==CONFIGURADO) resultado++;
    }
  return resultado; 
  }  

/********************************************************/
/*                                                      */
/*     Devuelve si el plan esta configurados            */
/*                                                      */
/********************************************************/ 
int secuenciadorClass::planConfigurado(uint8_t id)
  {
  if(id<0 || id>MAX_PLANES) return NO_CONFIGURADO;
    
  return planes[id].configurado;
  }

/********************************************************/
/*                                                      */
/*             Activa el secuenciador                   */
/*                                                      */
/********************************************************/ 
void secuenciadorClass::activarSecuenciador(void)
  {
  secuenciadorActivo=true;
  }

/********************************************************/
/*                                                      */
/*             Desactiva el secuenciador                */
/*                                                      */
/********************************************************/ 
void secuenciadorClass::desactivarSecuenciador(void)
  {
  secuenciadorActivo=false;
  }
  
/********************************************************/
/*                                                      */
/*     Devuelve el estado del secuenciador              */
/*                                                      */
/********************************************************/ 
boolean secuenciadorClass::estadoSecuenciador(void)
  {
  return secuenciadorActivo;
  }  

/********************************************************/
/*                                                      */
/*     Genera codigo HTML para representar el plan      */
/*                                                      */
/********************************************************/ 
String secuenciadorClass::pintaPlanHTML(int8_t plan)
  {
  String cad="";

  //validaciones previas
  if(plan<0 || plan>MAX_PLANES) return cad;

  cad += "<TABLE style=\"border: 2px solid black\">\n";
  cad += "<CAPTION>Plan " + String(plan) + "</CAPTION>\n";  

  //Cabecera
  cad += "<tr>";
  cad += "<th>Hora</th>";
  for(int8_t i=0;i<HORAS_EN_DIA;i++) cad += "<th style=\"width:40px\">" + String(i) + "</th>";
  cad += "</tr>";

  //Cada fila es un intervalo, cada columna un hora
  int mascara=1;  
  
  for(int8_t intervalo=0;intervalo<12;intervalo++)
    {
    Serial.printf("intervalo: %i | cad: %i\n",intervalo,cad.length());      
    cad += "<tr>";
    cad += "<td>" + String(intervalo) + ": (" + String(intervalo*5) + "-" + String(intervalo*5+4) + ")</td>";    
    for(int8_t i=0;i<HORAS_EN_DIA;i++) cad += "<td style=\"text-align:center;\">" + (planes[plan].horas[i] & mascara?String(1):String(0)) + "</td>";
    cad += "</tr>";
    
    mascara<<=1;
    }  
    
  return cad;  
  }

secuenciadorClass Secuenciador;