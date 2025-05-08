#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>
#include "event_io.h"
#include <TimeLib.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#define RX_PIN D5 // Pin RX (GPIO14)
#define TX_PIN D6 // Pin TX (GPIO12)
SoftwareSerial SeArduino(RX_PIN, TX_PIN); // Inisialisasi objek SoftwareSerial
WiFiClient client;
//Wifi Client

String formattedDate;
String nopol;
String host;      // Host sebagai String yang dapat diubah
int port;  
const char* ssid = "AP9811";                //Put wifi ssid within the quotes
const char* password = "Ilovesmartmt123";         //Put WiFi password within the quotesconst 
//const char* ssid = "AP9811";                //Put wifi ssid within the quotes
//const char* password = "Ilovesmartmt123";  
// const char* host = "182.23.69.206";
// const uint16_t port = 4278;

// const char* host = "139.99.69.137";
// const uint16_t port = 3333;
// String nopol = "B9298SEH";
// String serverName = "http://smartmt.patraniaga.com/smartmt/smtServletDataSync.SmtLogSync?logstring=";
String tabletName = "http://192.168.1.3:8078/?logstring=";
String sn = "";
String timestamp = "";
float voltage = 0.0;
float temperature = 0.0;
float humidity = 0.0;
float transducer = 0.0;
float ther_1, ther_2, ther_3, ther_4, ther_5, ther_6, ther_7, ther_8;
int dist_1, dist_2, dist_3, dist_4, dist_5, dist_6, dist_7, dist_8, dist_9, dist_10, dist_11, dist_12;
int distValue_1, distValue_2, distValue_3, distValue_4, distValue_5, distValue_6, distValue_7, distValue_8, distValue_9, distValue_10, distValue_11, distValue_12;
float tpms_1, tpms_2, tpms_3, tpms_4, tpms_5, tpms_6, tpms_7, tpms_8;
int statusFT = 0.0;
int statusSB1 = 0.0;
int statusSB2 = 0.0;
int comp_1, comp_2, comp_3;
String command = "";

int DistanceTreshold    = 200;
int MaxDistanceTreshold = 500;
int DistanceTresholddepan    = 400;
int MaxDistanceTresholddepan = 500;
///* Put IP Address details */

IPAddress local_ip(192, 168, 1, 2);   // IP Address of your ESP8266
IPAddress gateway(192, 168, 1, 1);      // Gateway IP
IPAddress subnet(255, 255, 255, 0);     // Subnet Mask
IPAddress primaryDNS(8, 8, 8, 8);       // Primary DNS (optional)
IPAddress secondaryDNS(8, 8, 4, 4);     // Secondary DNS (optional)

ESP8266WebServer server(80);
HTTPClient http;
HTTPClient http2;
HTTPClient http3;
String macAddress;
String data = "";
String buffer = "";
String dataString = "";
bool SuccessfullLogin = false;
//extern unsigned long currentTimestamp;
unsigned long long decimalValues;

unsigned long long stringToUnsignedLongLong(const char* str) {
    unsigned long long value = 0;
    char* endPtr;
    // Convert string to unsigned long long
    value = strtoull(str, &endPtr, 10);
    // Check if the conversion was successful
    if (*endPtr != '\0') {
        Serial.println("Error: Conversion failed (non-numeric characters found)");
        return 0;
    }
    // Check for range errors
    if (value < 0) {
        Serial.println("Error: Value is negative");
        return 0;
    }
    // Return the converted value
    return value;
}

// Function to convert date string to Unix timestamp
unsigned long convertToUnixTimestamp(const char* dateString) {
  int year = (dateString[0] - '0') * 1000 + (dateString[1] - '0') * 100 + (dateString[2] - '0') * 10 + (dateString[3] - '0');
  int month = (dateString[4] - '0') * 10 + (dateString[5] - '0');
  int day = (dateString[6] - '0') * 10 + (dateString[7] - '0');
  int hour = (dateString[8] - '0') * 10 + (dateString[9] - '0');
  int minute = (dateString[10] - '0') * 10 + (dateString[11] - '0');
  int second = (dateString[12] - '0') * 10 + (dateString[13] - '0');
  tmElements_t tm;
  tm.Year = year - 1970;  // Offset from 1970
  tm.Month = month;
  tm.Day = day;
  tm.Hour = hour;
  tm.Minute = minute;
  tm.Second = second;
  
  return makeTime(tm);
}

