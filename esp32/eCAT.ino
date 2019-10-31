/*********
Laser-Art
Oberdorf 2
CH-5636 Benzenschwil 

https://shop.laser-art.ch

*********/


#include <Wire.h>
#include <servo_PCA9685.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#define MAX_SERVOS  16

servo_PCA9685 servo = servo_PCA9685();

#define SERVOMIN  109 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  520 // this is the 'maximum' pulse length count (out of 4096)

const char* ssid = "SSID";
const char* password = "WLAN Password";
const char* host = "*";
long SrvValueOld[12];
AsyncWebServer server(80);

AsyncWebSocket ws("/servos");
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
  } else if(type == WS_EVT_DATA){
    Serial.println("Data received: ");
    String msg = "";
    for(size_t i=0; i < len; i++) {
          msg += (char) data[i];
    }
    Serial.println(msg);
    client->text("Receved:" + msg);
    
    int Srv[12];
    long SrvValue[12];
    float Offset[12];
    String Temp1,Temp2;
    String sParams[12];
    int iCount;
    int i,v;
    // parse only if exists
        float Range = (SERVOMAX-SERVOMIN)/2;
 
        iCount = StringSplit(msg,' ',sParams,16);
        Serial.print("Anzahl Servos");Serial.println(iCount);
        // print the extracted paramters
        for(i=0;i<iCount;i++) {
            Temp1 =sParams[i];
            Temp2 = getValue(Temp1, '=', 0);
            Srv[i] = atoi(Temp2.c_str());
            Temp2 = getValue(Temp1, '=', 1);
            Serial.print(Temp2);Serial.print("  ");Serial.print(Temp1);
            SrvValue[i] =  atoi(Temp2.c_str());
            SrvValue[i]  = map(SrvValue[i], 0, 180, SERVOMIN, SERVOMAX);
            float temp3 =SrvValue[i]-SrvValueOld[Srv[i]];
            Offset[i]=temp3/Range;
            Serial.print("Nr ");Serial.print(Srv[i]);Serial.print(" new  ");Serial.print(SrvValue[i]);Serial.print(" old ");Serial.print(SrvValueOld[Srv[i]]);Serial.print(" Offeset ");Serial.println(Offset[i]);
        }
          int SrvValueStep;
        for(v=0;v<Range;v++) {
            for(i=0;i<iCount;i++) {
              SrvValueStep=(int(Offset[i]*v))+SrvValueOld[Srv[i]];
              //Serial.println(SrvValueStep);
             servo.setPWM(Srv[i], 0,SrvValueStep );
            }
        }
        for(i=0;i<iCount;i++) {
            SrvValueOld[Srv[i]]=SrvValue[i];
        }
        delay(100);
         client->text("Servo moved to:" + msg);
         Serial.println("Servo moved");
    



   } 
  
}




String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}




//Super Split function
int StringSplit(String sInput, char cDelim, String sParams[], int iMaxParams)
{
    int iParamCount = 0;
    int iPosDelim, iPosStart = 0;

    do {
        // Searching the delimiter using indexOf()
        iPosDelim = sInput.indexOf(cDelim,iPosStart);
        if (iPosDelim > (iPosStart+1)) {
            // Adding a new parameter using substring() 
            sParams[iParamCount] = sInput.substring(iPosStart,iPosDelim);
            iParamCount++;
            // Checking the number of parameters
            if (iParamCount >= iMaxParams) {
                return (iParamCount);
            }
            iPosStart = iPosDelim + 1;
        }
    } while (iPosDelim >= 0);
    if (iParamCount < iMaxParams) {
        // Adding the last parameter as the end of the line
        sParams[iParamCount] = sInput.substring(iPosStart);
        iParamCount++;
    }

    return (iParamCount);
}



void setup(void) {
  Serial.begin(115200);
 int n;
        for(n=0;n<12;n++) {
            SrvValueOld[n]=int((SERVOMAX-SERVOMIN)/2);
        }

  if (!SPIFFS.begin(true)){
    Serial.println("Dateisystem no init");
    return;
  }
  Serial.println("Programm startet");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  Serial.println("");
  Serial.println("WiFi-Verbindung hergestellt.");
  Serial.println("IP-Addresse: ");
  Serial.println(WiFi.localIP());
  
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("CAT1a.jpg");
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("CAT1b.jpg");
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("CAT2.jpg");
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("CAT3a.jpg");
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("CAT3b.jpg");

            
  
//Send Data to browser
  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", "{\"pin36\": 5.2, \"pin39\": 0.322, \"pin5\": 1}");
    Serial.println("Datenanfrage an ESP32...");
  });

//Button Test
  server.on("/action", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html","OK");
    Serial.println("Got Action from ESP32 !");
     int paramsNr = request->params();

      for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);
        Serial.print("Param name: ");
        Serial.println(p->name());
        //servonum=p->name().toInt();
        Serial.print("Param value: ");
        Serial.println(p->value());
       // pulselen=p->value().toInt();
        //pulselen=strtol(p->value(), NULL, 0);
        Serial.println("------");
        gotAction(p->name().toInt(),p->value().toInt());
    }
  });
//Button Test
  server.on("/moveServo", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println(millis());
    request->send(200, "text/html","OK");
    Serial.println(millis());
      delay(10);
    //Serial.println("Got Action from ESP32 !");
    // int paramsNr = request->params();
        AsyncWebParameter* p = request->getParam(0);
        Serial.println(millis());
        gotAction(p->name().toInt(),p->value().toInt());
        Serial.println(millis());
      delay(10);
    
  });


  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  
  server.begin();
  servo.begin();
  Serial.println("Server Online");

  delay(10);
}

void gotAction(uint8_t Nr,uint16_t PulseL) {
 //   servo.setPWM(1, 0, 400);

 uint16_t y = map(PulseL, 0, 180, SERVOMIN, SERVOMAX);
  servo.setPWM(Nr, 0, y);
            //Serial.println("sERVO START");
            //Serial.print(Nr);
            //Serial.print(" = ");
            //Serial.println(y);
      // servo.setPWM(servonum, 0, pulselen);
    //  servo.setPWM(1, 0, 400);




}





void loop() {

yield();
  
//
}
