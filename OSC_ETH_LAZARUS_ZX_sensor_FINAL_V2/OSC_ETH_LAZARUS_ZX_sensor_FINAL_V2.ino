
#include <ArduinoOSCETH.h>

//ZX sensor lib

#include <Wire.h>
#include <ZX_Sensor.h>
// SD MMC lib

#include <Ticker.h>
#include "FS.h"
#include "SD_MMC.h"
//SD constants
int clk = 36;
int cmd = 35;
int d0 = 37;
int d1 = 38;
int d2 = 33;
int d3 = 39;


// declenchement du rest
unsigned long previousMillis = 0;
long interval = 9000;
int reset_interval = 1000;
// Constants
const int ZX_ADDR = 0x10;  // ZX Sensor I2C address
const int ZX_ADDR2 = 0x11;
// Global Variables
ZX_Sensor zx_sensor = ZX_Sensor(ZX_ADDR);
ZX_Sensor zx_sensor2 = ZX_Sensor(ZX_ADDR2);
uint8_t x_pos;
uint8_t z_pos;

uint8_t x_pos2;
uint8_t z_pos2;
// Ethernet stuff
IPAddress ip(192, 168, 0, 201);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// for ArduinoOSC
const char* host = "192.168.0.47";
int publish_port = 5005;

// send / receive varibales
int i;
float f_X = 0.42;
float f_Z = 0.42;
float f_X2 = 0.42;
float f_Z2 = 0.42;
//Config file
String config_array[100];
String path1;
String path2;
String path3;
String path4;
int refresh = 500;

Ticker Wake_up;

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

  host = config_array[5].c_str();
  publish_port = config_array[6].toInt();
  ip = stringToIPAddress(config_array[1]);
  gateway = stringToIPAddress(config_array[2]);
  subnet = stringToIPAddress(config_array[3]);
  path1 = config_array[8];
  path2 = config_array[9];
  path3 = config_array[10];
  path4 = config_array[11];
  refresh = config_array[12].toInt();
  reset_interval = config_array[13].toInt();
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
  Serial.println(path4);
  Serial.print("refresh frequency in millis : ");
  Serial.println(refresh);
  Serial.print("Interval reset in millis: ");
  Serial.println(interval);
}

void Wake() {
  f_X2 = random(100) / 100.0;
  f_Z2 = random(100) / 100.0;
  OscEther.post();
}

void setup() {
  Serial.begin(115200);
  //Wake_up.attach_ms(60000, Wake);
  delay(2000);
  //Read config.ini file in SD MMC card
  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  readConfigFile(SD_MMC, "/config_ZX.ini");

  // Ethernet stuff
  ETH.begin();
  ETH.begin(ip, gateway, subnet);

  OscEther.publish(host, publish_port, path1, f_X)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path2, f_Z)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path3, f_X2)
    ->setIntervalMsec(refresh);
  OscEther.publish(host, publish_port, path4, f_Z2)
    ->setIntervalMsec(refresh);
  // Initialize ZX Sensor (configure I2C and read model ID)
  if (zx_sensor.init()) {
    Serial.println("ZX Sensor initialization complete");
  } else {
    Serial.println("Something went wrong during ZX Sensor init!");
  }
  if (zx_sensor2.init()) {
    Serial.println("ZX Sensor 2 initialization complete");
  } else {
    Serial.println("Something went wrong during ZX Sensor 2 init!");
  }
  //SD mount
}

void loop() {
  // mise en route du timer reset
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    ESP.restart();
  }
  if (zx_sensor.positionAvailable()) {
    x_pos = zx_sensor.readX();
    if (x_pos != ZX_ERROR) {
      x_pos = map(x_pos, 0, 240, 0, 100);
      f_X = x_pos / 100.0;
    }
    z_pos = zx_sensor.readZ();
    if (z_pos != ZX_ERROR) {
      z_pos = map(z_pos, 0, 240, 0, 100);
      f_Z = z_pos / 100.0;
    }
    Serial.println("send");
    previousMillis = currentMillis;
    OscEther.post();  // to publish osc
  }

  if (zx_sensor2.positionAvailable()) {
    x_pos2 = zx_sensor2.readX();
    if (x_pos2 != ZX_ERROR) {
      x_pos2 = map(x_pos2, 0, 240, 0, 100);
      f_X2 = x_pos2 / 100.0;
    }
    z_pos2 = zx_sensor2.readZ();
    if (z_pos2 != ZX_ERROR) {
      z_pos2 = map(z_pos2, 0, 240, 0, 100);
      f_Z2 = z_pos2 / 100.0;
    }
    previousMillis = currentMillis;
    Serial.println("send2");
    OscEther.post();  // to publish osc
  }
}
