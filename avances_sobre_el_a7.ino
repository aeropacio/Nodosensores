//#include <Arduino.h> // Esta linea incluirla solo si se utiliza Atom o PlatformIo

//#include <NeoSWSerial.h>
#include <SoftwareSerial.h>
#include <String.h>
#include <Wire.h>
#include <LCD.h>

#define rxPin 4 // declaro pin 4 como RX del puerto Serial SOFT
#define txPin 3 // declaro pin 3 como TX del puerto Serial SOFT

//NeoSWSerial SerialAT(4, 3);
SoftwareSerial SerialAT(4, 3);
//********************* DECLARACION DE VARIABLES ****************************//
//***************************************************************************//

char phone_no[]="1538247530";
//Contenido del sms que enviamos. \x1A corresponde al caracter de finalizacion
char sms[] = "Mensaje enviado desde Finix SensorNode#1! \x1A \r\n";


int respuesta;
char aux_str[50];
//Contenido del sms que enviamos. \x1A corresponde al caracter de finalizacion
//Contenido de la dirección Http .En ella indicamos el Host, en este caso
// www.xxxx .net, y la página a la que queremos acceder: /prueba-http/ dentro del Host.
//Es decir sería lo equivalente a acceder a www.xxxx.net/prueba-http/.
char direccion[] = "GET /prueba-http/ HTTP/1.1\r\nHost: www.prometec.net\r\nConnection: close\r\n\r\n";
char direccion2[] = "GET /hola.php/ HTTP/1.1\r\nHost: finix-iot.ddns.net\r\nConnection: close\r\n\r\n";


String data = "serie=777&temp=77"; 
String url = "http://192.168.9.130/entrada_datos.php";
const char* host = "finix-iot.ddns.net";                //direccion ip del host
unsigned int tamano=0;

//********************** DECLARACION DE FUNCIONES ***************************//
//***************************************************************************//


void enviarAT(char* ATcommand, char* resp_correcta)
{
  int x = 0,cont_reintentos=0;
  bool out=0,flag_reintentos=1;
  char respuesta[100];
  unsigned long anterior;

while(flag_reintentos){
 

  memset(respuesta, '\0', 100); // Inicializa el string
  delay(100);


      while ( SerialAT.available() > 0){
        SerialAT.read();        // Limpia el buffer de entrada
      }
      
      anterior = millis();
      SerialAT.println(ATcommand);              // Envia el comando AT al modulo
    
    while(out==0){

        if (SerialAT.available() != 0){            // si hay datos el buffer de entrada del UART lee y comprueba la respuesta
                  
            if (x < 99) {                       //Comprueba que no haya desbordamiento en la capacidad del buffer
                respuesta[x] = SerialAT.read();
                x++;
            }
            else{                              // si hay desborde reinicio el a7 y el arduino
              reiniciar();
            }
         
        }

        if((millis() - anterior) > 30000){      //tiempo maximo de respuesta es 30segundos
            out=1;
            cont_reintentos++;                           //estado=2 quiere decir que se paso el tiempo de espera  SE REINICIA TODO EL COMANDO
            delay(3000);
        }


////////////////////////////////////////////ANALISIS DE LA RESPUESTA DENTRO DEL BUCLE////////////////////////////////////////////////////////////
        if (strstr(respuesta, resp_correcta) != NULL){       // Analizo si coincide la respuesta
            flag_reintentos=0;
            out=1; 
        }
        else if(strstr(respuesta,"COMMAND NO RESPONSE")!= NULL || strstr(respuesta,"+CME ERROR") != NULL){         // si la respuesta es alguna de estas vuelvo a reintentar
            out=1;
            cont_reintentos++;                        //cuanto los intentos 
            delay(3000);                              //  y vuelvo a reintentar
        }
        else{

          Serial.print("El arduino no es capaz de interpretar la siguiente respuesta:");
          Serial.print(respuesta);
          while(1);
        }
        
    }

    if(cont_reintentos > 10){                   //si se reintento 10 veces y sigue fallando tengo que reiniciar el arduino y el modulo
      reiniciar();
    }

}
     

}




bool consulta_AT(char* ATcommand, char* resp_correcta)
{

  int correcto = 0,x = 0;
  bool out=0;
  char respuesta[100];
  unsigned long anterior;

 

  memset(respuesta, '\0', 100); // Inicializa el string
  delay(100);
    anterior = millis();

      while ( SerialAT.available() > 0) SerialAT.read();        // Limpia el buffer de entrada
      SerialAT.println(ATcommand);                            // Envia el comando AT
      x = 0;

    
    while(out==0){

      if (SerialAT.available() != 0)                          // si hay datos el buffer de entrada del UART lee y comprueba la respuesta
        {          
            if (x < 99) {                                   //Comprueba que no haya desbordamiento en la capacidad del buffer
                respuesta[x] = SerialAT.read();
                x++;
            }
            else{
              out=1;
              correcto=0;
            }
         
        }

        if((millis() - anterior) > 3000){       //ESPERO MIENTRAS PASA EL TIMEOUT
            out=1;
        }
    }
            if (strstr(respuesta, resp_correcta) != NULL){       // SI LA RESPUESTA ES CORRECTA
               correcto = 1;
               out=1;//  SALE AUTOMATICAMENTE,NO REINTENTA   
            }
            else{ 
               correcto=0;                                                              
            }
     
  return correcto;                        //correcto=3    mala respuesta
                                          //correcto=2       si hay overflow.
                                          //correcto=1       si es correcta la respuesta
  }









