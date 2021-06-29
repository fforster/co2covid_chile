/***************************************************************************
Sensor CO2 con radio LoRa
Pablo Martín y Francisco Forster
***************************************************************************/
//Librerías
#include "Arduino.h"
#include "LoRa_E32.h"
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MHZ19_uart.h>
#include <TM1637TinyDisplay.h>
#include "DHT.h"
#include "Sensor_config.h"
#include <MHZ19_uart.h>

// conexiones
// ------------------------------------------------

#define E32TX 2 // Arduino RX <-- e32 TX
#define E32RX 3 // Arduino TX --> e32 RX
#define E32AUX 5 // E32 aux
#define E32M0 7 // E32 m0
#define E32M1 6 // E32 m1

#define MHZ19TX 8 // Arduino RX <-- MHZ19 TX
#define MHZ19RX 9 // Arduino TX --> MHZ19 RX

#define CLK 12   // define CLK pin (any digital pin)
#define DIO 11   // define DIO pin (any digital pin)

#define DHTPIN 10 // define DHT pin
#define DHTTYPE DHT11   // DHT 11

#define buzzerPin 13 // buzzer pin


// LoRa E32TTL100
// -----------------------------------------------------------------
#include <SoftwareSerial.h>
SoftwareSerial mySerial(E32TX, E32RX); // (no cambiar nombre mySerial
LoRa_E32 e32ttl100(&mySerial, E32AUX, E32M0, E32M1);
ResponseStatus rsb;

SoftwareSerial co2Serial(MHZ19TX, MHZ19RX); //D8: TX del sensor , D9: RX del sensor (confirmar)
MHZ19_uart mhz19;
  

//Tiempos de muestreo y calentamiento NDIR:
//Variables auxiliares NDIR:
unsigned long ppm = 0;
byte temp = 0;


// Buzzer (pasivo)
//-----------------------------------------------------------------
byte auxAlerta = 0;  // Si este valor es 1 por un tiempo x, sonará la alarma.


// Display de 7 segmentos
// -----------------------------------------------------------------
TM1637TinyDisplay display(CLK, DIO);

// sensor temperatura y humedad
DHT dht(DHTPIN, DHTTYPE);

// play tone
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(tone);
  }
}

// buzzer alert
void buzzerwarning() {

  if (activebuzzer) {
    pinMode(buzzerPin, OUTPUT);
    // 1 second music
    for (int i = 0; i < 2; i++) {
      playTone(1275, 50); // g note
      delay(100); 
    }
    pinMode(buzzerPin, INPUT); // this stops the buzzer (to avoid annoying background noise)
  } 
  else { 
      
    for (int j=0; j < 2; j++) {
      digitalWrite(buzzerPin ,LOW); //Setting pin to LOW
      delay(100); //Delaying
      digitalWrite(buzzerPin, HIGH); //Setting pin to high
      delay(100); //Delaying
    }
  }
}

// Setup 
// ******************************************************************
void setup() {



  //Inicialización de comunicaciones seriales:
  Serial.begin(9600);  // para debuggear
  if (mhz19lib) {
    mhz19.begin(MHZ19TX, MHZ19RX);
    if (autocalibration) {
      mhz19.setAutoCalibration(false);
    }
  } else {
    co2Serial.begin(9600); // comunicación sensor CO2 UART
  }
  e32ttl100.begin(); // comunicación LoRa
  dht.begin(); // sensor temperature y humedad
  pinMode(buzzerPin, OUTPUT); // bocina
  digitalWrite(buzzerPin, HIGH);  
  delay(1000);

  buzzerwarning();

  //Saludo en pantalla:
  display.setBrightness(BRIGHT_7);
  display.showString(StringId.c_str());
  
  //LoRa:
  display.showString("LORA");
  ResponseStructContainer c;
  c = e32ttl100.getConfiguration(); //Obtener configuración actual
  delay(10);
  Configuration configuration = *(Configuration*) c.data; //Info de la configuración para editar y luego reeemplazar en la radio
  //Display de info Lora
  Serial.println("CÓDIGO ARDUINO UNO");
  Serial.println("*******************************************");
  printParameters(configuration);
  Serial.println("*******************************************");

  
  // ******************************************************************************
  //  ESTE CÓDIGO SOLO SE CARGA PARA CONFIGURAR (LA PRIMERA VEZ),
  //  Luego se re compila el programa con esto comentado.
  //  ---------------------------------
  if (e32config) {
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
    delay(1000);
    display.showString("DONE");
    delay(1000);
  }
  
  //Función que espera 180 segundos para la preparación del NDIR:
  display.showString("HEAT");
  calentando();
  Serial.println("CONFIGURACIÓN LISTA ARDUINO UNO...");
  display.showString("DONE"); 
  delay(1000);
}