void setup() {
  Serial.begin(9600);
  SeArduino.begin(9600);
  //Wifi Client
    EEPROM.begin(512);
  // Load plate number from EEPROM
  char storedPlate[100];
  int i = 0;
  while (i < 100) {
    char c = EEPROM.read(i);
    if (c == '\0') break;
    storedPlate[i++] = c;
  }
  storedPlate[i] = '\0';
  nopol = String(storedPlate);
  
  // Load host dari EEPROM
  int hostLength = EEPROM.read(200);
  char storedHost[hostLength + 1];
  for (int i = 0; i < hostLength; ++i) {
    storedHost[i] = EEPROM.read(201 + i);  // Membaca host dari EEPROM
  }
  storedHost[hostLength] = '\0';
  host = strdup(storedHost);  // Simpan host yang di-load

  // Load port dari EEPROM
  EEPROM.get(300, port);  // Membaca port dari EEPROM

  WiFi.disconnect();
  if (!WiFi.config(local_ip, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  Serial.println(ssid);
  Serial.println(password);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
      Serial.println(ssid);
      Serial.println(password);
  }  
 // delay(40000);
  Serial.print("Alamat IP AP: ");
  Serial.println(local_ip);
  macAddress = WiFi.macAddress();
  macAddress.replace(":","");
  Serial.println("MAC address: ");
  Serial.println(macAddress);


  // Mengakhiri koneksi HTTP
  http.end();
  delay(200);
  Serial.print("Trying to log in to the server :");
  Serial.println(host);
  Serial.print("Trying to log in to the server port :");
  Serial.println(port);
  if(!client.connect(host,port)){
    Serial.println("connection failed");
    
  }
  if(client.connected()){
    String idMac ="0006"+macAddress;    
    SendingHex(idMac);
    Serial.println("Connect bro");
    delay(2000);
  }
  while(client.available()){
    char ch = static_cast<char>(client.read());
    if(ch==0x01){
      Serial.print("Success login to Server :");
      Serial.println(host);
      SuccessfullLogin =true;
   }
  }
  server.on("/", handle_OnConnect);  
  server.on("/admin", SendHTMLAdmin);  
  server.on("/a1off", handle_AdminSignalA1off);
  server.on("/a1on", handle_AdminSignalA1on);
  server.on("/a2off", handle_AdminSignalA2off);
  server.on("/a2on", handle_AdminSignalA2on);
  server.on("/a3off", handle_AdminSignalA3off);
  server.on("/a3on", handle_AdminSignalA3on);
  server.on("/aoff", handle_AdminSignalAoff);
  server.on("/aon", handle_AdminSignalAon);
  
  server.on("/c1on", handle_P1on);
  server.on("/c1off", handle_P1off);
  server.on("/c2on", handle_P2on);
  server.on("/c2off", handle_P2off);
  server.on("/c3on", handle_P3on);
  server.on("/c3off", handle_P3off);
  server.onNotFound(handle_NotFound);

  server.on("/set", HTTP_GET, handle_config);
  server.on("/settingwaktu", HTTP_GET, handle_UpdateWaktu);
  server.on("/update", HTTP_POST, handle_UpdatePlate);
  server.on("/update_hostport", HTTP_POST, handle_UpdateHostPort);

  server.begin();
  Serial.println("HTTP server started");
}
void SendingHex(String input) {
  Serial.print("Original String: ");
  byte byteArray[input.length()/2];
   for (int i = 0, j = 0; i < input.length(); i += 2, j++)
    {
      char hexChars[3] = {input[i], input[i + 1], '\0'};
      String dataString = String(hexChars);
      Serial.print(dataString);
      byteArray[j] =(byte) strtol(dataString.c_str(), NULL, 16);
    }
  Serial.println();
  // Print the hexadecimal array
  Serial.print("HEX DATA: ");
  for (int i = 0; i < input.length()/2; i++) {
    if(byteArray[i]<0x10){
      Serial.print(0);
    }
    Serial.print(byteArray[i], HEX);
    Serial.print(" ");
    //client.write(byteArray[i]);
  }
  // Send the byte array over the client
  client.write(byteArray, sizeof(byteArray));
  Serial.println();
}

void ReLogin()
{
  
  Serial.print("Login to server:");
  Serial.println(host);
  
  if(client.connected()){
    String idMac ="0006"+macAddress;    
    SendingHex(idMac);
  }
  while(client.available()){
    char ch = static_cast<char>(client.read());
    if(ch==0x01){
      Serial.print("Success login to Server :");
      Serial.println(host);
      SuccessfullLogin =true;
   }
  }
}
String splitString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
 
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  } 
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
void getdata(){
String getData = "";
  // Memecah stringData menjadi elemen-elemen sesuai dengan indeksnya
  sn = splitString(data, ';', 1);
  timestamp = splitString(data, ';', 2);
  voltage = splitString(data, ';', 3).toFloat();
  temperature = splitString(data, ';', 4).toFloat();
  humidity = splitString(data, ';', 5).toFloat();
  transducer = splitString(data, ';', 6).toFloat();
  ther_1 = splitString(data, ';', 7).toFloat();
  ther_2 = splitString(data, ';', 8).toFloat();
  ther_3 = splitString(data, ';', 9).toFloat();
  ther_4 = splitString(data, ';', 10).toFloat();
  ther_5 = splitString(data, ';', 11).toFloat();
  ther_6 = splitString(data, ';', 12).toFloat();
  ther_7 = splitString(data, ';', 13).toFloat();
  ther_8 = splitString(data, ';', 14).toFloat();
  dist_1 = splitString(data, ';', 15).toInt();
  dist_2 = splitString(data, ';', 16).toInt();
  dist_3 = splitString(data, ';', 17).toInt();
  dist_4 = splitString(data, ';', 18).toInt();
  dist_5 = splitString(data, ';', 19).toInt();
  dist_6 = splitString(data, ';', 20).toInt();
  dist_7 = splitString(data, ';', 21).toInt();
  dist_8 = splitString(data, ';', 22).toInt();
  dist_9 = splitString(data, ';', 23).toInt();
  dist_10 = splitString(data, ';', 24).toInt();
  dist_11 = splitString(data, ';', 25).toInt();
  dist_12 = splitString(data, ';', 26).toInt();
  tpms_1 = splitString(data, ';', 27).toFloat();
  tpms_2 = splitString(data, ';', 28).toFloat();
  tpms_3 = splitString(data, ';', 29).toFloat();
  tpms_4 = splitString(data, ';', 30).toFloat();
  tpms_5 = splitString(data, ';', 31).toFloat();
  tpms_6 = splitString(data, ';', 32).toFloat();
  tpms_7 = splitString(data, ';', 33).toFloat();
  tpms_8 = splitString(data, ';', 34).toFloat();
  statusFT = splitString(data, ';', 35).toInt();
  comp_1 = splitString(data, ';', 36).toInt();
  comp_2 = splitString(data, ';', 37).toInt();
  comp_3 = splitString(data, ';', 38).toInt();
  statusSB1 = splitString(data, ';', 39).toInt();
  statusSB2 = splitString(data, ';', 40).toInt();

                    distValue_1 = procValdepan(dist_1);
                    distValue_2 = procValdepan(dist_2);
                    distValue_3 = procVal(dist_3);
                    distValue_4 = procVal(dist_4);
                    distValue_5 = procVal(dist_5);
                    distValue_6 = procVal(dist_6);
                    distValue_7 = procVal(dist_7);
                    distValue_8 = procVal(dist_8);
                    distValue_9 = procVal(dist_9);
                    distValue_10 = procVal(dist_10);
                    distValue_11 = procValdepan(dist_11);
                    distValue_12 = procValdepan(dist_12);

                  
                    const char* timestampCharArray = timestamp.c_str();
                    unsigned long unixTimestamp  = convertToUnixTimestamp(timestampCharArray);
                    String currentTimestamp = String(unixTimestamp)+String("000");
                    //Serial.println(currentTimestamp);
                    const char* s = currentTimestamp.c_str();  // Example input string
                    // Mengonversi String menjadi unsigned long
                    decimalValues = stringToUnsignedLongLong(s);
                    //Serial.println(decimalValue);


                  getData = "GE;" + sn + ";" + timestamp + ";" + voltage + ";" + temperature + ";" + humidity + ";" + transducer + ";" + ther_1
                        + ";" + ther_2 + ";" + ther_3 + ";" + ther_4 + ";" + ther_5 + ";" + ther_6 + ";" + ther_7 + ";" + ther_8
                        + ";" +  distValue_1 + ";" + distValue_2 + ";" + distValue_3 + ";" + distValue_4 + ";" + distValue_5 + ";" + distValue_6 + ";" + distValue_7
                        + ";" + distValue_8 + ";" + distValue_9 + ";" + distValue_10 + ";" + distValue_11 + ";" + distValue_12 + ";" + tpms_1 + ";" + tpms_2
                        + ";" + tpms_3 + ";" + tpms_4 + ";" + tpms_5 + ";" + tpms_6 + ";" + tpms_7 + ";" + tpms_8 + ";" + statusFT
                        + ";" + comp_1 + ";" + comp_2 + ";" + comp_3 + ";" + statusSB1 + ";" + statusSB2;

                    

                    for (int i = 0; i < 1; i++) {
                        String serverPath2 = tabletName + getData;

                        http2.begin(client, serverPath2.c_str());

                        Serial.println(serverPath2.c_str());
                       
                        int httpResponseCode = http2.GET();
                        if (httpResponseCode > 0) {
                           // Serial.print("HTTP Response code 2: ");
                           // Serial.println(httpResponseCode);
                        } else {
                           // Serial.print("Error code 2: ");
                           // Serial.println(http.errorToString(httpResponseCode).c_str());
                        }
                        http2.end();
               }
           }
                
           
        




