/*****************************************************************/
/*                                                               */
/* Serevicios de gestion del servidor Web incorporado al ESP32   */
/*                                                               */
/*****************************************************************/
/************************************************************************************************
Servicio                       URL                     Formato entrada Formato salida                                            Comentario                                            Ejemplo peticion              Ejemplo respuesta
Estado de los reles            http://IP/estado        N/A             <id_0>#<nombre_0>#<estado_0>|<id_1>#<nombre_1>#<estado_1> Devuelve el id de cada rele y su estado               http://IP/estado              1#1|2#0
Activa rele                    http://IP/activaRele    id=<id>         <id>#<estado>                                             Activa el rele indicado y devuelve el estado leido    http://IP/activaRele?id=1     1|1
Desactivarele                  http://IP/desactivaRele id=<id>         <id>#<estado>                                             Desactiva el rele indicado y devuelve el estado leido http://IP/desactivaRele?id=0  0|0
Test                           http://IP/test          N/A             HTML                                                      Verifica el estado del Actuadot   
Reinicia el actuador           http://IP/restart
Informacion del Hw del sistema http://IP/info
************************************************************************************************/

/***************************** Defines *****************************/
//Configuracion de los servicios web
#define PUERTO_WEBSERVER  80

#define FONDO     String("#DDDDDD")
#define TEXTO     String("#000000")
#define ACTIVO    String("#FFFF00")
#define DESACTIVO String("#DDDDDD")
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Global.h>
#include <ServidorWeb.h>
//#include <WebServer.h> //#include <ESP8266WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OTA.h>
#include <FS.h>     //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
#include <Entradas.h>
/***************************** Includes *****************************/

