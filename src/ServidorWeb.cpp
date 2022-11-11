/*****************************************************************/
/*                                                               */
/* Serevicios de gestion del servidor Web incorporado al ESP32   */
/*                                                               */
/*****************************************************************/

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
#include <SD_MMC.h>
#include <Entradas.h>
/***************************** Includes *****************************/

//Variables globales
AsyncWebServer serverX(PUERTO_WEBSERVER);
fs::File fichero;
//Cadenas HTML precargadas
String miniCabecera="<html><head></head><body><link rel='stylesheet' type='text/css' href='css.css'>\n";
String miniPie="</body></html>";

//Prototipo de funciones
void handleNombre(AsyncWebServerRequest *request);
void handleMain(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);

void handleEntradas(AsyncWebServerRequest *request);
void handleEstadoEntradas(AsyncWebServerRequest *request);
void handleConfigEntradas(AsyncWebServerRequest *request);

void handleSalidas(AsyncWebServerRequest *request);
void handleEstadoSalidas(AsyncWebServerRequest *request);
void handleConfigSalidas(AsyncWebServerRequest *request);
/*
void handleSecuenciador(AsyncWebServerRequest *request);
void handleDesactivaSecuenciador(AsyncWebServerRequest *request);
void handleActivaSecuenciador(AsyncWebServerRequest *request);
void handleEstadoSecuenciador(AsyncWebServerRequest *request);
void handleConfigSecuenciador(AsyncWebServerRequest *request);
*/
void handleParticiones(AsyncWebServerRequest *request);
void handleSetNextBoot(AsyncWebServerRequest *request);
void handleInfo(AsyncWebServerRequest *request);
void handleRestart(AsyncWebServerRequest *request);

void handleFicheros(AsyncWebServerRequest *request);
void handleListaFicheros(AsyncWebServerRequest *request);
void handleEditaFichero(AsyncWebServerRequest *request);
void handleLeeFichero(AsyncWebServerRequest *request);
void handleBorraFichero(AsyncWebServerRequest *request);
void handleCreaFichero(AsyncWebServerRequest *request);