void loop() {
server.handleClient();
static unsigned long previousMillis = 0;
unsigned long currentMillis = millis();
unsigned long interval =20000; // Interval pengiriman data raw string (40 detik)
if (SeArduino.available()) {
    String dataString = SeArduino.readStringUntil('\n'); // Membaca data sampai newline
    Serial.println(dataString);
    dataString.trim(); // Menghapus spasi di awal dan akhir (jika ada)
    // Memeriksa panjang data minimal untuk memastikan data diterima dengan lengkap
    if (dataString.length() > 0) {
        // Memisahkan data menggunakan delimiter (;)
        String dataArray[44]; // Array untuk menyimpan elemen data (maksimum 44 elemen)
        int expectedElements = 41; // Jumlah elemen yang diharapkan

        // Pemisahan data menggunakan delimiter (;)
        int index = 0;
        int countDelimiters = 0;

        while (dataString.length() > 0 && index < 44) {
            int delimiterIndex = dataString.indexOf(';');
            if (delimiterIndex >= 0) {
                String element = dataString.substring(0, delimiterIndex);
                dataArray[index++] = element;
                dataString = dataString.substring(delimiterIndex + 1);
                countDelimiters++;
            } else {
                // Jika sudah tidak ada delimiter lagi, anggap sisa string sebagai elemen terakhir
                dataArray[index++] = dataString;
                dataString = "";
            }
        }
        if (index == expectedElements && countDelimiters >= 38 && countDelimiters <= 44) {
            //Serial.println("Data valid:");
            String combinedData;
            for (int i = 0; i < expectedElements; i++) {
                combinedData += dataArray[i];
                if (i < expectedElements - 1) {
                    combinedData += ";";
                }
            }
            //Serial.println(combinedData);
            data = combinedData;
            getdata();
        } else {
            // Data tidak sesuai dengan jumlah elemen yang diharapkan atau jumlah delimiter tidak sesuai
            // Serial.println("Data tidak valid. Jumlah elemen tidak sesuai atau jumlah delimiter tidak sesuai.");
        }
    }
    //data = dataString;
    //getdata();
}
  

if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Serial.println("1");
    client.connect(host,port);
    // delay(5000);
  if(client.connected()){
    String idMac ="0006"+macAddress;    
    SendingHex(idMac);
    // Serial.println("1.1");
    
  }
  while(client.available()){
    char ch = static_cast<char>(client.read());
    if(ch==0x01){
      Serial.print("Success login to Server :");
      //Serial.println(host);
      SuccessfullLogin =true;
   }
  }
  if(!client.connected())
  {
    client.connect(host,port);
    // Serial.println(host);
    // Serial.println(port);
    // delay(3000);
  }
  else
  {
   if(SuccessfullLogin)
  {     
    insertEventIO(11, voltage*100, 2);
    insertEventIO(12, temperature*100, 2);
    insertEventIO(14, humidity*100, 2);
    insertEventIO(13, transducer*100, 2);
    insertEventIO(51, ther_1*100, 2);
    insertEventIO(52, ther_2*100, 2);
    insertEventIO(53, ther_3*100, 2);
    insertEventIO(54, ther_4*100, 2);
    insertEventIO(55, ther_5*100, 2);
    insertEventIO(56, ther_6*100, 2);
    insertEventIO(57, ther_7*100, 2);
    insertEventIO(58, ther_8*100, 2);
    insertEventIO(301, distValue_1, 1);
    insertEventIO(302, distValue_2, 1);
    insertEventIO(303, distValue_3, 1);
    insertEventIO(304, distValue_4, 1);
    insertEventIO(305, distValue_5, 1);
    insertEventIO(306, distValue_6, 1);
    insertEventIO(307, distValue_7, 1);
    insertEventIO(308, distValue_8, 1);
    insertEventIO(309, distValue_9, 1);
    insertEventIO(310, distValue_10, 1);
    insertEventIO(311, distValue_11, 1);
    insertEventIO(312, distValue_12, 1);
    insertEventIO(71, tpms_1*100, 2);
    insertEventIO(72, tpms_2*100, 2);
    insertEventIO(73, tpms_3*100, 2);
    insertEventIO(74, tpms_4*100, 2);
    insertEventIO(75, tpms_5*100, 2);
    insertEventIO(76, tpms_6*100, 2);
    insertEventIO(77, tpms_7*100, 2);
    insertEventIO(78, tpms_8*100, 2);
    insertEventIO(61, statusFT, 1);
    insertEventIO(21, comp_1, 1);
    insertEventIO(22, comp_2, 1);
    insertEventIO(23, comp_3, 1);
    insertEventIO(401, statusSB1, 1);
    insertEventIO(402, statusSB2, 1);

    String hexString = toHexString(decimalValues);
    Serial.println(hexString);
    if(client.connected()){
      SendingHex(hexString);
      //  Serial.println("4");
      //delay(5000);
    }
  }
  else{
    //  Serial.println("5");
    ReLogin();
     }
    }
   }
  }

