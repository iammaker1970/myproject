#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFiClient.h> 
#include <Wire.h>

#ifndef STASSID
#define STASSID "kowine2.4G"
#define STAPSK  "0987654321"
#endif

const char* mqtt_server = "192.168.0.34";
const char* ssid = STASSID;
const char* password = STAPSK;

 // 전역 변수
WiFiClient espClient; 
 //IPAddress  server (192,168,0,34) ;   //서버 IP 주소 - http_site

//---------------------------------------------------
PubSubClient client(espClient);
//---------------------------------------------------
//long lastMsg = 0;
//char msg[50];
//int value = 0;
//---------------------------------------------------

 const int MPU=0x68;//MPU6050 I2C주소
 int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ,Temp;
 void get6050(); 
 //MPU6050 stop
 
void setup() {
 
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("RotaryCook4_sensor_2");

  // No authentication by default
  // ArduinoOTA.setPassword("0618");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //---------------------------------------------------
    client.setServer(mqtt_server, 1883);
  //---------------------------------------------------

  //MPU6050 start
  Wire.begin(0,2); //  ESP01모델 사용시 Wire.begin(0,2)
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);//MPU6050 을 동작 대기 모드로 변경
  Wire.endTransmission(true);

}
//---------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266ClientVindi")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//---------------------------------------------------
void loop() {
  ArduinoOTA.handle();
  
  //MPU6050 start
      get6050();//센서값 갱신

     delay(1000); //딜레이함수 사용을 재검토.
  //MPU6050 stop

//---------------------------------------------------

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
//  long now = millis();
//  if (now - lastMsg > 10000) {
//    lastMsg = now;
    
    client.publish("/RotaryCook4Sensor2/XAcc", String(AcX).c_str(), true);
    client.publish("/RotaryCook4Sensor2/YAcc", String(AcY).c_str(), true);
    client.publish("/RotaryCook4Sensor2/ZAcc", String(AcZ).c_str(), true);
    client.publish("/RotaryCook4Sensor2/XGyro", String(GyX).c_str(), true);
    client.publish("/RotaryCook4Sensor2/YGyro", String(GyY).c_str(), true);
    client.publish("/RotaryCook4Sensor2/ZGyro", String(GyZ).c_str(), true);
    client.publish("/RotaryCook4Sensor2/Temp", String(Temp).c_str(), true);  

    //Serial.print("esp32/XAcc");
    //Serial.println(String(AcX).c_str());
//  }
}
//---------------------------------------------------

  //MPU6050 start

 void get6050(){

  Wire.beginTransmission(MPU);//MPU6050 호출
  Wire.write(0x3B);//AcX 레지스터 위치 요청
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);//14byte의 데이터를 요청

  AcX=Wire.read()<<8|Wire.read();//두개의 나뉘어진 바이트를 하나로 이어붙입니다.
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  Tmp=Wire.read()<<8|Wire.read();
  GyX=Wire.read()<<8|Wire.read();
  GyY=Wire.read()<<8|Wire.read();
  GyZ=Wire.read()<<8|Wire.read(); 
  Temp = (Tmp/340.00+36.53);  
}
  //MPU6050 stop