void handleNotFound(AsyncWebServerRequest *request);
bool handleFileRead(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

String getContentType(String filename);
/*********************************** Inicializacion y configuracion *****************************************************************/
void inicializaWebServer(void)
  {
  /*******Configuracion del Servicio Web***********/  
  //Inicializo los serivcios, decalra las URIs a las que va a responder

  serverX.on("/", HTTP__GET, handleMain); //Responde con la identificacion del modulo
  serverX.on("/root", HTTP__GET, handleRoot); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos

  serverX.on("/estadoEntradas", HTTP__GET, handleEstadoEntradas); //Responde con la identificacion del modulo
  serverX.on("/estadoSalidas", HTTP__GET, handleEstadoSalidas); //Responde con la identificacion del modulo
  serverX.on("/nombre", HTTP__GET, handleNombre); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos
  serverX.on("/salidas", HTTP__GET, handleSalidas); //Servicio de estdo de reles
  serverX.on("/configSalidas", HTTP__GET, handleConfigSalidas); //Servicio de estdo de reles
  serverX.on("/entradas", HTTP__GET, handleEntradas); //Servicio de estdo de reles      
  serverX.on("/configEntradas", HTTP__GET, handleConfigEntradas); //Servicio de estdo de reles
  /*
  serverX.on("/secuenciador", HTTP__GET, handleSecuenciador); //Servicio de estdo de reles
  serverX.on("/configSecuenciador", HTTP__GET, handleConfigSecuenciador); //Servicio de estdo de reles
  serverX.on("/estadoSecuenciador", HTTP__ANY, handleEstadoSecuenciador);  //Serivico de estado del secuenciador
  serverX.on("/activaSecuenciador", HTTP__ANY, handleActivaSecuenciador);  //Servicio para activar el secuenciador
  serverX.on("/desactivaSecuenciador", HTTP__ANY, handleDesactivaSecuenciador);  //Servicio para desactivar el secuenciador
  */
  serverX.on("/ficheros", HTTP__ANY, handleFicheros);  //Devuelve la pagina estatica ficheros.html
  serverX.on("/listaFicheros", HTTP__GET, handleListaFicheros);  //Devuleve la lista de los ficheros
  serverX.on("/editaFichero", HTTP__GET, handleEditaFichero);  //Devuelve la pagina estatica editaFichero.html
  serverX.on("/leeFichero", HTTP__GET, handleLeeFichero);  //Devuelve el contenido del fichero que se pasa como pàrametro nombre
  serverX.on("/creaFichero", HTTP__GET, handleCreaFichero);  //URI de crear fichero
  serverX.on("/borraFichero", HTTP__GET, handleBorraFichero);  //URI de borrar fichero
  
  serverX.on("/particiones", HTTP__GET, handleParticiones);  //URI de test
  serverX.on("/setNextBoot", HTTP__GET, handleSetNextBoot);

  serverX.on("/restart", HTTP__GET, handleRestart);  //URI de test
  serverX.on("/info", HTTP__GET, handleInfo);  //URI de test


  //upload de ficheros
  serverX.on("/upload", HTTP__GET, [](AsyncWebServerRequest *request) {request->redirect("upload.html");});
  serverX.on("/upload", HTTP__POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload);

  serverX.onNotFound(handleNotFound);//pagina no encontrada

  serverX.begin();

  Serial.printf("Servicio web iniciado en puerto %i\n", PUERTO_WEBSERVER);
  }
/*********************************** Inicializacion y configuracion *****************************************************************/

/************************* Gestores de las diferentes URL coniguradas ******************************/
void handleMain(AsyncWebServerRequest *request) {request->redirect("main.html");}//Redirect to our html web page 
void handleRoot(AsyncWebServerRequest *request) {request->redirect("root.html");}//Redirect to our html web page 

/*******************************UTILIDADES*****************************************************/
/*************************************************/
/*                                               */
/*  Servicio de consulta del nombre del          */
/*  dispositivo y devuelve un formato json       */
/*                                               */
/*************************************************/  
void handleNombre(AsyncWebServerRequest *request)
  {
  const size_t capacity = JSON_OBJECT_SIZE(2);
  DynamicJsonBuffer jsonBuffer(capacity);
  
  JsonObject& root = jsonBuffer.createObject();
  root["nombre"] = cacharro.getNombreDispositivo();
  root["version"] = VERSION;
  
  String cad="";
  root.printTo(cad);
  request->send(200,"text/json",cad);
  }

/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las entradas y devuelve un formato json      */
/*                                               */
/*************************************************/  
void handleEstadoEntradas(AsyncWebServerRequest *request)
  {
  String cad=entradas.generaJsonEstado(false);
  
  request->send(200, "text/json", cad); 
  }  

/*************************************************/
/*                                               */
/*  Servicio de consulta de estado de            */
/*  las Salidas , devuelve un formato json       */
/*                                               */
/*************************************************/  
void handleEstadoSalidas(AsyncWebServerRequest *request)
  {
  String cad=salidas.generaJsonEstado(false);
  
  request->send(200, "text/json", cad); 
  }  

/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(AsyncWebServerRequest *request)
  {
  request->redirect("./");
  delay(500);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Pinta la lista de particiones en         */
/*  memoria                                  */ 
/*                                           */
/*********************************************/  
void handleParticiones(AsyncWebServerRequest *request)
  {
  String cad=miniCabecera;

  //cad += "<link rel='stylesheet' type='text/css' href='css.css'>";
  cad += pintaParticionHTML();
  cad += miniPie;

  request->send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*    Cambia la particion de arranque        */
/*                                           */
/*********************************************/  
void handleSetNextBoot(AsyncWebServerRequest *request)
  {
  if(request->hasArg("p")) {setParticionProximoArranque(request->arg("p"));}//si existen esos argumentos

  request->redirect("particiones");
  }
/*******************************FIN UTILIDADES*****************************************************/
/*******************************ACTIVACION/DESACITVACION ENTRADAS/SALIDAS*****************************************************/
/***************************************************/
/*                                                 */
/*  Servicio de consulta de estado de las salidas  */ 
/*  devuelve un formato json                       */
/*                                                 */
/***************************************************/  
void handleConfigSalidas(AsyncWebServerRequest *request)
  {
  String cad=salidas.generaJsonEstado();
  request->send(200, "text/json", cad);
  }
void handleSalidas(AsyncWebServerRequest *request) {request->redirect("salidas.html");}
  
/*****************************************************/
/*                                                   */
/*  Servicio de consulta de estado de las entradas   */
/*  devuelve un formato json                         */
/*                                                   */
/*****************************************************/  
void handleConfigEntradas(AsyncWebServerRequest *request)
  {
  String cad=entradas.generaJsonEstado();
  request->send(200, "text/json", cad); 
  }
void handleEntradas(AsyncWebServerRequest *request) {request->redirect("entradas.html");
}
/*******************************FIN ACTIVACION/DESACITVACION ENTRADAS/SALIDAS*****************************************************/
/****************************SECUENCIADOR******************************************/
/****************************************************************/
/*                                                              */
/*   Servicio de representacion de los datos del secuenciador   */
/*                                                              */
/****************************************************************/
/*
void handleSecuenciador(AsyncWebServerRequest *request){request->redirect("secuenciador.html");}
*/

/*********************************************************/
/*                                                       */
/*  Servicio de informacion delestado del secuenciador   */
/*                                                       */
/*********************************************************/
//estadoSecuenciador
/*
void handleEstadoSecuenciador(AsyncWebServerRequest *request){
    String cad=secuenciador.generaJsonEstado();

    request->send(200, "text/json", cad);
}
*/

/*********************************************************/
/*                                                       */
/*  Servicio de informacion de la configuracion          */
/*  del secuenciador                                     */
/*                                                       */
/*********************************************************/
/*
void handleConfigSecuenciador(AsyncWebServerRequest *request){
     request->send(SPIFFS, "/SecuenciadorConfig.json", "text/json");
}
*/

/*********************************************/
/*                                           */
/*  Servicio para activar el secuenciador    */
/*                                           */
/*********************************************/  
/*
void handleActivaSecuenciador(AsyncWebServerRequest *request){
  secuenciador.activar();
  handleRoot(request);
}
*/

/*********************************************/
/*                                           */
/*  Servicio para desactivar el secuenciador */
/*                                           */
/*********************************************/  
/*
void handleDesactivaSecuenciador(AsyncWebServerRequest *request){
  secuenciador.desactivar();
  handleRoot(request);
}
*/

/****************************FIN SECUENCIADOR******************************************/

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(AsyncWebServerRequest *request)
  {
  String cad="";

  cad+="<link rel='stylesheet' type='text/css' href='css.css'>\n";
  cad+= "<p style=\"color:black;\">";
  cad+= "-----------------Uptime---------------------<BR>";
  char tempcad[20]="";
  sprintf(tempcad,"%lu", (unsigned long)(esp_timer_get_time()/(unsigned long)1000000)); //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años  
  cad += "Uptime: " + String(tempcad) + " segundos"; //la funcion esp_timer_get_time() devuelve el contador de microsegundos desde el arranque. rota cada 292.000 años
  cad += "<BR>-----------------------------------------------<BR>";  

  cad+= "<BR>-----------------info logica-----------------<BR>";
  cad += "Hora actual: " + reloj.getHora(); 
  cad += "<BR>";
  cad += "Fecha actual: " + reloj.getFecha(); 
  cad += "<BR>";
  cad += "IP: " + String(RedWifi.getIP(debugGlobal));
  cad += "<BR>";  
  cad += "nivelActivo: " + String(cacharro.getNivelActivo());
  cad += "<BR>";  
  for(int8_t i=0;i<salidas.getNumSalidas();i++)
    {
    cad += "Rele " + String(i) + " nombre: " + salidas.getSalida(i).getNombre() + "| estado: " + salidas.getSalida(i).getEstado();    
    cad += "<BR>";   
    }
  cad += "-----------------------------------------------<BR>"; 
  
  cad += "<BR>-----------------WiFi info-----------------<BR>";
  cad += "SSID: " + RedWifi.nombreSSID();
  cad += "<BR>";    
  cad += "IP: " + WiFi.localIP().toString();
  cad += "<BR>";    
  cad += "Potencia: " + String(WiFi.RSSI());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";  

  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "IP broker: " + miMQTT.getIPBroker().toString();
  cad += "<BR>";
  cad += "Puerto broker: " + String(miMQTT.getPuertoBroker());
  cad += "<BR>";  
  cad += "Usuario: " + miMQTT.getUsuarioMQTT();
  cad += "<BR>";  
  cad += "Password: " + miMQTT.getPasswordMQTT();
  cad += "<BR>";  
//  cad += "Topic root: " + miMQTT.getTopicRoot();
//  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  

  cad += "<BR>-----------------Hardware info-----------------<BR>";
  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "<BR>";
  cad += "ChipId: " + String(ESP.getChipRevision());
  cad += "<BR>";  
  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "<BR>";  
  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "<BR>";  
     //gets the size of the flash as set by the compiler
  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "<BR>";  
  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
  cad += "</p>";
 
  request->send(200, "text/html", cad);     
  }

/****************************GESTION DE FICHEROS******************************************/
/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*  COPIADO DE ACTUADOR                      */
/*********************************************/  
void handleCreaFichero(AsyncWebServerRequest *request)
  {
  String cad="";
  String nombreFichero="";
  String contenidoFichero="";

  if(request->hasArg("nombre") && request->hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");
    contenidoFichero=request->arg("contenido");

    if(SistemaFicheros.salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) 
	    {
      String cad=SistemaFicheros.directorioFichero(nombreFichero);
      request->redirect("ficheros?dir=" + cad);
      return;
	    }  
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*  COPIADO DE ACTUADOR                      */
/*********************************************/  
void handleBorraFichero(AsyncWebServerRequest *request)
  {
  String nombreFichero="";
  String contenidoFichero="";
  String cad="";

  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=request->arg("nombre");

    if(SistemaFicheros.borraFichero(nombreFichero)) 
      {
      String cad=SistemaFicheros.directorioFichero(nombreFichero);
      request->redirect("ficheros?dir=" + cad);
      return;
      }
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  request->send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*  COPIADO DE ACTUADOR                      */
/*********************************************/
void handleLeeFichero(AsyncWebServerRequest *request){
  if(request->hasArg("nombre") ) {
    String nombreFichero = request->arg("nombre");
    String contentType = getContentType(nombreFichero);

    if(!nombreFichero.startsWith("/"))  nombreFichero = "/" + nombreFichero;

    fichero=SD_MMC.open(nombreFichero,"r");
    if(!fichero) request->send(404,"text/html", "Fichero no encontrado");

    //request->send(fichero,  nombreFichero, contentType,false);
    AsyncWebServerResponse *response=request->beginChunkedResponse(contentType, [&](uint8_t *buffer, size_t maxlen, size_t index)->size_t{
      size_t salida=fichero.read(buffer,maxlen);

      if(!salida) fichero.close();
      return salida;
    });

    request->send(response);
  }
}

/*********************************************/
/*                                           */
/*  Habilita la edicion y borrado del        */
/*  fichero indicado, a traves de una        */
/*  peticion HTTP                            */ 
/*  COPIADO DE ACTUADOR                      */
/*********************************************/ 
void handleEditaFichero(AsyncWebServerRequest *request){
  if(request->hasArg("nombre") ) //si existen esos argumentos
    {
    request->redirect("editaFichero.html?nombre=" + request->arg("nombre"));
    return;
    }
  
  AsyncResponseStream *response = request->beginResponseStream("text/html");   
  response->printf(miniCabecera.c_str());
  response->printf("Falta el argumento <nombre de fichero>"); 
  response->printf(miniPie.c_str());
  request->send(response);     
}

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*  COPIADO DE ACTUADOR                      */
/*********************************************/  
void handleListaFicheros(AsyncWebServerRequest *request)
  {
  String prefix="/";  

  if(request->hasArg("dir")) prefix=request->arg("dir");

  request->send(200,"text/json",SistemaFicheros.listadoFicheros(prefix));
  }

/*  COPIADO DE ACTUADOR                      */
void handleFicheros(AsyncWebServerRequest *request)
  {
  String prefix="/";  

  if(request->hasArg("dir")) prefix=request->arg("dir");

  request->redirect("ficheros.html?dir=" + prefix);
  }
  
/****************************FIN DE GESTION DE FICHEROS******************************************/

/****************************INICIO DE NOT FOUND Y RETORNO DESDE SPIFSS******************************************/
/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound(AsyncWebServerRequest *request)
  {
  if(!handleFileRead(request))
    {
    Serial.printf("No se encontro el fichero en /SD/www\n");  
    String message = miniCabecera;

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
      
    message += miniPie;
    request->send(404, "text/html", message);
    }
  }

/*****************************************************/
/*                                                   */
/*  Determina el contentType de la respuesta en base */
/*  al fichero que se lee y se devuelve              */
/*                                                   */
/*****************************************************/
String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  //if (request->hasArg("download")) return "application/octet-stream";
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

/*****************************************************/
/*                                                   */
/*  Lee un fichero del SDMMC y lo envia como         */
/*  respuest a la request                            */
/*                                                   */
/*****************************************************/
bool handleFileRead(AsyncWebServerRequest *request) 
  {
  String path=request->url();
  Serial.println("handleFileRead: " + path);
  
  if (!path.startsWith("/")) path = "/" + path;
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www

  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  
  if(SistemaFicheros.existeFichero(pathWithGz) || SistemaFicheros.existeFichero(path))
    { // If the file exists, either as a compressed archive, or normal
    if (SistemaFicheros.existeFichero(pathWithGz)) path += ".gz";  // If there's a compressed version available, use the compressed verion

    fs::File fichero=SD_MMC.open(path,"r");
    request->send(fichero, path, contentType,false);
    //request->send(SD_MMC, path, contentType);

    //Serial.printf("\tfichero enviado: %s | contentType: %s\n",path.c_str(),contentType.c_str());
    return true;
    }
  Serial.printf("\tfichero no encontrado en SD: %s\n", path.c_str());   // If the file doesn't exist, return false
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
    newFile = SD_MMC.open(nombre.c_str(), FILE_WRITE);//abro el fichero, si existe lo borra

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