void handle_AdminSignalA1off() {
  Serial.println("#1");
  SeArduino.println("#1");
  SendHTMLAdmin(); 
  server.send(200, "text/html");

}

void handle_AdminSignalA1on() {
  Serial.println("#2");
   SeArduino.println("#2");
  SendHTMLAdmin(); 
  server.send(200, "text/html");
}

void handle_AdminSignalA2off() {
  Serial.println("#3");
   SeArduino.println("#3");
  SendHTMLAdmin(); 
  server.send(200, "text/html");
}

void handle_AdminSignalA2on() {
  Serial.println("#4");
   SeArduino.println("#4");
  SendHTMLAdmin(); 
  server.send(200, "text/html");
}

void handle_AdminSignalA3off() {
  Serial.println("#5");
   SeArduino.println("#5");
  SendHTMLAdmin(); 
  server.send(200, "text/html");
}

void handle_AdminSignalA3on() {
  Serial.println("#6");
   SeArduino.println("#6");
  SendHTMLAdmin(); 
  server.send(200, "text/html");
}

void handle_AdminSignalAoff() {
  Serial.println("#0");
   SeArduino.println("#0");
  SendHTMLAdmin(); 
  server.send(200, "text/html");
}

void handle_AdminSignalAon(){
  Serial.println("#9");
   SeArduino.println("#9");
  SendHTMLAdmin(); 
  server.send(200, "text/html");
}

void handle_P1off() {
  Serial.println("#1");
   SeArduino.println("#1");
  server.send(200, "text/html"); 
}

void handle_P1on() {
  Serial.println("#2");
   SeArduino.println("#2");
  server.send(200, "text/html"); 
}

void handle_P2off() {
  Serial.println("#3");
   SeArduino.println("#3");
  server.send(200, "text/html"); 
}

void handle_P2on() {
  Serial.println("#4");
   SeArduino.println("#4");
  server.send(200, "text/html"); 
}

void handle_P3off() {
  Serial.println("#5");
   SeArduino.println("#5");
  server.send(200, "text/html"); 
}

void handle_P3on() {
  Serial.println("#6");
   SeArduino.println("#6");
  server.send(200, "text/html"); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTMLMain()); 
}

int procVal(int val){
    int result;
    if (val >= 200 && val <= 1200) {
    { result = 0; }// GOOD DISTANCE
    } else if (val > 15 && val < 200) {
     {   result = 1; }// DISTANCE WARNING
    } else if (val >= 1200 || val <= 15) {
     {  result = 2; }// DISTANCE ERROR
    } else {
     {   result = 0; }// SENSOR NOT CONNECTED
    }
    return result;
}

int procValdepan(int valdepan){
int resultdepan;
if(valdepan >= 400 && valdepan <= 1200)
  { resultdepan = 0; } //GOOD DISTANCE
else if (valdepan > 15 && valdepan < 400)
  { resultdepan = 1; } //DISTANCE WARNING
else if (valdepan >= 1200 || valdepan <= 15) 
  { resultdepan = 2;  }// DISTANCE ERROR
else
  { resultdepan = 0; } //SENSOR NOT CONNECTED
  return resultdepan;
}

