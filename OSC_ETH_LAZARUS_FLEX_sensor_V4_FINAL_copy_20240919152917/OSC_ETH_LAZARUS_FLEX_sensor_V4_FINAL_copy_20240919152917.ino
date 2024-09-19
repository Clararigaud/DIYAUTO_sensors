//ESP32 V
#define ARDUINOOSC_DEBUGLOG_ENABLE
#include <ArduinoOSCETH.h>
#include <ADS1115_WE.h>
#include <Wire.h>
#include <DataTome.h>
#include <Ticker.h>

//ADC converter I2C
#define I2C_ADDRESS_1 0x48
#define I2C_ADDRESS_2 0x49
Ticker Wake_up;
ADS1115_WE adc_1 = ADS1115_WE(I2C_ADDRESS_1);
ADS1115_WE adc_2 = ADS1115_WE(I2C_ADDRESS_2);

// Average Lib
DataTomeMvAvg<unsigned, unsigned long> averageFlex_1(50);
DataTomeMvAvg<unsigned, unsigned long> averageFlex_2(50);
DataTomeMvAvg<unsigned, unsigned long> averageFlex_3(50);
DataTomeMvAvg<unsigned, unsigned long> averageFlex_4(50);
DataTomeMvAvg<unsigned, unsigned long> averageFlex_5(50);
DataTomeMvAvg<unsigned, unsigned long> averageFlex_6(50);
DataTomeMvAvg<unsigned, unsigned long> averageFlex_7(50);
DataTomeMvAvg<unsigned, unsigned long> averageFlex_8(50);
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

// declenchement du rest
unsigned long previousMillis = 0;
long interval = 9000;
int reset_interval=1000;

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
String path1;
int gain1 = 0;
int gain2 = 0;
int gain3 = 0;
int gain4 = 0;
int gain5 = 0;
int gain6 = 0;
int gain7 = 0;
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
  gain1 = config_array[7].toInt();
  gain2 = config_array[8].toInt();
  gain3 = config_array[9].toInt();
  gain4 = config_array[10].toInt();
  gain5 = config_array[11].toInt();
  gain6 = config_array[12].toInt();
  gain7 = config_array[13].toInt();
  gain8 = config_array[14].toInt();
  msg1 = config_array[15].toFloat();
  msg2 = config_array[16].toFloat();
  msg3 = config_array[17].toFloat();
  msg4 = config_array[18].toFloat();
  msg5 = config_array[19].toFloat();
  msg6 = config_array[20].toFloat();
  msg7 = config_array[21].toFloat();
  msg8 = config_array[22].toFloat();
  reset_interval = config_array[23].toInt();
  interval = reset_interval*60*1000;

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
  Serial.print("gain 2 : ");
  Serial.println(gain2);
  Serial.print("gain 3 : ");
  Serial.println(gain3);
  Serial.print("gain 4 : ");
  Serial.println(gain4);
  Serial.print("gain 5 : ");
  Serial.println(gain5);
  Serial.print("gain 6 : ");
  Serial.println(gain6);
  Serial.print("gain 7 : ");
  Serial.println(gain7);
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
   Serial.print("Interval reset in millis: ");
  Serial.println(interval);
}
void WiFiEvent(WiFiEvent_t event) {
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

  readConfigFile(SD_MMC, "/config_flex.ini");
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
  //WiFi.onEvent(WiFiEvent);
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
}

void loop() {
// mise en route du timer reset
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= interval) {
  ESP.restart();
}
  //Lecture des ADC
  averageFlex_1.push(readChannel_1(ADS1115_COMP_0_GND));
  double value_1 = averageFlex_1.get() - averageFlex_1[0];

  averageFlex_2.push(readChannel_1(ADS1115_COMP_1_GND));
  float value_2 = averageFlex_2.get() - averageFlex_2[0];

  averageFlex_3.push(readChannel_1(ADS1115_COMP_2_GND));
  float value_3 = averageFlex_3.get() - averageFlex_3[0];

  averageFlex_4.push(readChannel_1(ADS1115_COMP_3_GND));
  double value_4 = averageFlex_4.get() - averageFlex_4[0];

  averageFlex_5.push(readChannel_2(ADS1115_COMP_0_GND));
  double value_5 = averageFlex_5.get() - averageFlex_5[0];

  averageFlex_6.push(readChannel_2(ADS1115_COMP_1_GND));
  float value_6 = averageFlex_6.get() - averageFlex_6[0];

  averageFlex_7.push(readChannel_2(ADS1115_COMP_2_GND));
  float value_7 = averageFlex_7.get() - averageFlex_7[0];

  averageFlex_8.push(readChannel_2(ADS1115_COMP_3_GND));
  double value_8 = averageFlex_8.get() - averageFlex_8[0];

  if (value_1 > gain1 && value_1 < 1000.0) {
    msg = constrain(msg1, 0.0, 1.0);
    previousMillis = currentMillis;
    OscEther.post();
  }
  if (value_2 > gain2 && value_2 < 1000.0) {
    msg = constrain(msg2, 0.0, 1.0);
    previousMillis = currentMillis;
    OscEther.post();
  }
  if (value_3 > gain3 && value_3 < 1000.0) {
    msg = constrain(msg3, 0.0, 1.0);
    previousMillis = currentMillis;
    OscEther.post();
  }
  if (value_4 > gain4 && value_4 < 1000.0) {
    msg = constrain(msg4, 0.0, 1.0);
    previousMillis = currentMillis;
    OscEther.post();
  }

 if (value_5 > gain5 && value_5 < 1000.0) {
    msg = constrain(msg5, 0.0, 1.0);
    previousMillis = currentMillis;
    OscEther.post();
  }
  if (value_6 > gain6 && value_6 < 1000.0) {
    msg = constrain(msg6, 0.0, 1.0);
    previousMillis = currentMillis;
    OscEther.post();
  }
  if (value_7 > gain7 && value_7 < 1000.0) {
    msg = constrain(msg7, 0.0, 1.0);
    previousMillis = currentMillis;
    OscEther.post();
  }
  if (value_8 > gain8 && value_8 < 1000.0) {
    msg = constrain(msg8, 0.0, 1.0);
    previousMillis = currentMillis;
    Serial.println(msg);
    OscEther.post();
  }
/*
  Serial.print(value_1);
  Serial.print(",");
  Serial.print(value_2);
  Serial.print(",");
  Serial.print(value_3);
  Serial.print(",");*/
  
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