// Void POWER_ON , Funcion para Encender el Modulo A7 pero comprobando que el
// modulo no este encendido (enviando comandos AT)
void power_on()
{
  int respuesta = 0;

  // Comprueba que el modulo A7 esta arrancado
  while(consulta_AT("AT","OK")){

    Serial.print("error en la comunicacion con el modulo gsm, reintentando.");
    }

  
}


// Void POWER_OFF , Funcion para apagar el modulo A7
void power_off()
{
  digitalWrite(9, HIGH);
  delay(1000);
  digitalWrite(9, LOW);
  delay(1000);
}

// Void REINICIAR, Funcion para Reiniciar el Modulo A7 en caso de perder la
// conexion
void reiniciar()
{
  Serial.println("Reiniciando...");
  power_off();
  delay (5000);
  power_on();
  asm volatile ("  jmp 0");                        //intruccion assembler para reiniciar el arduino
}

void iniciar()
{
  Serial.println("Conectando a la red...");
  delay (3000);

  //espera hasta estar conectado a la red movil
  while(consulta_AT("AT+CREG?", "+CREG: 1")){
    Serial.println("error 1");
  }
  
  Serial.println("Conectado a la red. \n");

  enviarAT("AT+CGATT=1\r", "OK"); //Iniciamos la conexión GPRS
  Serial.println("iniciando gprs");
    
  enviarAT("AT+CSTT= \"igprs.claro.com.ar\",\"clarogprs\",\"clarogprs999\"", "OK"); //Definimos el APN, usuario y clave a utilizar
  Serial.println("APN DEFINIDO");
    
  enviarAT("AT+CIICR", "OK"); //Activamos el perfil de datos inalámbrico
  Serial.println("Activado perfil de datos inalambricos 1");
    
  enviarAT("AT+CIFSR", "");   //obtengo direccion ip
  Serial.println("Activado perfil de datos inalambricos 2");
}



void PeticionHttp()
{

  Serial.println("iniciando protocolo TCP con PROMETEC HOST");
  enviarAT("AT+CIPSTART= \"TCP\",\"www.prometec.net\",80", "CONNECT OK"); //Inicia una conexión TCP

  Serial.println("protocolo TCP iniciado");
  
  sprintf(aux_str, "AT+CIPSEND=%d", strlen(direccion));
  enviarAT(aux_str, ">");
  
  Serial.println("enviado el largo del request");
   
  enviarAT(direccion, "OK");
  Serial.println("peticion http exitosa!");
  
}

void Peticion2()
{
  Serial.println("iniciando protocolo TCP con FINIX HOST");
  enviarAT("AT+CIPSTART=\"TCP\",\"finix-iot.ddns.net\",80", "CONNECT OK"); //Inicia una conexión TCP

  Serial.println("protocolo TCP iniciado");
  
  sprintf(aux_str, "AT+CIPSEND=%d", strlen(direccion2));
  enviarAT(aux_str, ">");
   
  Serial.println("enviado el largo del request");
   
  enviarAT(direccion2, "OK");
  Serial.println("peticion http exitosa!");
  
}


void upload_db()
{
  Serial.println("iniciando protocolo TCP con FINIX HOST + MYSQL");
  enviarAT("AT+CIPSTART=\"TCP\",\"finix-iot.ddns.net\",80", "CONNECT OK"); //Inicia una conexión TCP

  Serial.println("protocolo TCP iniciado");

  
String data = "serie=444&temp=17";
String url = "http://192.168.9.130/entrada_datos.php";
String request= String ("POST ") + "http://192.168.9.130/entrada_datos.php" + " HTTP/1.0\r\n" + "Host: " + "finix-iot.ddns.net" + "\r\n" + "Accept: *" + "/" + "*\r\n" + "Content-Length: " + "24" + "\r\n" + "Content-Type: application/x-www-form-urlencoded\r\n" + "\r\n" + "serie=222&temp=17&hum=71";

//tamano = request.length();
tamano = 188;
Serial.println(tamano);

  
  sprintf(aux_str, "AT+CIPSEND=%d", tamano);
  enviarAT(aux_str, ">");
  
  Serial.println("enviado el largo del request");

//  enviarAT(request, "OK");
//  Serial.println("peticion http exitosa!");
}



//******************************** SETUP *************************************//
//***************************************************************************//
void setup()
{
  pinMode(rxPin, INPUT); // Pin 4 como entrada
  pinMode(txPin, OUTPUT); // Pin 3 como Salida
  pinMode(LED_BUILTIN, OUTPUT);
  // Seteo el Baud Rate del Puerto Serial
  SerialAT.begin (38400);
  SerialAT.flush();
  // Seteo el Baud Rate del Puerto NeoSoftSerial
  Serial.begin(115200);
  Serial.flush();
  delay(1000);
  Serial.println("Iniciando puerto SoftSerial");

  power_on();
  iniciar();
}

//********************************** LOOP ***********************************//
//***************************************************************************//
void loop()
{




  if (Serial.available())
    switch (Serial.read())
    {
      case 'h':
      
        PeticionHttp();
        break;
      case 'p':
        Peticion2();
        break;
      case 'r':
        upload_db();
        break;
//       case 'm':
//        mensaje_sms();
//        break;
       case 'x':
       while(consulta_AT("AT","OK") != 1);
       Serial.println("AT OK");
       break;
      case 'q':
        // Closes the socket
        enviarAT("AT+CIPCLOSE", "CLOSE OK"); //Cerramos la conexion
        enviarAT("AT+CIPSHUT", "OK"); //Cierra el contexto PDP del GPRS
        delay(10000);
        break;
    }
  if (SerialAT.available()) {
    Serial.write(SerialAT.read());
  }
}