String SendHTMLMain(){
  String ptr = "<!DOCTYPE html><html>\n";
  ptr +="<head>\n";
  ptr +="<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Pneumatic Control</title>\n";
  ptr +="<style>\n";
  ptr +="html {font-family: Tahoma, sans-serif; display: inline-block; margin: 0 auto; text-align: center;}\n";
  ptr +="body {background-color: #444445; margin: 0; padding: 0;}\n";
  ptr +="table {width: 90%; max-width: 600px; border-collapse: collapse; margin: 20px auto;}\n";
  ptr +="th, td {border: 1px solid #727272; padding: 5px;}\n";
  ptr +="th {background-color: #37363c; color: #ccccce;}\n";
  ptr +="td {background-color: #56565c; color: #ccccce;}\n";
  ptr +=".legend {width: 90%; max-width: 600px; margin: 20px auto; background-color: #37363c; color: #ccccce; padding: 10px; border-radius: 5px;}\n";
  ptr +=".legend ul {list-style-type: none; padding: 0; margin: 0;}\n";
  ptr +=".legend li {text-align: left; padding: 5px 0;}\n";
  ptr +=".title-header {font-size: 28px;}\n";
  ptr +="@media (max-width: 600px) {\n";
  ptr +="th, td {padding: 5px;}\n";
  ptr +="th[colspan=\"4\"], td[colspan=\"4\"] {font-size: 0.9em;}\n";
  ptr +="th, td {font-size: 0.8em;}\n";
  ptr +=".legend {padding: 5px;}\n";
  ptr +=".legend li {font-size: 0.8em;}\n";
  ptr +="}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<table>\n";
  ptr +="<tr><th colspan=\"4\" class=\"title-header\">SMART MT PENSEAL</th></tr>\n";
  ptr +="<tr><th colspan=\"4\">DASHBOARD " + nopol + "</th></tr>\n";
  ptr +="<tr><th>Electronic Sensor</th><th>Pressure</th><th>Temperature</th><th>Humidity</th></tr>\n";
  ptr +="<tr><td>" + String(voltage) + "v</td><td>" + String(transducer) + "psi</td><td>" + String(temperature) + "c</td><td>" + String(humidity) + "%</td></tr>\n";
  ptr +="<tr><th>Thermal 1</th><th>Thermal 2</th><th>Thermal 3</th><th>Thermal 4</th></tr>\n";
  ptr +="<tr><td>" + String(ther_1) + "c</td><td>" + String(ther_2) + "c</td><td>" + String(ther_3) + "c</td><td>" + String(ther_4) + "c</td></tr>\n";
  ptr +="<tr><th>Thermal 5</th><th>Thermal 6</th><th>Thermal 7</th><th>Thermal 8</th></tr>\n";
  ptr +="<tr><td>" + String(ther_5) + "c</td><td>" + String(ther_6) + "c</td><td>" + String(ther_7) + "c</td><td>" + String(ther_8) + "c</td></tr>\n";
  ptr +="<tr><th>Distance 1</th><th>Distance 2</th><th>Distance 3</th><th>Distance 4</th></tr>\n";
  ptr +="<tr><td>" + String(dist_1) + "cm</td><td>" + String(dist_2) + "cm</td><td>" + String(dist_3) + "cm</td><td>" + String(dist_4) + "cm</td></tr>\n";
  ptr +="<tr><th>Distance 5</th><th>Distance 6</th><th>Distance 7</th><th>Distance 8</th></tr>\n";
  ptr +="<tr><td>" + String(dist_5) + "cm</td><td>" + String(dist_6) + "cm</td><td>" + String(dist_7) + "cm</td><td>" + String(dist_8) + "cm</td></tr>\n";
  ptr +="<tr><th>Distance 9</th><th>Distance 10</th><th>Distance 11</th><th>Distance 12</th></tr>\n";
  ptr +="<tr><td>" + String(dist_9) + "cm</td><td>" + String(dist_10) + "cm</td><td>" + String(dist_11) + "cm</td><td>" + String(dist_12) + "cm</td></tr>\n";
  ptr +="<tr><th colspan=\"2\">Status Flametrap</th><th>Status Seatbelt 1</th><th>Status Seatbelt 2</th></tr>\n";
  ptr +="<tr><td colspan=\"2\">" + String(statusFT) + "</td><td>" + String(statusSB1) + "</td><td>" + String(statusSB2) + "</td></tr>\n";
  ptr +="<tr><th>TPMS 1</th><th>TPMS 2</th><th>TPMS 3</th><th>TPMS 4</th></tr>\n";
  ptr +="<tr><td>" + String(tpms_1) + "psi</td><td>" + String(tpms_2) + "psi</td><td>" + String(tpms_3) + "psi</td><td>" + String(tpms_4) + "psi</td></tr>\n";
  ptr +="<tr><th>TPMS 5</th><th>TPMS 6</th><th>TPMS 7</th><th>TPMS 8</th></tr>\n";
  ptr +="<tr><td>" + String(tpms_5) + "psi</td><td>" + String(tpms_6) + "psi</td><td>" + String(tpms_7) + "psi</td><td>" + String(tpms_8) + "psi</td></tr>\n";
  ptr +="<tr><th>MCU IMEI</th><th>UNIQ ID EDR</th><th colspan=\"2\">Timestamp UTC0</th></tr>\n";
  ptr +="<tr><td>" + String("0006"+macAddress)+"</td><td>"+String(sn)+"</td><td colspan=\"2\">"+String(timestamp)+"</td></tr>\n";
  ptr +="</table>\n";
  ptr +="<div class=\"legend\">\n";
  ptr +="<h3>Keterangan Sensor</h3>\n";
  ptr +="<ul>\n";
  ptr +="<li><strong>Electronic Sensor:</strong> Sensor tegangan yang masuk ke dalam MCU atau main control unit</li>\n";
  ptr +="<li><strong>Pressure:</strong> Sensor pressure yang membaca angin yang masuk ke panseal dari interlock</li>\n";
  ptr +="<li><strong>Temperature:</strong> Sensor temperature yang ada pada dalam box MCU atau sensor suhu di MCU</li>\n";
  ptr +="<li><strong>Humidity:</strong> Sensor kelembapan yang berada pada dalam box MCU</li>\n";
  ptr +="<li><strong>Thermal 1:</strong> Sensor thermocouple yang berada pada depan kiri ban</li>\n";
  ptr +="<li><strong>Thermal 2:</strong> Sensor thermocouple yang berada pada depan kanan ban</li>\n";
  ptr +="<li><strong>Thermal 3:</strong> Sensor thermocouple yang berada di tengah kiri ban</li>\n";
  ptr +="<li><strong>Thermal 4:</strong> Sensor thermocouple yang berada di tengah kanan ban</li>\n";
  ptr +="<li><strong>Thermal 5:</strong> Sensor thermocouple yang berada pada belakang kiri ban</li>\n";
  ptr +="<li><strong>Thermal 6:</strong> Sensor thermocouple yang berada belakang kanan ban</li>\n";
  ptr +="<li><strong>Thermal 7:</strong> Sensor thermocouple yang berada pada paling belakang kiri ban</li>\n";
  ptr +="<li><strong>Thermal 8:</strong> Sensor thermocouple yang berada pada paling belakang kanan ban</li>\n";
  ptr +="<li><strong>Distance 1-12:</strong> Sensor jarak berurutan dari kiri depan ke kanan belakang</li>\n";
  ptr +="<li><strong>Status Flametrap:</strong> Keterangan status open or close flametrap</li>\n";
  ptr +="<li><strong>Status Seatbelt:</strong> Keterangan status seatbelt pada kursi AMT1 dan AMT2</li>\n";
  ptr +="<li><strong>TPMS 1-8:</strong> Sensor tekanan ban berurutan dari kiri depan ke kanan belakang</li>\n";
  ptr +="<li><strong>MCU IMEI:</strong> Keterangan Uniq id main control unit smartmt merupakan identitas mcu di server</li>\n";
  ptr +="<li><strong>UNIQ ID EDR:</strong> Keterangan Uniq id EDR (electronic data recorder) </li>\n";
  ptr +="<li><strong>Timestamp UTC0:</strong> merupakan waktu yang terkonfigurasi MCU dalam zona utc 0 atau perbedaan -7 jam dengan WIB (jika tidak sesuai tolong konfigurasi kembali)</li>\n";
  ptr +="</ul>\n";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}


