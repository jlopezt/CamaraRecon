/*****************************************/
/*                                       */
/*  Control de salidas                   */
/*                                       */
/*****************************************/

/***************************** Defines *****************************/
/***************************** Defines *****************************/

/***************************** Includes *****************************/
#include <Salidas.h>
#include <salida.h>
#include <Entradas.h>

//#include <MQTT.h>
//#include <Ficheros.h>
/***************************** Includes *****************************/

/**********************************************************Constructor******************************************************************/
Salida::Salida(void){
    //inicializo la parte logica
    nombre="No configurado";
    estado=NO_CONFIGURADO;
    modo=NO_CONFIGURADO;
    modo_inicial=NO_CONFIGURADO;
    pin=-1;
    tipo=TIPO_DIGITAL;
    valorPWM=0;
    canal=-1;
    frecuencia=FRECUENCIA_PWM;
    resolucion=RESOLUCION_PWM;
    anchoPulso=0;
    controlador=NO_CONFIGURADO;
    finPulso=0;
    inicio=0;
    nombreEstados[0]="Nombre0";
    nombreEstados[1]="Nombre1";    
    mensajes[0]="Mensaje0";
    mensajes[1]="Mensaje1";
}
/**********************************************************Fin constructor******************************************************************/    
/**********************************************************Configuracion******************************************************************/    
void Salida::configura(String _nombre, int8_t _tipo, int8_t _pin, int8_t _inicio, int16_t _valorPWM, int16_t _anchoPulso, int8_t _modo, int8_t _canal, int16_t _frecuencia, int8_t _resolucion, int8_t _controlador, String _nombres[2], String _mensajes[2]){
    nombre=_nombre;
    tipo=_tipo;
    pin=_pin;
    inicio=_inicio; //Si de inicio debe estar activado o desactivado
    modo=_modo;
    modo_inicial=_modo;

    anchoPulso=_anchoPulso;
    controlador=_controlador;

    valorPWM=_valorPWM;
    canal=_canal;
    frecuencia=_frecuencia;
    resolucion=_resolucion;   
    
    nombreEstados[0]=_nombres[0];
    nombreEstados[1]=_nombres[1];
    mensajes[0]=_mensajes[0];
    mensajes[1]=_mensajes[1];    

    //estado y finPulso las inicializa el constructor
}
/**********************************************************Fin configuracion******************************************************************/    
/**********************************************************SALIDA******************************************************************/    
int8_t Salida::actualizaPulso(void){
  if(estado!=ESTADO_PULSO) return NO_CONFIGURADO; //No configurado
    
  if(finPulso>anchoPulso){    //el contador de millis no desborda durante el pulso
    if(millis()>=finPulso){   //El pulso ya ha acabado
      conmuta(ESTADO_DESACTIVO);  
      Serial.printf("Fin del pulso. millis()= %li\n",millis());
    }
  }
  else{ //El contador de millis desbordar durante el pulso
    if(UINT64_MAX-anchoPulso>millis()){  //Ya ha desbordado
      if(millis()>=finPulso){            //El pulso ha acabado
        conmuta(ESTADO_DESACTIVO);
        Serial.printf("Fin del pulso. millis()= %li\n",millis());
      }
    }
  }

  return CONFIGURADO;  
}
  
/******************************************************/
/* Evalua el estado de cada salida y la actualiza     */
/* segun el modo de operacion                         */
/* estado=0 o 1 encendido o apagado segun nivelActivo */
/* estado=2 modo secuenciador                         */
/* estado=3 modo seguimiento de entrada (anchoPulso)  */
/******************************************************/
void Salida::actualiza(void){
  switch (modo){
    case MODO_MANUAL://manual
      if(estado==ESTADO_PULSO) actualizaPulso();//si esta en modo pulso
      break;
    case MODO_SECUENCIADOR://Controlada por un secuenciador
      break;
    case MODO_SEGUIMIENTO://Sigue a una entradaseguimiento      
      //Si es un seguidor de pulso
      if(anchoPulso>0){
        if(entradas.getEntrada(controlador).getEstado()==ESTADO_ACTIVO) setPulso(); //Si la entrada esta activa, actualizo
        else if(estado==ESTADO_PULSO) actualizaPulso();//si no esta activa, reviso el pulso
      }
      //Si es un seguidor tal cual
      else if(setSalida(entradas.getEntrada(controlador).getEstado())==-1) Serial.printf("Error al actualizar la salida seguidor %s\n\n",nombre.c_str());
      break;
    case MODO_MAQUINA://Controlada por una maquina de estados  
      break;
    default:  
      break;    
  }
}