// Loop  
// ***********************************************************
void loop() {
  //FOR I = 1,...,60
  //IF NEWS... DISPLAY(ALARM?), TRANSMIT AND BREAK
  //ELSE DISPLAY
  if (mhz19lib) {
    ppm = mhz19.getPPM(); //readCO2UART();  // add this line
  } else {
    co2Serial.listen();  // add this line
    ppm = readCO2UART();  // add this line
  }
  Serial.println(ppm);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  

  display.showNumber(int(ppm)); 
  if(ppm >= ppmAlerta){
    Serial.println("Alerta");
    buzzerwarning();
  } 
  for (int i = 0; i < 60; i++) {
    Serial.println(i);
    mySerial.listen();
    if (e32ttl100.available() > 1) {
      ResponseContainer rc = e32ttl100.receiveMessage();
      Serial.println(rc.data);
      if (StringId.equals(rc.data)) {
        // this message cannot be very long, it will be reformatted by the gateway
        String msg = StringId + String(": {\"co2\": ") + ppm
            + String(", \"T\": ") + temperature
            + String(", \"H\": ") + humidity + String("}");
        e32ttl100.sendMessage(msg.c_str());
        Serial.println(msg);
        break;
      }
    }
    if (ppm >= ppmAlerta) {
      for (int j = 0; j < 2; j ++) {
        display.showString("");
        delay(200);
        display.showNumber(int(ppm));
        delay(300);
      }
    } else {
      delay(1000);
    }
  }
}

// Calentar MHZ19
void calentando(){
  for(int i=0; i<= tHeat ;i++){
      display.clear();
      int j = tHeat-i;
      display.showNumber(j); 
      delay(1000);
  }
}

// Obtener datos NDIR:
int readCO2UART(){
  byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
  byte response[9]; // for answer

  Serial.println("Sending CO2 request...");
  co2Serial.write(cmd, 9); //request PPM CO2

  // clear the buffer
  memset(response, 0, 9);
  int i = 0;
  while (co2Serial.available() == 0) {
    //Serial.print("Waiting for response ");
    //Serial.print(i);
    //Serial.println(" s");
    if(i==5){Serial.println("NDIR timeout"); break;}
    delay(1000);
    i++;
  }
  if (co2Serial.available() > 0) {
      co2Serial.readBytes(response, 9);
  }
  // print out the response in hexa
//  for (int i = 0; i < 9; i++) {
//    Serial.print(String(response[i], HEX));
//    Serial.print("   ");
//  }
//  Serial.println("");

  // checksum
  //byte check = getCheckSum(response);
  //if (response[8] != check) {
  //  Serial.println("Checksum not OK!");
  //  Serial.print("Received: ");
  //  Serial.println(response[8]);
  //  Serial.print("Should be: ");
  //  Serial.println(check);
  //}
 
  // ppm
  int ppm_uart = 256 * (int)response[2] + response[3];
//  Serial.print("PPM UART: ");
//  Serial.println(ppm_uart);

  // temp
  temp = response[4] - 40;
//  Serial.print("Temperature? ");
//  Serial.println(temp);

  // status
  byte status = response[5];
//  Serial.print("Status? ");
//  Serial.println(status);
//  if (status == 0x40) {
//    Serial.println("Status OK");
//  }
 
  return ppm_uart;
}

// CheckSum
byte getCheckSum(char *packet) {
  byte i;
  unsigned char checksum = 0;
  for (i = 1; i < 8; i++) {
    checksum += packet[i];
  }
  checksum = 0xff - checksum;
  checksum += 1;
  return checksum;
}

// Imprimir parámetros E32
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
