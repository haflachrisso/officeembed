#include <Arduino.h>

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
void connectAndSendHum(char humStr[6]);
void connectAndSendTemp(char tempStr[6]);
void tempPjort(char tempStr[6]);
void humPjort(char humStr[6]);
#define DHTPIN 4     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321


// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();
}

void loop() {


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  //Changes the temperature float to string
  char str_temp[6];
  dtostrf(t, 4, 2, str_temp);
  //Changes the humidity float to string
  char str_hum[6];
  dtostrf(h, 4, 2, str_hum);
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(str_hum);
  Serial.print(F("%  Temperature: "));
  Serial.print(str_temp);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  //Sends the data to my servers
  connectAndSendTemp(str_temp);
  connectAndSendHum(str_hum);
  //humPjort(str_hum);
  //tempPjort(str_temp);
  
  //Sends data to Pjort
  // Wait a few seconds between measurements.
  delay(30000);
}

/*
 Web client

*/
#include <SPI.h>
#include <Ethernet.h>

const int controllerId = 1;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//IP Address of my Pi Express server
char server[] = "192.168.1.147";    // name address for Google (using DNS)
char pjortServer[] = "192.168.1.144";

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement


void connectAndSendTemp(char tempStr[6]) {
    Serial.println(tempStr);

    char postTemp[] = "POST /api/tempMeasurement HTTP/1.1";
    char tempJsonData[600];

    //Adds my concatenated json string to the char array above
    int tempSize = snprintf(tempJsonData, 600,"{\"tempCelcius\": %s, \"controllerId\": %d}",tempStr, controllerId);

    Serial.println(tempSize);

    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true) {
          delay(1); // do nothing, no point running without Ethernet hardware
        }
      }
      if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
      }
      // try to configure using IP address instead of DHCP:
      Ethernet.begin(mac, ip, myDns);
    } else {
      Serial.print("  DHCP assigned IP ");
      Serial.println(Ethernet.localIP());
    }
    // give the Ethernet shield a second to initialize:
    delay(1000);
    Serial.print("connecting to ");
    Serial.print(server);
    Serial.println("...");
    
    // if you get a connection, report back via serial:
    if (client.connect(server, 8080)) {
      Serial.print("connected to ");
      Serial.println(client.remoteIP());
      // Make a HTTP request:
      client.println(postTemp);
      client.println("Host: 192.168.1.147");

      client.println("User-Agent: Arduino/1.0");
      client.println("Connection: close");
      client.println("Content-Type: application/json");

      client.print("Content-Length: ");

      client.println(tempSize);

      client.println();

      client.println(tempJsonData);

      Serial.println("Request is: ");

      Serial.println(tempJsonData);
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
    beginMicros = micros();

}

void connectAndSendHum(char humStr[6]) {
    Serial.println(humStr);

    char postHum[] = "POST /api/humMeasurement HTTP/1.1";
    char humJsonData[600];
    int humSize = snprintf(humJsonData, 600,"{\"humPercent\": %s, \"controllerId\": %d}",humStr, controllerId);

    Serial.println(humSize);

    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true) {
          delay(1); // do nothing, no point running without Ethernet hardware
        }
      }
      if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
      }
      // try to configure using IP address instead of DHCP:
      Ethernet.begin(mac, ip, myDns);
    } else {
      Serial.print("  DHCP assigned IP ");
      Serial.println(Ethernet.localIP());
    }
    // give the Ethernet shield a second to initialize:
    delay(1000);
    Serial.print("connecting to ");
    Serial.print(server);
    Serial.println("...");
    
    // if you get a connection, report back via serial:
    if (client.connect(server, 8080)) {
      Serial.print("connected to ");
      Serial.println(client.remoteIP());
      // Make a HTTP request:
      client.println(postHum);
      client.println("Host: 192.168.1.147");

      client.println("User-Agent: Arduino/1.0");
      client.println("Connection: close");
      client.println("Content-Type: application/json");

      client.print("Content-Length: ");

      client.println(humSize);

      client.println();

      client.println(humJsonData);

      Serial.println("Request is: ");

      Serial.println(humJsonData);
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
    beginMicros = micros();
}

void tempPjort(char tempStr[6]) {
    Serial.println(tempStr);

    char postTemp[600]; // = "POST /api/tempMeasurement HTTP/1.1";
    //char tempJsonData[600];

    //Adds my concatenated json string to the char array above
    int tempSize = snprintf(postTemp, 600,"POST /temperatures/?temperature=%s&device_id=3 HTTP/1.1",tempStr);

    Serial.println(tempSize);

    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true) {
          delay(1); // do nothing, no point running without Ethernet hardware
        }
      }
      if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
      }
      // try to configure using IP address instead of DHCP:
      Ethernet.begin(mac, ip, myDns);
    } else {
      Serial.print("  DHCP assigned IP ");
      Serial.println(Ethernet.localIP());
    }
    // give the Ethernet shield a second to initialize:
    delay(1000);
    Serial.print("connecting to ");
    Serial.print(pjortServer);
    Serial.println("...");
    
    // if you get a connection, report back via serial:
    if (client.connect(pjortServer, 8000)) {
      Serial.print("connected to ");
      Serial.println(client.remoteIP());
      // Make a HTTP request:
      client.println(postTemp);
      client.println("Host: 192.168.1.144");
      client.println();
      client.println();

      // Serial.println(tempJsonData);
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
    beginMicros = micros();
}

void humPjort(char humStr[6]) {
    Serial.println(humStr);

    char postHum[600];
    //char tempJsonData[600];

    //Adds my concatenated json string to the char array above
    int tempSize = snprintf(postHum, 600,"POST /humidities/?humidity=%s&device_id=3 HTTP/1.1",humStr);

    Serial.println(tempSize);

    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true) {
          delay(1); // do nothing, no point running without Ethernet hardware
        }
      }
      if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
      }
      // try to configure using IP address instead of DHCP:
      Ethernet.begin(mac, ip, myDns);
    } else {
      Serial.print("  DHCP assigned IP ");
      Serial.println(Ethernet.localIP());
    }
    // give the Ethernet shield a second to initialize:
    delay(1000);
    Serial.print("connecting to ");
    Serial.print(pjortServer);
    Serial.println("...");
    
    // if you get a connection, report back via serial:
    if (client.connect(pjortServer, 8000)) {
      Serial.print("connected to ");
      Serial.println(client.remoteIP());
      // Make a HTTP request:
      client.println(postHum);
      client.println("Host: 192.168.1.144");
      client.println();
      client.println();

      //Serial.println("End ");

      // Serial.println(tempJsonData);
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
    beginMicros = micros();
}