//Prototipo de funciones
bool handleFileRead(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

//Variables globales
AsyncWebServer serverX(PUERTO_WEBSERVER);

//Cadenas HTML precargadas
String cabeceraHTML="";

//version de la web propia del cacharro
/*
String pagina_a = "<!DOCTYPE html>\n<html lang=\"es\">\n <head>\n <meta charset=\"UTF-8\">\n <TITLE>Domoticae</TITLE>\n <link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\">\n </HEAD>\n <BODY>\n <table style=\"width:100%;\" cellpadding=\"10\" cellspacing=\"0\">\n  <tr style=\"height:20%; background-color:black\">\n  <th align=\"left\">\n   <span style=\"font-family:verdana;font-size:30px;color:white\">DOMOTI</span><span style=\"font-family:verdana;font-size:30px;color:red\">C</span><span style=\"font-family:verdana;font-size:30px;color:white\">AE - ";
//en medio va el nombre_dispositivo
String pagina_b = "</span> \n </th>\n </tr>\n <tr style=\"height:10%;\">\n <td>";
String enlaces = "<table class=\"tabla\">\n	<tr class=\"modo1\">\n <td><a href=\"./\" target=\"_self\" style=\"text-decoration:none; color: black;\">Home</a></td>\n	<td><a href=\"index.html\" target=\"_self\" style=\"text-decoration:none; color: black;\">Imagen</a></td>\n	<td><a href=\"configEntradas\" target=\"_self\" style=\"text-decoration:none; color: black;\">Entradas</a></td>\n	<td><a href=\"listaFicheros\" target=\"_self\" style=\"text-decoration:none; color: black;\">Ficheros</a></td>\n		<td><a href=\"info\" target=\"_self\" style=\"text-decoration:none; color: black;\">Info</a></td>\n		<td><a href=\"particiones\" target=\"_self\" style=\"text-decoration:none; color: black;\">Particiones</a></td>\n	<td><a href=\"restart\" target=\"_self\" style=\"text-decoration:none; color: black;\">Restart</a></td>\n	</tr>\n</table>\n";
String pagina_c = "</td></tr><TR style=\"height:60%\"><TD>";
//En medio va el cuerpo de la pagina
String pieHTML = "</TD>\n</TR>\n<TR>\n<TD style=\"color:white; background-color:black\"><a href=\"https://domoticae.lopeztola.com\" target=\"_self\" style=\"text-decoration:none; color:white;\">domoticae-2020</a></TD>\n</TR>\n</table>\n</BODY>\n</HTML>";
*/
const char pagina_a[] PROGMEM = "<!DOCTYPE html>\n<html lang=\"es\">\n <head>\n <meta charset=\"UTF-8\">\n <TITLE>Domoticae</TITLE>\n <link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\">\n </HEAD>\n <BODY>\n <table style=\"width:100%;\" cellpadding=\"10\" cellspacing=\"0\">\n  <tr style=\"height:20%; background-color:black\">\n  <th align=\"left\">\n   <span style=\"font-family:verdana;font-size:30px;color:white\">DOMOTI</span><span style=\"font-family:verdana;font-size:30px;color:red\">C</span><span style=\"font-family:verdana;font-size:30px;color:white\">AE - ";
//en medio va el nombre_dispositivo
const char pagina_b[] PROGMEM = "</span> \n </th>\n </tr>\n <tr style=\"height:10%;\">\n <td>";
const char enlaces[] PROGMEM = "<table class=\"tabla\">\n	<tr class=\"modo1\">\n <td><a href=\"./\" target=\"_self\" style=\"text-decoration:none; color: black;\">Home</a></td>\n	<td><a href=\"index.html\" target=\"_self\" style=\"text-decoration:none; color: black;\">Imagen</a></td>\n	<td><a href=\"configEntradas\" target=\"_self\" style=\"text-decoration:none; color: black;\">Entradas</a></td>\n	<td><a href=\"listaFicheros\" target=\"_self\" style=\"text-decoration:none; color: black;\">Ficheros</a></td>\n		<td><a href=\"info\" target=\"_self\" style=\"text-decoration:none; color: black;\">Info</a></td>\n		<td><a href=\"particiones\" target=\"_self\" style=\"text-decoration:none; color: black;\">Particiones</a></td>\n	<td><a href=\"restart\" target=\"_self\" style=\"text-decoration:none; color: black;\">Restart</a></td>\n	</tr>\n</table>\n";
const char pagina_c[] PROGMEM = "</td></tr><TR style=\"height:60%\"><TD>";
//En medio va el cuerpo de la pagina
const char pieHTML[] PROGMEM = "</TD>\n</TR>\n<TR>\n<TD style=\"color:white; background-color:black\"><a href=\"https://domoticae.lopeztola.com\" target=\"_self\" style=\"text-decoration:none; color:white;\">domoticae-2020</a></TD>\n</TR>\n</table>\n</BODY>\n</HTML>";

/*******************************************************/
/*                                                     */ 
/* Invocado desde Loop, gestiona las peticiones web    */
/*                                                     */
/*******************************************************/
/*
void webServer(int debug)
  {
  server.handleClient();  
  }
*/
/************************* Gestores de las diferentes URL coniguradas ******************************/
/*************************************************/
/*                                               */
/*  Pagina principal. informacion de E/S         */
/*  Enlaces a las principales funciones          */
/*                                               */
/*************************************************/  
void handleRoot(AsyncWebServerRequest *request) 
  {
  String cad=cabeceraHTML;
  String orden="";

  //Entradas    
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";    
  for(int8_t i=0;i<MAX_ENTRADAS;i++)
    {    
    if(Entradas.entradaConfigurada(i)==CONFIGURADO) 
      {
      //cad += "<TR><TD>" + nombreEntrada(i) + "-></TD><TD>" + String(nombreEstadoEntrada(i,estadoEntrada(i))) + "</TD></TR>\n";    
      cad += "<TR class=\"modo2\">";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + FONDO + "; width: 100px\">" + Entradas.nombreEntrada(i) + "</TD>";
      cad += "<TD STYLE=\"color: " + TEXTO + "; text-align: center; background-color: " + String((Entradas.estadoEntrada(i)==0?DESACTIVO:ACTIVO)) + "; width: 200px\">" + String(Entradas.estadoEntrada(i)) + "</TD>";
      cad += "</TR>";      
      }
    }
  cad += "</TABLE>\n";
  cad += "<BR>\n";
/////////////////////////

  //Informacion del dispositivo
  cad += "<p style=\"font-size: 12px;color:black;\">" + cacharro.getNombreDispositivo() + " - Version " + String(VERSION) + "</p>";

  cad += pieHTML;
  request->send(200, "text/html", cad);
  }

/*****************************************************/
/*                                                   */
/*  Servicio de consulta de estado de las entradas   */
/*  devuelve un formato json                         */
/*                                                   */
/*****************************************************/  
void handleConfigEntradas(AsyncWebServerRequest *request)
  {
  String cad="";

  //genero la respuesta por defecto
  cad += cabeceraHTML;

  //Estados
  cad += "<TABLE border=\"0\" width=\"80%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Entradas</CAPTION>\n";  

  cad += "<TR>"; 
  cad += "<TH>id</TH>";  
  cad += "<TH>Nombre</TH>";
  cad += "<TH>Configurada</TH>";
  cad += "<TH>Tipo</TH>";
  cad += "<TH>Pin</TH>";
  cad += "<TH>Estado</TH>";
  /*
  cad += "<TH>Estado activo</TH>";
  cad += "<TH>Estado fisico</TH>";
  cad += "<TH>Estado</TH>";
  cad += "<TH>Nombre del estado</TH>";
  cad += "<TH>mensaje</TH>";
  */
  cad += "</TR>"; 

  for(uint8_t entrada=0;entrada<MAX_ENTRADAS;entrada++)
    {
    cad += "<TR class=\"modo2\">"; 
    cad += "<TD>" + String(entrada) + "</TD>";
    cad += "<TD>" + String(Entradas.nombreEntrada(entrada)) + "</TD>";  
    cad += "<TD>" + String(Entradas.entradaConfigurada(entrada)) + "</TD>";
    cad += "<TD>" + String(Entradas.tipoEntrada(entrada)) + "</TD>";
    cad += "<TD>" + String(Entradas.pinEntrada(entrada)) + "</TD>";
    cad += "<TD>" + String(Entradas.estadoEntrada(entrada)) + "</TD>";
    /*
    cad += "<TD>" + String(estadoActivoEntrada(entrada)) + "</TD>";
    cad += "<TD>" + String(estadoFisicoEntrada(entrada)) + "</TD>";    
    cad += "<TD>" + String(estadoEntrada(entrada)) + "</TD>";
    cad += "<TD>" + String(nombreEstadoEntrada(entrada,estadoEntrada(entrada))) + "</TD>";
    cad += "<TD>" + String(mensajeEstadoEntrada(entrada,estadoEntrada(entrada))) + "</TD>";
    */
    cad += "</TR>";     
    }
  cad += "</TABLE>";

  cad += pieHTML;
  request->send(200, "text/HTML", cad); 
  }
  
/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  //cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  
  cad += "Reiniciando...<br>";
  cad += pieHTML;
    
  request->send(200, "text/html", cad);     
  delay(100);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  //cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";

  cad+= "<BR>-----------------Uptime---------------------<BR>";
  char tempcad[20]="";
  sprintf(tempcad,"%" PRIu64, (esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
  cad += "Uptime: " + String(tempcad) + " segundos"; //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años
  cad += "<BR>-----------------------------------------------<BR>";  

  cad+= "<BR>-----------------info logica-----------------<BR>";
  cad += "IP: " + String(RedWifi.getIP(debugGlobal));
  cad += "<BR>";  
  cad += "nivelActivo: " + String(cacharro.getNivelActivo());
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
  
  cad += "<BR>-----------------WiFi info-----------------<BR>";
  cad += "SSID: " + RedWifi.nombreSSID();
  cad += "<BR>";    
  cad += "IP: " + WiFi.localIP().toString();
  cad += "<BR>";    
  cad += "Potencia: " + String(WiFi.RSSI());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";    
/*
  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "IP broker: " + IPBroker.toString();
  cad += "<BR>";
  cad += "Puerto broker: " +   puertoBroker=0;
  cad += "<BR>";  
  cad += "Usuario: " + usuarioMQTT="";
  cad += "<BR>";  
  cad += "Password: " + passwordMQTT="";
  cad += "<BR>";  
  cad += "Topic root: " + topicRoot="";
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
*/
  cad += "<BR>-----------------Hardware info-----------------<BR>";
  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "<BR>";
  cad += "ChipId: " + String(ESP.getChipRevision());
  cad += "<BR>";  
  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "<BR>";  
  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "<BR>";  
  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "<BR>";  
  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";  
  
  cad += pieHTML;
  request->send(200, "text/html", cad);     
  }

/*********************************************/
/*                                           */
/*  Pinta la lista de particiones en         */
/*  memoria                                  */ 
/*                                           */
/*********************************************/  
void handleParticiones(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  //cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";

  cad +=pintaParticionHTML();

  cad += pieHTML;
  request->send(200, "text/html", cad);     
  }

/*********************************************/
/*                                           */
/*    Cambia la particion de arranque        */
/*                                           */
/*********************************************/  
void handleSetNextBoot(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  //cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";

  if(request->hasArg("p")) //si existen esos argumentos
    {
    if(setParticionProximoArranque(request->arg("p"))) handleParticiones(request);  //cad += "EXITO";
    else cad += "FRACASO";
    }

  cad += pieHTML;
  request->send(200, "text/html", cad);     
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*********************************************/  
void handleListaFicheros(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  //cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  cad += "<h2>Lista de ficheros</h2>";

  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(SistemaFicheros.listaFicheros(contenido)) 
    {
    Serial.printf("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad +="<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 10px; text-align: left;}</style>";
    cad += "<TABLE><tr><td>";
    cad += "<TABLE>";
    while(to!=-1)
      {
      fichero=contenido.substring(0, to);//cojo el principio como el fichero
      contenido=contenido.substring(to+1); //la cadena ahora es desde el separador al final del fichero anterior
      to=contenido.indexOf(SEPARADOR); //busco el siguiente separador

      cad += "<TR><TD>" + fichero + "</TD>";           
      cad += "<TD>";
      cad += "<form action=\"manageFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"editar\">";
      cad += "</form>";
      cad += "</TD><TD>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "</form>";
      cad += "</TD></TR>";
      }
    cad += "</TABLE>\n";
    cad += "<BR>";
    cad += "</TD><TD>";
    
    //Para crear un fichero nuevo
    cad += "<h2>Crear un fichero nuevo:</h2>";
    cad += "<table><tr><td>";      
    cad += "<form action=\"creaFichero\" target=\"_self\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvar\">";
    cad += "  </p>";
    cad += "</td></tr></table>";      
    cad += "</td></tr></table>";      
    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  //cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";

  if(request->hasArg("nombre") && request->hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");
    contenidoFichero=request->arg("contenido");

    if(SistemaFicheros.salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) //cad += "Fichero salvado con exito<br>";
      { 
      //cad += "Fichero salvado con exito<br>"; 
      handleListaFicheros(request); 
      return; 
      }      
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleBorraFichero(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

  //cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");

    if(SistemaFicheros.borraFichero(nombreFichero)) //cad += "El fichero " + nombreFichero + " ha sido borrado.\n";
      { 
      //cad += "Fichero salvado con exito<br>"; 
      handleListaFicheros(request); 
      return; 
      }  
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(AsyncWebServerRequest *request)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenido="";
  
  //cad += "<h1>" + cacharro.getNombreDispositivo() + "</h1>";
  
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");

    if(SistemaFicheros.leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";           
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=75 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";      
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Gestion de un fichero a traves de una    */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleManageFichero(AsyncWebServerRequest *request)
  { 
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenido="";
   
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");
    //cad += "<h2>Fichero: " + nombreFichero + "</h2><BR>";  

    if(SistemaFicheros.leeFichero(nombreFichero, contenido))
      {
      cad += "<form id=\"borrarFichero\" action=\"borraFichero\">\n";
      cad += "  <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">\n";
      cad += "</form>\n";

      cad += "<form id=\"salvarFichero\" action=\"creaFichero\" target=\"_self\">";
      cad += "  <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "</form>\n";

      cad += "<div id=\"contenedor\" style=\"width:900px;\">\n";
      cad += "  <p align=\"center\" style=\"margin-top: 0px;font-size: 16px; background-color: #83aec0; background-repeat: repeat-x; color: #FFFFFF; font-family: Trebuchet MS, Arial; text-transform: uppercase;\">Fichero: " + nombreFichero + "(" + contenido.length() + ")</p>\n";
      cad += "  <BR>\n";
      cad += "  <button form=\"salvarFichero\" style=\"float: left;\" type=\"submit\" value=\"Submit\">Salvar</button>\n";
      cad += "  <button form=\"borrarFichero\" style=\"float: right;\" type=\"submit\" value=\"Submit\">Borrar</button>\n";
      cad += "  <BR><BR>\n";
      cad += "  <textarea form=\"salvarFichero\" cols=120 rows=45 name=\"contenido\">" + contenido + "</textarea>\n";
      cad += "</div>\n";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound(AsyncWebServerRequest *request)
  {
  if(handleFileRead(request)) return;

  String message = cabeceraHTML;

  message = "<h1>" + cacharro.getNombreDispositivo() + "<br></h1>";
  message += "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += request->methodToString();
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i=0; i<request->args(); i++)
    {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }

  message += pieHTML;    
  request->send(404, "text/html", message);
  }

/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  //lo inicializo aqui, despues de leer el nombre del dispositivo en la configuracion del cacharro  
  cabeceraHTML = String(FPSTR(pagina_a)) + cacharro.getNombreDispositivo() + FPSTR(pagina_b) + FPSTR(enlaces) + FPSTR(pagina_c);

  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios  
  //decalra las URIs a las que va a responder
  serverX.on("/", HTTP__GET, handleRoot); //Responde con la identificacion del modulo
  serverX.on("/configEntradas", HTTP__GET, handleConfigEntradas); //Servicio de estdo de reles
  
  serverX.on("/restart", HTTP__GET, handleRestart);  //URI de test
  serverX.on("/info", HTTP__GET, handleInfo);  //URI de test

  serverX.on("/particiones", HTTP__GET, handleParticiones);  //URI de test
  serverX.on("/setNextBoot", HTTP__GET, handleSetNextBoot);

  serverX.on("/listaFicheros", HTTP__GET, handleListaFicheros);  //URI de leer fichero
  serverX.on("/creaFichero", HTTP__GET, handleCreaFichero);  //URI de crear fichero
  serverX.on("/borraFichero", HTTP__GET, handleBorraFichero);  //URI de borrar fichero
  serverX.on("/leeFichero", HTTP__GET, handleLeeFichero);  //URI de leer fichero
  serverX.on("/manageFichero", HTTP__GET, handleManageFichero);  //URI de leer fichero

  //upload de ficheros
  serverX.on("/upload", HTTP__GET, [](AsyncWebServerRequest *request) {request->redirect("upload.html");});
  serverX.on("/upload", HTTP__POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload);

  serverX.onNotFound(handleNotFound);//pagina no encontrada

  serverX.begin();

  Serial.printf("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
  }

/**********************************************************************/
String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(AsyncWebServerRequest *request) 
  {
  String path=request->url();
  Serial.println("handleFileRead: " + path);
  
  if (!path.startsWith("/")) path += "/";
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www

  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SistemaFicheros.existeFichero(pathWithGz) || SistemaFicheros.existeFichero(path))
    { // If the file exists, either as a compressed archive, or normal
    if (SistemaFicheros.existeFichero(pathWithGz)) path += ".gz";  // If there's a compressed version available, use the compressed verion

    request->send(SPIFFS, path, contentType);

    //Serial.printf("\tfichero enviado: %s | contentType: %s\n",path.c_str(),contentType.c_str());
    return true;
    }
  Serial.printf("\tfichero no encontrado en SPIFFS: %s\n", path.c_str());   // If the file doesn't exist, return false
  return false;
  }

/*****************************************************/
/*                                                   */
/*  Recive los ficheros en la subida fileUpload      */
/*  https://github.com/CelliesProjects/minimalUploadAuthESP32/blob/master/upload.htm*/
/*  https://github.com/espressif/arduino-esp32/blob/master/libraries/FS/src/FS.h*/
/*                                                   */
/*****************************************************/
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  static File newFile;

  if(!index)
    {
    String nombre=filename;
    if (!nombre.startsWith("/")) nombre = "/" + nombre;  
    Serial.printf("UploadStart: %s\n", nombre.c_str());
    newFile = SPIFFS.open(nombre.c_str(), FILE_WRITE);//abro el fichero, si existe lo borra

    if (!newFile) request->send(200,"text/plain","Error al abrir fichero");
    }

  Serial.printf("Index %u\nlen: %i\n",index,len);
  for(size_t i=0; i<len; i++)
    {
    Serial.write(data[i]);    
    if(!newFile.write(data[i])) request->send(200,"text/plain","Error al guardar en el fichero");
    }

  if(final){
    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
    newFile.close();      
    request->redirect("resultadoUpload.html");
  }
}
/*
void handleFileUpload()
  {
  File fsUploadFile;
  HTTPUpload& upload = server.upload();
  String path;
 
  if(upload.status == UPLOAD_FILE_START)
    {
    path = upload.filename;
    if(!path.startsWith("/")) path = "/"+path;
    if(!path.startsWith("/www")) path = "/www"+path;
      
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
    }
  else if(upload.status == UPLOAD_FILE_WRITE)
    {
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    } 
  else if(upload.status == UPLOAD_FILE_END)
    {
    if(fsUploadFile) 
      {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
      }
    else 
      {
      server.send(500, "text/plain", "500: couldn't create file");
      }
    }
  }
*/