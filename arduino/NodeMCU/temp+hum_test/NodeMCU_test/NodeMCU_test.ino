/***************************************************************************
CO2 SENSOR AND TRANSMISSIÓN SYSTEM
Pablo Martín, Francisco Förster
NODEMCU V4
***************************************************************************/
//Librerías
//-----------------------------------------------------------------
#include <ESP8266WiFi.h>
#include "LoRa_E32.h"
#include <TM1637TinyDisplay.h>
#include <PubSubClient.h>
extern "C" {
#include "libb64/cdecode.h"
}
#include "NodeMCU_config.h"
#include "aws_credentials.h"
#include "DHT.h" 

#define DHTPIN D7     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11


// WIFI
//-----------------------------------------------------------------
WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 

//Variables auxiliares
//-----------------------------------------------------------------
unsigned long lastPublish;
int msgCount;

DHT dht(DHTPIN, DHTTYPE);

void setup() {

  dht.begin();
  
  //Inicialización de comunicaciones seriales:
  Serial.begin(9600);  //Para debuggear

  //Wifi
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());
  
  //Obtener marca de tiempo actual, de lo contrario etiquetar como "expirado"
  setCurrentTime();

  //Certificación AWS
  uint8_t binaryCert[certificatePemCrt.length() * 3 / 4];
  int len = b64decode(certificatePemCrt, binaryCert);
  wiFiClient.setCertificate(binaryCert, len);
  
  uint8_t binaryPrivate[privatePemKey.length() * 3 / 4];
  len = b64decode(privatePemKey, binaryPrivate);
  wiFiClient.setPrivateKey(binaryPrivate, len);

  uint8_t binaryCA[caPemCrt.length() * 3 / 4];
  len = b64decode(caPemCrt, binaryCA);
  wiFiClient.setCACert(binaryCA, len);
  delay(1000);
  
  
}//setup

//-----------------------------------------------------------------
//Loop  ***********************************************************
//-----------------------------------------------------------------
void loop() {

  // read data
  delay(100);
  float humidity = dht.readHumidity();
  delay(100);
  float temperature = dht.readTemperature();  
  int co2ppm = 400;
  
  // connect  
  pubSubCheckConnect();

  // send message data
  if (millis() - lastPublish > wait) {
    String topic = String("device/") + device_id + String("/data");
    String msg = String("{\"co2\": ") + co2ppm
            + String(", \"temperature\": ") + temperature
            + String(", \"humidity\": ") + humidity + String("}");
    if ((msg.substring(msg.length()-1, msg.length())).equals("}")) {
        Serial.println(topic);
        pubSubClient.publish(topic.c_str(), msg.c_str());
        Serial.print("Published: "); Serial.println(msg.c_str());
        lastPublish = millis();
     }
  }

  // wait time
  delay(1000);
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

int b64decode(String b64Text, uint8_t* output) {
  base64_decodestate s;
  base64_init_decodestate(&s);
  int cnt = base64_decode_block(b64Text.c_str(), b64Text.length(), (char*)output, &s);
  return cnt;
}


void msgReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
  delay(1000);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void pubSubCheckConnect() {
  if ( ! pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("ESPthing");
    }
    Serial.println(" connected");
    pubSubClient.subscribe("inTopic");
  }
  pubSubClient.loop();
}
