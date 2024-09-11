//ESP32 V
#include <ArduinoOSCETH.h>
#include <ADS1115_WE.h>
#include <Wire.h>
#include <DataTome.h>
#include <Ticker.h>

// Average Lib
DataTomeMvAvg<unsigned, unsigned long> average_1(25);

//ADC converter I2C
#define I2C_ADDRESS_1 0x48
#define I2C_ADDRESS_2 0x49

ADS1115_WE adc_1 = ADS1115_WE(I2C_ADDRESS_1);
ADS1115_WE adc_2 = ADS1115_WE(I2C_ADDRESS_2);

// Average Lib
Ticker Wake_up;
// SD MMC lib
#include "FS.h"
#include "SD_MMC.h"
//debug local
boolean debug_juju = false;
static bool eth_connected = false;
//SD constants
int clk = 36;
int cmd = 35;
int d0 = 37;
int d1 = 38;
int d2 = 33;
int d3 = 39;

// Global Variables

// Ethernet stuff
IPAddress ip(192, 168, 1, 201);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);
uint8_t mac[] = { 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45 };

// for ArduinoOSC
const char* host = "192.168.1.47";
int publish_port = 7001;

// send / receive varibales
int i;

//Config file
String config_array[100];
String path;
String path1;
String path2;
String path3;
String path4;
String path5;
String path6;
String path7;
String path8;
int gain8 = 0;
float msg = 0.0;
float msg1 = 0.0;
float msg2 = 0.0;
float msg3 = 0.0;
float msg4 = 0.0;
float msg5 = 0.0;
float msg6 = 0.0;
float msg7 = 0.0;
float msg8 = 0.0;
int refresh = 500;
int nb_ech = 50;
int threshold = 200;
float etal;
float humidity;
long value_1;
boolean first_etal = true;

float msg1A = 0.0;
float msg2A = 0.0;
float msg3A = 0.0;
float msg4A = 0.0;
float msg5A = 0.0;
float msg6A = 0.0;

IPAddress stringToIPAddress(String ipAddressString) {
  // Split the string into four parts using '.' as delimiter
  int parts[4];
  int partIndex = 0;
  int strIndex = 0;
  while (partIndex < 4 && strIndex < ipAddressString.length()) {
    int delimiterIndex = ipAddressString.indexOf('.', strIndex);
    if (delimiterIndex == -1) {
      delimiterIndex = ipAddressString.length();
    }
    parts[partIndex++] = ipAddressString.substring(strIndex, delimiterIndex).toInt();
    strIndex = delimiterIndex + 1;
  }

  // Create an IPAddress object
  if (partIndex == 4) {
    return IPAddress(parts[0], parts[1], parts[2], parts[3]);
  } else {
    return IPAddress(0, 0, 0, 0);
  }
}

