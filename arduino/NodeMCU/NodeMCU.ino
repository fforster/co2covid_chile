/***************************************************************************
CO2 SENSOR AND TRANSMISSIÓN SYSTEM
Pablo Martín, Francisco Förster
NODEMCU V4
***************************************************************************/
//Librerías
//-----------------------------------------------------------------
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "LoRa_E32.h"
//#include <Wire.h>
//#include <SoftwareSerial.h>
#include <TM1637TinyDisplay.h>
#include <PubSubClient.h>
extern "C" {
#include "libb64/cdecode.h"
}
#include "NodeMCU_config.h"
#include "aws_credentials.h"

//LoRa E32TTL100
//-----------------------------------------------------------------
//Creación de instancia y comunicación serial de Radio LoRa para ESP8266
LoRa_E32 e32ttl100(D2, D3, D5, D7, D6);

// WIFI
//-----------------------------------------------------------------
WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 

//Display de 7 segmentos:
//-----------------------------------------------------------------
const byte CLK = 12;   // define CLK pin (any digital pin)
const byte DIO = 13;   // define DIO pin (any digital pin)
TM1637TinyDisplay display(CLK, DIO);


//Buzzer
//-----------------------------------------------------------------
byte buzzerPin = 7;

//Valores fijos en la rutina
//-----------------------------------------------------------------
int pauseBetweenLoops = 15; //en seg
int ppmAlarma = 800; //Nivel de alarma por nivel de CO2

//Variables auxiliares
//-----------------------------------------------------------------
unsigned long lastPublish;
int msgCount;

//-----------------------------------------------------------------
//Setup ***********************************************************
//-----------------------------------------------------------------

void setup() {
  
  //Inicialización de comunicaciones seriales:
  Serial.begin(9600);  //Para debuggear
  e32ttl100.begin(); //Comunicación LoRa
  delay(1000);
  
  //Saludo en pantalla:
  display.setBrightness(BRIGHT_7);
  display.showString("HOLA");

  //Wifi
  display.showString("WIFI");
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());
  display.showString("DONE");
  
  //Obtener marca de tiempo actual, de lo contrario etiquetar como "expirado"
  display.showString("TIME");
  setCurrentTime();
  display.showString("DONE");

  //Certificación AWS
  display.showString("AWS");
  uint8_t binaryCert[certificatePemCrt.length() * 3 / 4];
  int len = b64decode(certificatePemCrt, binaryCert);
  wiFiClient.setCertificate(binaryCert, len);
  
  uint8_t binaryPrivate[privatePemKey.length() * 3 / 4];
  len = b64decode(privatePemKey, binaryPrivate);
  wiFiClient.setPrivateKey(binaryPrivate, len);

  uint8_t binaryCA[caPemCrt.length() * 3 / 4];
  len = b64decode(caPemCrt, binaryCA);
  wiFiClient.setCACert(binaryCA, len);
  display.showString("DONE");
  delay(1000);
  
  //LoRa:
  display.showString("LORA");
  ResponseStructContainer c;
  c = e32ttl100.getConfiguration(); //Obtener configuración actual
  delay(10);
  Configuration configuration = *(Configuration*) c.data; //Info de la configuración para editar y luego reeemplazar en la radio
  //Display de info Lora
  Serial.println("CÓDIGO NODE MCU");
  Serial.println("*******************************************");
  printParameters(configuration);
  Serial.println("*******************************************");

// ******************************************************************************
//  ESTE CÓDIGO SOLO SE CARGA PARA CONFIGURAR (LA PRIMERA VEZ),
//  Luego se re compila el programa con esto comentado.
//  ---------------------------------
  e32ttl100.setMode(MODE_3_SLEEP);
  delay(10);
  configuration.ADDL = 0x07;
  configuration.ADDH = 0x07;
  configuration.CHAN = 0x17;
  configuration.OPTION.fec = FEC_1_ON;
  configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
  configuration.OPTION.transmissionPower = POWER_20;
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;
  //Set configuration changed and set to not hold the configuration
  ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);
// ******************************************************************************

  c.close(); //Cierra el struct container.  
  Serial.println("ELIGIENDO MODO NORMAL DE TRANSMISIÓN...");
  e32ttl100.setMode(MODE_0_NORMAL);
  delay(10);
  Serial.println("CONFIGURACIÓN LISTA GATEWAY MCU...");

  display.showString("DONE");
  delay(500);
  display.showString("INIT");
  delay(1000);
  
}//setup

//-----------------------------------------------------------------
//Loop  ***********************************************************
//-----------------------------------------------------------------
void loop() {
  for (int i = 0; i == numeroSensores; i++) {
    String nodoIdAux = idRoot;
    if(i<10){
      nodoIdAux = nodoIdAux +"0"+ String(i);
      }
    else{
      nodoIdAux = nodoIdAux + String(i);
      }
      consultarA(nodoIdAux);
    }
  delay(pauseBetweenLoops);
}

//-----------------------------------------------------------------
//Funciones *******************************************************
//-----------------------------------------------------------------
void consultarA(String nodo){
 Serial.println("Enviando mensaje a: " + nodo);
  ResponseStatus rs = e32ttl100.sendMessage(nodo);
  //If something available
  delay(5000);
  if(e32ttl100.available()  > 1 ){ 
    ResponseContainer rc = e32ttl100.receiveMessage();
    Serial.println(nodo + ": " + rc.data);
  
    //Conexión para envío a AWS:
    pubSubCheckConnect();
  
    //Envío a  AWS:
    if (millis() - lastPublish > 10000) {
      int device_id = 1;
      String msg = rc.data;
      if ((msg.substring(msg.length()-1, msg.length())).equals("}")) {
          String topic = String("device/") + device_id + String("/data");
          Serial.println(topic);
          pubSubClient.publish(topic.c_str(), rc.data.c_str());
          Serial.print("Published: "); Serial.println(rc.data.c_str());
          lastPublish = millis();
       }
    }
  }//if e32 dispoinble
  else{;}
}//void consultarA

void printParameters(struct Configuration configuration) {
  Serial.println("----------------------------------------");
  Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN);Serial.print(" ");Serial.print(configuration.HEAD, DEC);Serial.print(" ");Serial.println(configuration.HEAD, HEX);
  Serial.println(F(" "));
  Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, BIN);
  Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, BIN);
  Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
  Serial.println(F(" "));
  Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
  Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
  Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

  Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
  Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
  Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
  Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
  Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

  Serial.println("----------------------------------------");
}

void msgReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
  display.showString("HOLA");
  delay(1000);
  display.showString("");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void pubSubCheckConnect() {
  if ( ! pubSubClient.connected()) {
    display.showString("PUBSUB");
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("ESPthing");
    }
    Serial.println(" connected");
    pubSubClient.subscribe("inTopic");
    display.showString("DONE");
  }
  pubSubClient.loop();
}

int b64decode(String b64Text, uint8_t* output) {
  base64_decodestate s;
  base64_init_decodestate(&s);
  int cnt = base64_decode_block(b64Text.c_str(), b64Text.length(), (char*)output, &s);
  return cnt;
}

void setCurrentTime() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: "); Serial.print(asctime(&timeinfo));
}