/*************************************************/
/*  Devuelve el estado  del rele indicado en id  */
/*  puede ser 0 apagado, 1 encendido, 2 pulsando */
/*************************************************/
int8_t Salida::getEstado(){return estado;}

/********************************************************/
/*  Devuelve el tipo de salida digital/PWM              */
/********************************************************/
int8_t Salida::getTipo(void){return tipo;
}

/********************************************************/
/*  Devuelve el nombre del tipo de salida digital/PWM   */
/********************************************************/
String Salida::getTipoNombre(){
  String cad="";
  switch(tipo){
    case TIPO_DIGITAL:
      cad="Digital";
      break;
    case TIPO_LED:  
      cad="LED";
      break;
    case TIPO_PWM:
      cad="PWM";
      break;
    default:
      cad="Error";
      break;      
  }
    
  return cad;
}

/********************************************************/
/*  Devuelve el valor de PWM,                           */
/*  si la salida es de ese tipo                         */
/********************************************************/
int16_t Salida::getValorPWM(void){  
  if(tipo==TIPO_PWM) return valorPWM;
  else return NO_CONFIGURADO;
}

/********************************************************/
/*  establece el valor de la salida PWM                 */
/********************************************************/
int8_t Salida::setValorPWM(int16_t valor){
  if(tipo==TIPO_PWM){
    uint16_t PWMMax=1;//b0000000000000001
    PWMMax<<= RESOLUCION_PWM;
    if(valor>=PWMMax) valor=PWMMax-1;
    valorPWM=valor;
    return 0;
  }
    
  return NO_CONFIGURADO;
}

/*************************************************/
/*conmuta el rele indicado en id                 */
/*devuelve 1 si ok, -1 si ko                     */
/* LA ENTRADA ES EL ESTADO LOGICO. ADAPTO EL     */
/*  ESTADO FISICO SEGUN nivelActivo              */
/*************************************************/
int8_t Salida::conmuta(int8_t estado_final){
  //parte logica
  estado=estado_final;//Lo que llega es el estado logico. No hace falta mapeo
  
  //parte fisica. Me he hecho un mapa de karnaugh y sale asi
  if(estado_final==cacharro.getNivelActivo()){
    switch (tipo){
      case TIPO_DIGITAL:
        digitalWrite(pin, HIGH); //controlo el rele
        break;
      case TIPO_LED:
        if(cacharro.getNivelActivo()==1) ledcWrite(canal,valorPWM);
        else ledcWrite(canal,(1<<RESOLUCION_PWM)-valorPWM);
        break;
    }
  }
  else{
    switch (tipo){
      case TIPO_DIGITAL:
        digitalWrite(pin, LOW); //controlo el rele
        break;
      case TIPO_LED:
        ledcWrite(canal,0);
        if(cacharro.getNivelActivo()==1) ledcWrite(canal,0);
        else ledcWrite(canal,(1<<RESOLUCION_PWM));
        break;
    }
  }
  
  //Serial.printf("nombre: %s; GPIO: %i; Estado fisico: ",nombre.c_str(),(int)pin);
  //Serial.printf("%i\n",digitalRead(pin));
    
  return 1;
}

/****************************************************/
/*   Genera un pulso en rele indicado en id         */
/*   devuelve 1 si ok, -1 si ko                     */
/****************************************************/
int8_t Salida::setPulso(void){
  //validaciones previas
  if(modo!=MODO_MANUAL && modo!=MODO_SEGUIMIENTO) return NO_CONFIGURADO;
      
  //Pongo el rele en nivel Activo  
  if(!conmuta(ESTADO_ACTIVO)) return 0; //Si no puede retorna -1

  //cargo el campo con el valor definido para el ancho del pulso
  estado=ESTADO_PULSO;//estado EN_PULSO
  finPulso=millis()+anchoPulso; 

  Serial.printf("Incio de pulso %li| fin calculado %li\n",millis(),finPulso);
  
  return 1;  
}

/********************************************************/
/*     Recubre las dos funciones anteriores para        */
/*     actuar sobre un rele                             */
/********************************************************/ 
int8_t Salida::setSalida(int8_t estado)
  {
  //Si esta en modo secuenciador o modo maquina no deberia actuar, solo si esta en modo manual o seguimiento
  if(modo!=MODO_MANUAL && modo!=MODO_SEGUIMIENTO) return -1;
   
  switch(estado)
    {
    case ESTADO_DESACTIVO:
      return conmuta(ESTADO_DESACTIVO);
      break;
    case ESTADO_ACTIVO:
      return conmuta(ESTADO_ACTIVO);
      break;
    case ESTADO_PULSO:
      return setPulso();
      break;      
    default://no deberia pasar nunca!!
      return -1;
    }
  }