void readConfigFile(fs::FS& fs, const char* path) {
  Serial.printf("Reading Config file: %s\n", path);
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  int i = 0;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    config_array[i] = line.substring(line.indexOf(":") + 1);
    config_array[i].trim();
    //Serial.println(line);
    i++;
  }
  ip = stringToIPAddress(config_array[0]);
  gateway = stringToIPAddress(config_array[1]);
  subnet = stringToIPAddress(config_array[2]);

  host = config_array[3].c_str();
  publish_port = config_array[4].toInt();
  refresh = config_array[5].toInt();
  path1 = config_array[6];
  path2 = config_array[7];
  path3 = config_array[8];
  path4 = config_array[9];
  path5 = config_array[10];
  path6 = config_array[11];
  path7 = config_array[12];
  gain8 = config_array[13].toInt();
  msg1 = config_array[14].toFloat();
  msg2 = config_array[15].toFloat();
  msg3 = config_array[16].toFloat();
  msg4 = config_array[17].toFloat();
  msg5 = config_array[18].toFloat();
  msg6 = config_array[19].toFloat();
  msg7 = config_array[20].toFloat();
  msg8 = config_array[21].toFloat();
  nb_ech = config_array[23].toInt();
  threshold = config_array[22].toInt();

  Serial.print("Ip : ");
  Serial.println(ip);
  Serial.print("Gateway : ");
  Serial.println(gateway);
  Serial.print("subnet: ");
  Serial.println(subnet);
  Serial.print("Host Target Ip : ");
  Serial.println(host);
  Serial.print("Host Target Port : ");
  Serial.println(publish_port);
  Serial.print("path 1 : ");
  Serial.println(path1);
  Serial.print("path 2 : ");
  Serial.println(path2);
  Serial.print("path 3 : ");
  Serial.println(path3);
  Serial.print("path 4 : ");
  Serial.println(path4);
  Serial.print("path 5 : ");
  Serial.println(path5);
  Serial.print("path 6 : ");
  Serial.println(path6);
  Serial.print("path 7 : ");
  Serial.println(path7);
  Serial.print("gain 8 : ");
  Serial.println(gain8);
  Serial.print("msg 1 : ");
  Serial.println(msg1);
  Serial.print("msg 2 : ");
  Serial.println(msg2);
  Serial.print("msg 3 : ");
  Serial.println(msg3);
  Serial.print("msg 4 : ");
  Serial.println(msg4);
  Serial.print("msg 5 : ");
  Serial.println(msg5);
  Serial.print("msg 6 : ");
  Serial.println(msg6);
  Serial.print("msg 7 : ");
  Serial.println(msg7);
  Serial.print("msg 8 : ");
  Serial.println(msg8);
  Serial.print("refresh frequency in millis : ");
  Serial.println(refresh);
  Serial.print("nombre echantillons : ");
  Serial.println(nb_ech);
  Serial.print("threshold : ");
  Serial.println(threshold);
}
/*
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}*/
void Wake() {
  OscEther.update();
}
void setup() {

  Wire.begin();
  Serial.begin(115200);
  Wake_up.attach_ms(50000, Wake);
  delay(2000);
  //Read config.ini file in SD MMC card



  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }

  readConfigFile(SD_MMC, "/config_windy.ini");
  if (debug_juju) {
    host = "192.168.0.53";
    path1 = "/test";
  }

  SD_MMC.end();
  // Initialize PIEZO sensors
  if (!adc_1.init()) {
    Serial.print("ADS1115 No 1 not connected!");
  } else {
    Serial.print("ADS1115 No 1 is connected!");
  }
  adc_1.setVoltageRange_mV(ADS1115_RANGE_6144);

  if (!adc_2.init()) {
    Serial.print("ADS1115 No 2 not connected!");
  } else {
    Serial.print("ADS1115 No 2 is connected!");
  }
  adc_2.setVoltageRange_mV(ADS1115_RANGE_6144);

  // Ethernet stuff
  // WiFi.onEvent(WiFiEvent);
  ETH.begin();
  ETH.begin(ip, gateway, subnet);
  Serial.print("Host Target Ip : ");
  Serial.println(host);
  Serial.print("Host Target Port : ");
  Serial.println(publish_port);
  Serial.print("path 1 : ");
  Serial.println(path1);

  OscEther.publish(host, publish_port, path1, msg)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path2, msg1A)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path3, msg2A)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path4, msg3A)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path5, msg4A)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path6, msg5A)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path7, msg6A)
    ->setIntervalMsec(refresh);


  //Etallonnage des capteurs
}

void loop() {
  int thresold = 2000;
  //Lecture des ADC
  if (readChannel_1(ADS1115_COMP_0_GND) > thresold) {
    msg1A = msg1;
    path = path2;

    Serial.println("bt 1");
  } else {
    msg1A = 0;
  }

  if (readChannel_1(ADS1115_COMP_1_GND) > thresold) {
    //send msg
    msg2A = msg2;
    path = path3;

    Serial.println("bt 2");
  } else {
    msg2A = 0;
  }

  if (readChannel_1(ADS1115_COMP_2_GND) > thresold) {
    //send msg
    msg3A = msg3;

    Serial.println("bt 3");
  } else {
    msg3A = 0;
  }

  if (readChannel_1(ADS1115_COMP_3_GND) > thresold) {
    //send msg
    msg4A = msg4;

    Serial.println("bt 4");
  } else {
    msg4A = 0;
  }
  if (readChannel_2(ADS1115_COMP_0_GND) > thresold) {
    //send msg
    msg5A = msg5;

    Serial.println("bt 5");
  } else {
    msg5A = 0;
  }

  if (readChannel_2(ADS1115_COMP_1_GND) > thresold) {
    //send msg
    msg6A = msg6;

    Serial.println("bt 6");
  } else {
    msg6A = 0;
  }
  /*
  if (aht20.available() == true)
  {
    humidity = aht20.getHumidity();
    averageHum_1.push(humidity);
    if (first_etal) {
      etal = averageHum_1.get();
      first_etal = false;
    }
    value_1 = averageHum_1.get() - etal ;
    // Serial.println(value_1);

  }
*/
  value_1 = 0;
  for (int i = 0; i < nb_ech; i++) {
    value_1 = value_1 + readChannel_2(ADS1115_COMP_2_GND);
  }
  value_1 = value_1 / nb_ech;
  if (value_1 < threshold) {
    value_1 = 0;
  }
  msg = constrain(map(value_1, threshold, 1000, 0, 100), 0, 100) / 100.0;

  path = path1;
  Serial.println(msg);



  OscEther.post();
}


float readChannel_2(ADS1115_MUX channel) {
  int voltage = 0;
  adc_2.setCompareChannels(channel);
  adc_2.startSingleMeasurement();
  while (adc_2.isBusy()) {}
  voltage = adc_2.getResult_mV();  // alternative: getResult_mV for Millivolt
  return voltage;
}


float readChannel_1(ADS1115_MUX channel) {
  int voltage = 0;
  adc_1.setCompareChannels(channel);
  adc_1.startSingleMeasurement();
  while (adc_1.isBusy()) {}
  voltage = adc_1.getResult_mV();  // alternative: getResult_mV for Millivolt
  return voltage;
}