void SendHTMLAdmin() {
  String html = R"=====(
  <!DOCTYPE html> 
  <html>
  <head><meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Pneumatic Control</title>
  <style>html {font-family: tahoma; display: inline-block; margin: 0px auto; text-align: center;}
  body{background-color: #444445;}
  .button {display: inline-block;width: 80px;background-color: #19191a; border: 3px solid #7d7d88;color: #eeeef0;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 10px;cursor: pointer;border-radius: 8px;}
  .button-on {background-color: #19191a;}
  .button-on:active {background-color: #4f4f53;}
  .button-off {background-color: #19191a;}
  .button-off:active {background-color: #4f4f53;}
  
  table{table-layout:auto; width:350px; border-collapse: inherit;}
  tr:first-child td:first-child{border-top-left-radius: 10px;}
  tr:first-child td:last-child {border-top-right-radius: 10px;}
  tr:nth-child(5) td:nth-child(1){border-bottom-left-radius: 10px; border-bottom-right-radius: 10px;}  
  
  tr:nth-child(7) td:nth-child(1){border-top-left-radius: 5px; border-top-right-radius: 5px; border-left: 3px solid #56565c; border-top: 3px solid #56565c; border-right: 3px solid #56565c;}  
  tr:nth-child(8) td:nth-child(1){border-left: 3px solid #56565c;}
  tr:nth-child(8) td:nth-child(2){border-right: 3px solid #56565c;}
  tr:nth-child(9) td:nth-child(1){border-left: 3px solid #56565c; border-bottom: 3px solid #56565c; border-right: 3px solid #56565c; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px;}  

  tr:nth-child(11) td:nth-child(1){border-top-left-radius: 5px; border-top-right-radius: 5px; border-left: 3px solid #56565c; border-top: 3px solid #56565c; border-right: 3px solid #56565c;}  
  tr:nth-child(12) td:nth-child(1){border-left: 3px solid #56565c;}
  tr:nth-child(12) td:nth-child(2){border-right: 3px solid #56565c;}
  tr:nth-child(13) td:nth-child(1){border-left: 3px solid #56565c; border-bottom: 3px solid #56565c; border-right: 3px solid #56565c; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px;}  

  tr:nth-child(15) td:nth-child(1){border-top-left-radius: 5px; border-top-right-radius: 5px; border-left: 3px solid #56565c; border-top: 3px solid #56565c; border-right: 3px solid #56565c;}  
  tr:nth-child(16) td:nth-child(1){border-left: 3px solid #56565c;}
  tr:nth-child(16) td:nth-child(2){border-right: 3px solid #56565c;}
  tr:nth-child(17) td:nth-child(1){border-left: 3px solid #56565c; border-bottom: 3px solid #56565c; border-right: 3px solid #56565c; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px;}  

  tr:nth-child(19) td:nth-child(1){border-top-left-radius: 5px; border-top-right-radius: 5px; border-left: 3px solid #56565c; border-top: 3px solid #56565c; border-right: 3px solid #56565c;}  
  tr:nth-child(20) td:nth-child(1){border-left: 3px solid #56565c;}
  tr:nth-child(20) td:nth-child(2){border-right: 3px solid #56565c;}
  tr:nth-child(21) td:nth-child(1){border-left: 3px solid #56565c; border-bottom: 3px solid #56565c; border-right: 3px solid #56565c; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px;}  
  
  td{font-family: tahoma; color: #ccccce; font-size: 20px; font-weight: normal;} 
  tr:nth-child(2) td:nth-child(1){font-weight:bold;}
  tr:nth-child(4) td:nth-child(1){font-size: 16px; color: #969698;}

  @media only screen and (min-width: 450px){table{table-layout:auto; width:400px; border-collapse: inherit;}}
  </style>
  </head>
  <body align='center'>

  <table align='center' cellspacing='0' cellpadding='0';>
    <tr height='5'>
      <td style='background-color:#37363c'; colspan='2'></td>
    </tr>
    
    <tr>
      <td style='background-color:#37363c'; colspan='2'>SMART MT PENSEAL</td>
    </tr>
    
    <tr height='5'>
      <td style='background-color:#37363c'; colspan='2'></td>
    </tr>
    
    <tr>
      <td colspan='2' style='background-color:#37363c'>CONTROLLER</td>
    </tr>

    <tr height='5'>
      <td style='background-color:#37363c'; colspan='2'></td>
    </tr>

    <tr height='30'>
      <td colspan='2'></td>
    </tr>
    
    <tr vertical-align='center' height='50'>
      <td style='background-color:#56565c'; colspan='2'>Compartment 1</td>
    </tr>
    
    <tr>
      <td style='background-color:#56565c';><a class="button button-off" href="/a1off">OFF</a></td>
      <td style='background-color:#56565c';><a class="button button-off" href="/a1on">ON</a></td>
    </tr>

    <tr height='5'>
      <td style='background-color:#56565c'; colspan='2'></td>
    </tr>

    <tr height='30'>
      <td colspan='2'></td>
    </tr>
    
    <tr vertical-align='center' height='50'>
      <td style='background-color:#56565c' colspan='2'>Compartment 2</td>
    </tr>
    
    <tr>
      <td style='background-color:#56565c';><a class="button button-off" href="/a2off">OFF</a></td>
      <td style='background-color:#56565c';><a class="button button-off" href="/a2on">ON</a></td>
    </tr>

     <tr height='5'>
      <td style='background-color:#56565c'; colspan='2'></td>
    </tr>

    <tr height='30'>
      <td colspan='2'></td>
    </tr>
    
    <tr vertical-align='center' height='50'>
      <td style='background-color:#56565c' colspan='2'>Compartment 3</td>
    </tr>
    
    <tr>
      <td style='background-color:#56565c';><a class="button button-off" href="/a3off">OFF</a></td>
      <td style='background-color:#56565c';><a class="button button-off" href="/a3on">ON</a></td>
    </tr>

    <tr height='5'>
      <td style='background-color:#56565c'; colspan='2'></td>
    </tr>

    <tr height='30'>
      <td colspan='2'></td>
    </tr>

    <tr vertical-align='center' height='20'>
      <td style='background-color:#56565c' colspan='2'></td>
    </tr>
    
    <tr>      
      <td style='background-color:#56565c';><a class="button button-off" href="/aoff">CLOSE ALL</a></td>      
      <td style='background-color:#56565c';><a class="button button-off" href="/aon">OPEN ALL</a></td>                                            
    </tr>

    <tr height='5'>
      <td style='background-color:#56565c'; colspan='2'></td>
    </tr>

    <tr height='30'>
      <td colspan='2'></td>
    </tr>


    </tr>
      <tr vertical-align='center' height='20'>
    <td style='background-color:#56565c' colspan='2'></td>
    </tr>
    <tr>
      <td style='background-color:#56565c'; colspan='2'><a class="button button-off" href="/">Sensor</a></td>
    </tr>
      <tr height='5'>
      <td style='background-color:#56565c'; colspan='2'></td>
    </tr>



  </table>
  </body>
  </html>
  )=====";
  server.send(200, "text/html", html);
}


void handle_config() {
  String html = "<html><head><style>";
  html += "html {font-family: Tahoma, sans-serif; display: inline-block; margin: 0 auto; text-align: center;}\n";
  html += "body {background-color: #444445; margin: 0; padding: 0;}\n";
  html += "h1 { color: #ccccce; }\n";
  html += "p { color: #ccccce; }\n";
  html += "form { max-width: 400px; margin: 20px auto; padding: 20px; background-color: #37363c; border-radius: 8px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }\n";
  html += "label { display: inline-block; width: 100px; margin: 5px 0; color: #ccccce; }\n";
  html += "input[type='text'], input[type='date'] { width: calc(100% - 112px); padding: 10px; margin: 5px 0; box-sizing: border-box; background-color: #56565c; color: #ccccce; border: none; }\n";
  html += "input[type='submit'] { width: 100%; padding: 10px; margin-top: 10px; background-color: #4caf50; color: #ffffff; border: none; border-radius: 4px; cursor: pointer; }\n";
  html += "</style></head><body>";
  html += "<h1>Setting Jam RTC</h1>";
  html += "<form action='/settingwaktu' method='get'>";
  html += "<label for='date'>Tanggal:</label><input type='date' id='date' name='date'><br>";
  html += "<label for='hour'>Jam:</label><input type='text' id='hour' name='hour'><br>";
  html += "<label for='minute'>Menit:</label><input type='text' id='minute' name='minute'><br>";
  html += "<label for='second'>Detik:</label><input type='text' id='second' name='second'><br>";
  html += "<input type='submit' value='Set Waktu'>";
  html += "</form>";
  html += "<h1>Nomor Plat Mobil</h1>";
  html += "<p>Nomor plat saat ini: " + nopol + "</p>";
  html += "<form action='/update' method='POST'>";
  html += "<p>Nomor Plat Baru:</p> <input type='text' name='plate'><br>";
  html += "<input type='submit' value='Update'>";
  html += "</form>";

  html += "<h1>Konfigurasi Host dan Port</h1>";
  html += "<p>Host saat ini: " + String(host) + "</p>";
  html += "<p>Port saat ini: " + String(port) + "</p>";
  html += "<form action='/update_hostport' method='POST'>";
  html += "<p>Host Baru:</p> <input type='text' name='host'><br>";
  html += "<p>Port Baru:</p> <input type='text' name='port'><br>";
  html += "<input type='submit' value='Update Host dan Port'>";
  html += "</form>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handle_UpdateWaktu() {
  String dateParam = server.arg("date");
  String hourParam = server.arg("hour");
  String minuteParam = server.arg("minute");
  String secondParam = server.arg("second");

  int year = dateParam.substring(0, 4).toInt();
  int month = dateParam.substring(5, 7).toInt();
  int day = dateParam.substring(8, 10).toInt();
  int hour = hourParam.toInt();
  int minute = minuteParam.toInt();
  int second = secondParam.toInt();

  formattedDate = String(year) + print2digits(month) + print2digits(day) + print2digits(hour) + print2digits(minute) + print2digits(second);
  Serial.println("Set Time: " + formattedDate);  // Menampilkan waktu yang diatur ke Serial Monitor
  SeArduino.println("set");  // Menampilkan waktu yang diatur ke Serial Monitor
  delay(4000);
  SeArduino.println(formattedDate);  // Menampilkan waktu yang diatur ke Serial Monitor
  String html = "<html><head><style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; }\n";
  html += "body {background-color: #444445; margin: 0; padding: 0;}\n";
  html += "h1 { color: #4caf50; }\n"; // Green color for headings
  html += "p { color: #ccccce; }\n"; // Dark gray color for paragraphs
  html += "a { color: #4caf50; text-decoration: none; }\n"; // Green color for links
  html += "</style></head><body>";
  html += "<h1>Update berhasil!</h1>";
  html += "<p>Waktu RTC berhasil diatur: " + formattedDate + "</p>";
  html += "<a href='/set'>Kembali</a>";
  html += "</body></html>";
  server.send(200, "text/html", html); // Send 200 OK response for successful update
  // server.send(200, "text/plain", "Waktu RTC berhasil diatur: " + formattedDate);
}
void handle_UpdatePlate() {
  if (server.hasArg("plate")) {
    String newPlate = server.arg("plate");
    if (newPlate.length() > 0) {
      nopol = newPlate;
      for (int i = 0; i < nopol.length(); ++i) {
        EEPROM.write(i, nopol[i]);
      }
      EEPROM.write(nopol.length(), '\0');
      EEPROM.commit();
      // Successful update response
      String html = "<html><head><style>";
      html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; }\n";
      html += "body {background-color: #444445; margin: 0; padding: 0;}\n";
      html += "h1 { color: #4caf50; }\n"; // Green color for headings
      html += "p { color: #333; }\n"; // Dark gray color for paragraphs
      html += "a { color: #4caf50; text-decoration: none; }\n"; // Green color for links
      html += "</style></head><body>";
      html += "<h1>Update berhasil!</h1>";
      html += "<a href='/set'>Kembali</a>";
      html += "</body></html>";
      server.send(200, "text/html", html); // Send 200 OK response for successful update
    } else {
      // Invalid plate number response
      String html = "<html><head><style>";
      html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; }\n";
      html += "body {background-color: #444445; margin: 0; padding: 0;}\n";
      html += "h1 { color: #ff3333; }\n"; // Red color for headings on failure
      html += "p { color: #333; }\n"; // Dark gray color for paragraphs
      html += "a { color: #4caf50; text-decoration: none; }\n"; // Green color for links
      html += "</style></head><body>";
      html += "<h1>Update gagal. Nomor plat tidak valid.</h1>";
      html += "<a href='/set'>Kembali</a>";
      html += "</body></html>";
      server.send(400, "text/html", html); // Send 400 Bad Request response for invalid plate number
    }
  }
}

void handle_UpdateHostPort() {
  if (server.hasArg("host") && server.hasArg("port")) {
    String newHost = server.arg("host");
    int newPort = server.arg("port").toInt();

    if (newHost.length() > 0 && newPort > 0) {
      // Update host dan port
      host = strdup(newHost.c_str());  // Allocate new memory for the host string
      port = newPort;

      // Menyimpan ke EEPROM
      EEPROM.write(200, newHost.length());  // Write host length for retrieval
      for (int i = 0; i < newHost.length(); ++i) {
        EEPROM.write(201 + i, newHost[i]);  // Starting at address 201 for host
      }
      EEPROM.put(300, port);  // Store port starting at address 300
      EEPROM.commit();

      // Tampilkan pesan sukses
      String html = "<html><head><style>";
      html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; }\n";
      html += "body {background-color: #444445; margin: 0; padding: 0;}\n";
      html += "h1 { color: #4caf50; }\n";
      html += "</style></head><body>";
      html += "<h1>Update berhasil!</h1>";
      html += "<a href='/set'>Kembali</a>";
      html += "</body></html>";
      server.send(200, "text/html", html);
    } else {
      // Tampilkan pesan error
      String html = "<html><head><style>";
      html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; }\n";
      html += "body {background-color: #444445; margin: 0; padding: 0;}\n";
      html += "h1 { color: #ff3333; }\n";
      html += "</style></head><body>";
      html += "<h1>Update gagal. Host atau port tidak valid.</h1>";
      html += "<a href='/set'>Kembali</a>";
      html += "</body></html>";
      server.send(400, "text/html", html);
    }
  }
}


String print2digits(int number) {
  if (number < 10) {
    return "0" + String(number);
  }
  return String(number);
}