/****************************************************/
/*   Genera un pulso en rele indicado en id         */
/*   devuelve 1 si ok, -1 si ko                     */
/****************************************************/
int8_t Salida::salidaMaquinaEstados(int8_t estado)
  {
  //validaciones previas
  if(modo!=MODO_MAQUINA) return NO_CONFIGURADO;

  return conmuta(estado);
  }

/********************************************************/
/*     Asocia la salida a un plan de secuenciador       */
/********************************************************/ 
void Salida::asociarSecuenciador(int8_t plan)
  {
    modo=MODO_SECUENCIADOR;
    controlador=plan; 
  }  

/********************************************************/
/*     Fuerza el modo manual en una salida que esta en  */
/*     en otro modo                                     */
/********************************************************/ 
void Salida::setModoManual(void) {modo=MODO_MANUAL;}

/********************************************************/
/*     Fuerza el modo manual en una salida que esta en  */
/*     en otro modo                                     */
/********************************************************/ 
void Salida::setModoInicial(void)
  {
  modo=modo_inicial;
  conmuta(ESTADO_DESACTIVO);
  }  

/********************************************************/
/*     Devuelve el estado incial de la salida           */
/********************************************************/ 
int8_t Salida::getEstadoInicial() {return inicio;}  

/********************************************************/
/*     Devuelve el nombre de la salida                  */
/********************************************************/ 
String Salida::getNombre() {return nombre;}   

/********************************************************/
/*     Devuelve el modo de la salida                    */
/********************************************************/ 
uint8_t Salida::getPin() {return pin;}   

/********************************************************/
/*     Devuelve el ancho del pulso de la salida         */
/********************************************************/ 
uint16_t Salida::getAnchoPulso(void) {return anchoPulso;}   

/********************************************************/
/*     Devuelve el fin del pulso de la salida           */
/********************************************************/ 
unsigned long Salida::getFinPulso(void) {return finPulso;}   

/********************************************************/
/*  Devuelve el nombre del estado de una salida         */
/********************************************************/ 
String Salida::getNombreEstado(uint8_t estado)
  {
  if(estado>2) return "ERROR";
       
  if (estado!=ESTADO_DESACTIVO) return nombreEstados[ESTADO_ACTIVO];
  
  return nombreEstados[ESTADO_DESACTIVO];
  }   

/********************************************************/
/*  Devuelve el nombre del estado actual de una salida  */
/********************************************************/ 
String Salida::getNombreEstadoActual(void) {return getNombreEstado(estado);}

/********************************************************/
/*  Devuelve el mensaje de una salida en un estado      */
/********************************************************/ 
String Salida::getMensajeEstado(uint8_t estado)
  {
  //validaciones previas
  if(estado>2) return "ERROR";
       
  return mensajes[estado];
  }   

/********************************************************/
/*  Devuelve el mensaje del estado actual una salida    */
/********************************************************/ 
String Salida::getMensajeEstadoActual(void){return getMensajeEstado(estado);}

/********************************************************/
/*     Devuelve el controlador de la salida si esta     */
/*     asociada a un plan de secuenciador               */
/********************************************************/ 
int8_t Salida::getControlador(void)
  {
  //validaciones previas
  if(modo!=MODO_SECUENCIADOR && modo!=MODO_SEGUIMIENTO) return NO_CONFIGURADO;
       
  return controlador;  
  }   

/********************************************************/
/*     Devuelve el modo de la salida                    */
/********************************************************/ 
uint8_t Salida::getModo(void) {return modo;}   

/********************************************************/
/*     Devuelve el nombre del modo de la salida         */
/********************************************************/ 
String Salida::getModoNombre(void)
  {
  String cad="";
  switch(modo)
    {
    case MODO_MANUAL:
      cad="Manual";
      break;
    case MODO_SECUENCIADOR:
      cad="Secuenciador";
      break;
    case MODO_SEGUIMIENTO:
      cad="Seguimiento";
      break;
    case MODO_MAQUINA:
      cad="MaquinaEstados";
      break;    
    default:
      cad="Error";
      break;
    }
  return cad;
  }   

/********************************************************/
/*     Devuelve el modo inicial de la salida            */
/********************************************************/ 
uint8_t Salida::getModoInical(void) {return modo_inicial;}   
/********************************************************** Fin salida ******************************************************************/  


int8_t Salida::getCanal(void){return canal;}
int8_t Salida::getFrecuencia(void){return frecuencia;}
int16_t Salida::getResolucion(void){return resolucion;}