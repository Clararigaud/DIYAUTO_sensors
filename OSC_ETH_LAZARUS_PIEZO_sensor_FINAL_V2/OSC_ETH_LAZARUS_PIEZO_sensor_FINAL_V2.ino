//ESP32 V
#include <ArduinoOSCETH.h>
#include <ADS1115_WE.h>
#include <Wire.h>
#include <Ticker.h>

#define I2C_ADDRESS_1 0x48
#define I2C_ADDRESS_2 0x49

ADS1115_WE adc_1 = ADS1115_WE(I2C_ADDRESS_1);
ADS1115_WE adc_2 = ADS1115_WE(I2C_ADDRESS_2);

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

// declenchement du reset
unsigned long previousMillis = 0;
long interval = 9000;
int reset_interval = 1000;

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

//piezo variables

int trigger = 400;
int piezo_value = 0;
float a0, a1, a2, a3, a4;

//Config file
String config_array[100];
String path;
String path1;
String path2;
String path3;
String path4;
String path5;
int gain1 = 0;
int gain2 = 0;
int gain3 = 0;
int gain4 = 0;
int gain5 = 0;
int refresh = 500;

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
  gain1 = config_array[11].toInt();
  gain2 = config_array[12].toInt();
  gain3 = config_array[13].toInt();
  gain4 = config_array[14].toInt();
  gain5 = config_array[15].toInt();
  reset_interval = config_array[16].toInt();
  interval = reset_interval * 60 * 1000;



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
  Serial.println(path5);
  Serial.print("path 5 : ");
  Serial.println(path1);
  Serial.print("refresh frequency in millis : ");
  Serial.println(refresh);
  Serial.print("Interval reset in millis: ");
  Serial.println(interval);
}

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

  readConfigFile(SD_MMC, "/config_piezo.ini");
  if (debug_juju) {
    host = "192.168.0.52";
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

  ETH.begin();
  ETH.begin(ip, gateway, subnet);
  Serial.print("Host Target Ip : ");
  Serial.println(host);
  Serial.print("Host Target Port : ");
  Serial.println(publish_port);
  Serial.print("path 1 : ");
  Serial.println(path1);

  OscEther.publish(host, publish_port, path1, a0)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path2, a1)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path3, a2)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path4, a3)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path5, a4)
    ->setIntervalMsec(refresh);
  // OscEther.publish(host, publish_port, path, piezo_value)
  // ->setIntervalMsec(10);

  const char* host_juju = "192.168.1.13";
  OscEther.publish(host_juju, publish_port, "/coucou", a0)
    ->setIntervalMsec(refresh);
}

void loop() {
  // mise en route du timer reset
  unsigned long currentMillis = millis();
  //Lecture des ADC
  a0 = map(readChannel_1(ADS1115_COMP_0_GND), 0, gain1, 0, 1);
  a1 = map(readChannel_1(ADS1115_COMP_1_GND), 0, gain2, 0, 1);
  a2 = map(readChannel_1(ADS1115_COMP_2_GND), 0, gain3, 0, 1);
  a3 = map(readChannel_1(ADS1115_COMP_3_GND), 0, gain4, 0, 1);
  a4 = map(readChannel_2(ADS1115_COMP_0_GND), 0, gain5, 0, 1);
  if (a0 > 1) {
    a0 = 1;
    previousMillis = currentMillis;
  }
  if (a1 > 1) {
    a1 = 1;
    previousMillis = currentMillis;
  }
  if (a2 > 1) {
    a2 = 1;
    previousMillis = currentMillis;
  }
  if (a3 > 1) {
    a3 = 1;
    previousMillis = currentMillis;
  }
  if (a4 > 1) {
    a4 = 1;
    previousMillis = currentMillis;
  }

  if (a0 < 0) {
    a0 = 0;
  }
  if (a1 < 0) {
    a1 = 0;
  }
  if (a2 < 0) {
    a2 = 0;
  }
  if (a3 < 0) {
    a3 = 0;
  }
  if (a4 < 0) {
    a4 = 0;
  }
  OscEther.post();  // to publish osc
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
