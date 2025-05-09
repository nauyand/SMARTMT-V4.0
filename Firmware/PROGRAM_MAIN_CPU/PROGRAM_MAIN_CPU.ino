#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include "DHT.h"
#include "max6675.h"
#include <ArduinoUniqueID.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#define MasterEn 49        // Pin untuk RS485 Enable
#define EEPROM_ADDR 0x50  // Alamat I2C AT24256
#define PASSWORD_ADDRESS 0  // Alamat penyimpanan password di EEPROM
const char validCode[] = "CITIA1234"; // Kode keamanan yang harus ada di EEPROM

// Tentukan pin RX dan TX yang akan digunakan
#define RX_PIN 10 // Pin RX
#define TX_PIN 11 // Pin TX

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Inisialisasi objek SoftwareSerial

void readEEPROM(unsigned int address, char* buffer, int length) {
  for (int i = 0; i < length; i++) {
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write((address + i) >> 8); // MSB
    Wire.write((address + i) & 0xFF); // LSB
    Wire.endTransmission();
    
    Wire.requestFrom(EEPROM_ADDR, 1);
    if (Wire.available()) {
      buffer[i] = Wire.read();
    }
    delay(5);  // Beri jeda untuk komunikasi EEPROM
  }
  buffer[length] = '\0';  // Tambahkan null terminator untuk string
}

char UniqIdBuffers [18];
String UniqID = "";
String DataToESP = "";
String PenToESP = "";
String DataToLogger = "";

String PenBefEncrypt = "";
int FlagWriteFile = 0;

float kalibrasiSuhu(float temp) {
  float suhuKalibrasi = temp;
  if (temp >= 30 && temp <= 100) {
    float persen = (temp - 30) / 70.0; // 70 = (100 - 30)
    float koreksi = 1.0 + persen * 31.0;   // 31 = (32 - 1)
    suhuKalibrasi = temp + koreksi;
  }
  else if (temp > 100) {
    suhuKalibrasi = temp + 32.0;
  }
  return suhuKalibrasi;
}

const int thermoDO_[8] = {42, 44, 36, 35, 39, 29, 25, 24};
const int thermoCS_[8] = {38, 45, 37, 34, 32, 28, 23, 27};
const int thermoCLK_[8] = {43, 41, 40, 33, 31, 30, 22, 26};

float tc1 = 0.0, tc2 = 0.0, tc3 = 0.0, tc4 = 0.0, tc5 = 0.0, tc6 = 0.0, tc7 = 0.0, tc8 = 0.0;
float max_thermo = 2000.00;
String datatcbelakang = "0.00;0.00;0.00;0.00";
String datatc = "0.00;0.00;0.00;0.00"; // Membaca seluruh string yang dikirim
MAX6675 thermocouple_[8] = {
  MAX6675(thermoCLK_[0], thermoCS_[0], thermoDO_[0]),
  MAX6675(thermoCLK_[1], thermoCS_[1], thermoDO_[1]),
  MAX6675(thermoCLK_[2], thermoCS_[2], thermoDO_[2]),
  MAX6675(thermoCLK_[3], thermoCS_[3], thermoDO_[3]),
  MAX6675(thermoCLK_[4], thermoCS_[4], thermoDO_[4]),
  MAX6675(thermoCLK_[5], thermoCS_[5], thermoDO_[5]),
  MAX6675(thermoCLK_[6], thermoCS_[6], thermoDO_[6]),
  MAX6675(thermoCLK_[7], thermoCS_[7], thermoDO_[7])
};
void GetThermocouple(float &tc1,float &tc2,float &tc3,float &tc4,float &tc5,float &tc6,float &tc7,float &tc8) {
  float temp1 = thermocouple_[0].readCelsius();
  float temp2 = thermocouple_[1].readCelsius();
  float temp3 = thermocouple_[2].readCelsius();
  float temp4 = thermocouple_[3].readCelsius();
  float temp5 = thermocouple_[4].readCelsius();
  float temp6 = thermocouple_[5].readCelsius();
  float temp7 = thermocouple_[6].readCelsius();
  float temp8 = thermocouple_[7].readCelsius();
  if(isnan(temp1)){
    temp1 = 0;
  }
  if(isnan(temp2)){
    temp2 = 0;
  }
  if(isnan(temp3)){
    temp3 = 0;
  }    
  if(isnan(temp4)){
    temp4 = 0;
  } 
  if(isnan(temp5)){
    temp5 = 0;
  } 
  if(isnan(temp6)){
    temp6 = 0;
  } 
   if(isnan(temp7)){
    temp7 = 0;
  } 
  if(isnan(temp8)){
    temp8 = 0;
  } 
  tc1 = kalibrasiSuhu(temp1);   // Panggil fungsi kalibrasi
  tc2 = kalibrasiSuhu(temp2);   // Panggil fungsi kalibrasi
  tc3 = kalibrasiSuhu(temp3);   // Panggil fungsi kalibrasi
  tc4 = kalibrasiSuhu(temp4);   // Panggil fungsi kalibrasi
  tc5 = kalibrasiSuhu(temp5);   // Panggil fungsi kalibrasi
  tc6 = kalibrasiSuhu(temp6);   // Panggil fungsi kalibrasi
  tc7 = kalibrasiSuhu(temp7);   // Panggil fungsi kalibrasi
  tc8 = kalibrasiSuhu(temp8);   // Panggil fungsi kalibrasi

}

int tire_position1 = 1;
float pressure1 = 0.00;

int tire_position2 = 2;
float pressure2 = 0.00;

int tire_position3 = 3;
float pressure3 = 0.00;

int tire_position4 = 4;
float pressure4 = 0.00;

int tire_position5 = 5;
float pressure5 = 0.00;

int tire_position6 = 6;
float pressure6 = 0.00;

int tire_position7 = 7;
float pressure7 = 0.00;

int tire_position8 = 8;
float pressure8 = 0.00;


void parseAndSaveData(byte data[]) {
  // Interpret sensor information
  byte tire_position = data[3];

  switch (tire_position) {
  case 1:
    tire_position1 = 1;
    pressure1 = ((data[4] * 5.5) - 99) * 0.145038;
          if (pressure1 > 150) {
    pressure1 = 0;
  }
    break;

  case 2:
    tire_position2 = 2;
    pressure2 = ((data[4] * 5.5) - 99) * 0.145038;
          if (pressure2 > 150) {
    pressure2 = 0;
  }
    break;

  case 3:
    tire_position3 = 3;
    pressure3 = ((data[4] * 5.5) - 99) * 0.145038;
          if (pressure3 > 150) {
    pressure3 = 0;
  }
    break;

  case 4:
    tire_position4 = 4;
    pressure4 = ((data[4] * 5.5) - 99) * 0.145038;
          if (pressure4 > 150) {
    pressure4 = 0;
  }
    break;

  case 5:
    tire_position5 = 5;
    pressure5 = ((data[4] * 5.5) - 99) * 0.145038;
          if (pressure5 > 150) {
    pressure5 = 0;
  }
    break;

  case 6:
    tire_position6 = 6;
    pressure6 = ((data[4] * 5.5) - 99) * 0.145038;
          if (pressure6 > 150) {
    pressure6 = 0;
  }
    break;

  case 7:
    tire_position7 = 7;
    pressure7 = ((data[4] * 5.5) - 99) * 0.145038;
          if (pressure7 > 150) {
    pressure7 = 0;
  }
    break;

  case 8:
    tire_position8 = 8;
    pressure8 = ((data[4] * 5.5) - 99) * 0.145038;
          if (pressure8 > 150) {
    pressure8 = 0;
  }
    break;
    
    
  }
}

const int pressureInput = A14; //select the analog input pin for the pressure transducer
const int pressureZero = 102.4; //analog reading of pressure transducer at 0psi
const int pressureMax = 921.6; //analog reading of pressure transducer at 150psi
const int pressuretransducermaxPSI = 150; //psi value of transducer being used
float pressureValue = 0; //variable to store the value coming from the pressure transducer

void GetTransducer(float &pressureValue) {
    pressureValue = analogRead(pressureInput); //reads value from input pin and assigns to variable
    pressureValue = ((pressureValue - pressureZero) * pressuretransducermaxPSI) / (pressureMax - pressureZero); //conversion equation to convert analog reading to psi

  if (pressureValue > 150) {
    pressureValue = 150;
  }
  if (pressureValue < 0) {
    pressureValue = 0.00;
  }
  //  Serial.print(pressureValue, 1); //prints value from previous line to serial
}

const int voltageSensor = A15;
float Vout = 0.0;
float VoltageSensorcounting = 0.0;
float VoltageSensorReading = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
int value = 0;
void GetVoltage(float &VoltageSensorReading) {
  value = analogRead(voltageSensor);
  Vout = (value * 5.0) / 1024.0;
  VoltageSensorcounting = Vout / (R2 / (R1 + R2));
  VoltageSensorReading = VoltageSensorcounting + 1;
}

#define DHTPIN 46    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
float TemperatureSensorReading = 0.0;
float HumiditySensorReading = 0.0;
void GetHumidityTemperature(float &hum, float &tem) {
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    h = 0;
    t = 0;
  }
  hum = h;
  tem = t;
  }

String comp_1 = "2";
String comp_2 = "2";
String comp_3 = "2";

void Pn1Close(){ //-24V
  FlagWriteFile = 1;
  // compartmentString = ";2;1;0;";
  comp_1 = "1";
  delay(50);
  Serial.println("panseal 1 tertutup");
}
void Pn1Open(){ //+24V
  FlagWriteFile = 1;
  // compartmentString = ";2;1;1;";
  comp_1 = "0";
  delay(50);
  Serial.println("panseal 1 terbuka");
}
void Pn2Close(){ //-24V
  FlagWriteFile = 1;
  // compartmentString = ";2;2;0;";
  comp_2 = "1";
  delay(50);
  Serial.println("panseal 2 tertutup");
}
void Pn2Open(){ //+24V
  FlagWriteFile = 1;
  // compartmentString = ";2;2;1;";
  comp_2 = "0";
  delay(50);
  Serial.println("panseal 2 terbuka");
}
void Pn3Close(){ //-24V
  FlagWriteFile = 1;
  // compartmentString = ";2;3;0;";
  comp_3 = "1";
  delay(50);
  Serial.println("panseal 3 tertutup");
}
void Pn3Open(){ //+24V
  FlagWriteFile = 1;
  // compartmentString = ";2;3;1;";
  comp_3 = "0";
  delay(50);
  Serial.println("panseal 3 terbuka");
}
void Pn4Close(){ //-24V
  FlagWriteFile = 1;
  // compartmentString = ";2;4;0;";
  // comp_4 = "1"
  delay(50);
  Serial.println("panseal 4 tertutup");
}
void Pn4Open(){ //+24V
  FlagWriteFile = 1;
  // compartmentString = ";2;4;1;";
  // comp_4 = "0"
  delay(50);
  Serial.println("panseal 4 terbuka");
}
File GeDataFile;
File PenDataFile;
String filename1;
String filename2;
const int ChipSelect = 53;// Pin untuk modul SD Card
RTC_DS3231 rtc;
String year, month, day, hour, minute, second;
bool settingTime = false;
DateTime gmt7Time;
DateTime utc; // Variabel untuk menyimpan waktu dalam GMT+7
void enterSettingMode() {
  settingTime = true;
  Serial.println("Masukkan waktu GMT+7 dalam format YYYYMMDDHHMMSS");
  }

void setTimeInput(String input) {
  if (input.length() != 14) {
    Serial.println("Format waktu tidak valid!");
    return;
  }
  int year = input.substring(0, 4).toInt();
  int month = input.substring(4, 6).toInt();
  int day = input.substring(6, 8).toInt();
  int hour = input.substring(8, 10).toInt();
  int minute = input.substring(10, 12).toInt();
  int second = input.substring(12, 14).toInt();
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  Serial.println("Waktu RTC berhasil diatur!");
  settingTime = false;
}

int statusRelayFT ;
int statusSeatbelt1 ;
int statusSeatbelt2 ;

int randomIndex = 0;
char randomReplacement = ' ';
char replacements[10][3] = {// Array untuk menyimpan kemungkinan karakter pengganti
  {'3', 'Q', 'C'}, // Untuk 0
  {'6', 'M', 'S'}, // Untuk 1
  {'9', 'E', 'D'}, // Untuk 2
  {'2', 'R', 'F'}, // Untuk 3
  {'A', 'T', 'G'}, // Untuk 4
  {'8', 'Y', 'H'}, // Untuk 5
  {'4', 'U', 'J'}, // Untuk 6
  {'V', 'B', 'K'}, // Untuk 7
  {'5', 'L', 'N'}, // Untuk 8
  {'7', 'P', 'X'}  // Untuk 9
};
String enkripsi(String plaintext) {// Fungsi untuk mengenkripsi string menggunakan metode Caesar
  String ciphertext = ""; // Untuk menyimpan string yang dienkripsi
  for (int i = 0; i < plaintext.length(); i++) {
    char character = plaintext.charAt(i); // Ambil karakter pada posisi i
    if (isDigit(character)) {
      ciphertext += enkripsiDigit(character); // Enkripsi digit
    } else {
      ciphertext += character; // Jika bukan digit, tambahkan karakter asli ke string ciphertext
    }
  }
  return ciphertext; // Kembalikan string yang telah dienkripsi
}
char enkripsiDigit(char digit) {// Fungsi untuk mengenkripsi digit sesuai dengan aturan yang diberikan
  int digitIndex = digit - '0'; // Ubah karakter digit menjadi indeks array
  randomIndex = random(3); // Pilih secara acak antara karakter pengganti
  return replacements[digitIndex][randomIndex];
}

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(19200);
  Serial3.begin(115200);
  dht.begin();
  Wire.begin();
  pinMode(MasterEn, OUTPUT);
  digitalWrite(MasterEn, LOW);

  char storedPassword[sizeof(validCode)];
  readEEPROM(PASSWORD_ADDRESS, storedPassword, sizeof(validCode) - 1);
    // Serial.print("Stored Password: ");
  // Serial.println(storedPassword);
  if (strcmp(storedPassword, validCode) == 0) {
    Serial.println("Password BENAR! Program dilanjutkan...");
  } else {
    Serial.println("Password SALAH! Program dihentikan.");
    while (1);  // Berhenti di sini jika password salah
  }

  for (size_t i = 0; i < UniqueIDsize; i++) //Inisialisasi Uniq ID arduino
  {
    UniqID += String(UniqueID[i], HEX);
  }
    
  Serial.println("Inisialisasi SD Card...");
    if (!SD.begin(ChipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  
  
  if (!rtc.begin()) { // Inisialisasi modul RTC
    Serial.println("Gagal inisialisasi RTC!");
    return;
  }

}

void loop() {
  static unsigned long previousMillis = 0;
  unsigned long interval =3000; // Interval pengiriman data raw string (40 detik)
  unsigned long currentMillis = millis();
  
  DateTime gmt7Time = rtc.now(); // Baca waktu dari RTC
  utc = gmt7Time + TimeSpan(0, -7, 0, 0);// Hitung waktu GMT+7
  char TimeResult[21];
  sprintf(TimeResult, "%02d,%02d,%04d,%02d,%02d,%02d", gmt7Time.day(), gmt7Time.month(), gmt7Time.year(), gmt7Time.hour(), gmt7Time.minute(), gmt7Time.second());
  //Serial.print("Datetime GMT :");
  //Serial.println(TimeResult);
  String dayString = String(TimeResult).substring(0, 2);
  String monthString = String(TimeResult).substring(3, 5);
  String yearString = String(TimeResult).substring(6, 10);
  String hourString = String(TimeResult).substring(11, 13);
  String minuteString = String(TimeResult).substring(14, 16);
  String secondString = String(TimeResult).substring(17, 19);
  char TimeResultutc[21];
  sprintf(TimeResultutc, "%02d;%02d;%04d;%02d;%02d;%02d", utc.day(), utc.month(), utc.year(), utc.hour(), utc.minute(), utc.second());
  String dayStringUtc = String(TimeResultutc).substring(0, 2);
  String monthStringUtc = String(TimeResultutc).substring(3, 5);
  String yearStringUtc = String(TimeResultutc).substring(6, 10);
  String hourStringUtc = String(TimeResultutc).substring(11, 13);
  String minuteStringUtc = String(TimeResultutc).substring(14, 16);
  String secondStringUtc = String(TimeResultutc).substring(17, 19);
  char DateLogger[7];
  snprintf(DateLogger, sizeof(DateLogger), "%02d%02d%02d", gmt7Time.year() % 100, gmt7Time.month(), gmt7Time.day());
  // Serial.print("Datetime GMT :");
  // Serial.println(TimeResult);
  // Serial.print("Datetime UTC :");
  // Serial.println(TimeResultutc);
  // delay(2000);

  if (Serial.available()) {
    String input = Serial.readString();
    input.trim();
    if (input == "set") {
      enterSettingMode();
    } else if (settingTime) {
      setTimeInput(input);
    }}


  if (mySerial.available()) {
    String input = mySerial.readString();
    digitalWrite(MasterEn, HIGH);
    delay(50);
    Serial.print(input);
    Serial1.print(input); 
    delay(50);
    digitalWrite(MasterEn, LOW);

    if (input == "set") {
      enterSettingMode();
    } else if (settingTime) {
      setTimeInput(input);
    }
    input.trim();
    if (input == "#1") {
      Pn1Close();
    } 
    else if (input == "#2") {
      Pn1Open();
    }
    else if (input == "#3") {
      Pn2Close();
    }
    else if (input == "#4") {
      Pn2Open();
    }
    else if (input == "#5") {
      Pn3Close();
    }
    else if (input == "#6") {
      Pn3Open();
    }
    else if (input == "#0") {
      Pn1Close();
      delay(500);
      Pn2Close();
      delay(500);
      Pn3Close();
      delay(500);
    }
    else if (input == "#9") {
      Pn1Open();
      delay(500);
      Pn2Open();
      delay(500);
      Pn3Open();
      delay(500);
    }    
  }
  
  if (Serial1.available()) {
    datatc = Serial1.readString(); // Membaca seluruh string yang dikirim
        datatc.trim();
        for (char incomingChar : datatc) {
            // Cek apakah karakter termasuk yang diperbolehkan
            if ((incomingChar >= '0' && incomingChar <= '9') ||
                incomingChar == ',' || incomingChar == ';' || incomingChar == '#'|| incomingChar == '.') {
                datatcbelakang += incomingChar; // Menambahkan karakter yang valid ke string
            }
        }
        
        if (datatc.length() > 0) {
            Serial.println(datatc); // Menampilkan string yang sudah difilter
        }
  Serial1.flush(); // Pastikan semua data terkirim
  }

  if (Serial2.available() >= 10) {
    byte data[10];
    Serial2.readBytes(data, 10);
    int syncAttempts = 0;
    while ((data[0] != 0x55 || data[1] != 0xAA) && syncAttempts < 10) {
       //Serial.println("Resynchronize");
      for (int i = 0; i < 9; i++) {
        data[i] = Serial2.read();
      }
      syncAttempts++;
    }
    if (data[0] == 0x55 && data[1] == 0xAA) {
      //Serial.println("Resynchronized complete!");
      parseAndSaveData(data);
    }
  }


   if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
  GetHumidityTemperature(HumiditySensorReading, TemperatureSensorReading);
  GetVoltage(VoltageSensorReading);
  GetTransducer(pressureValue);
  GetThermocouple(tc1, tc2, tc3, tc4, tc5, tc6, tc7, tc8);

  //GE;58fffefffeffffffff;20231226184800;24.00;34.60;47.00;0.00;
    //33.25;40.25;36.75;40.25;38.50;39.50;39.50;39.50;450;350;100;1;1;1;1;1;1;1;1;1;
    //100.20;100.20;100.20;100.20;100.20;100.20;100.20;100.20;1;1;0;1;1;0
    DataToESP = String("GE")+";"+UniqID+";"+yearStringUtc+monthStringUtc+dayStringUtc+hourStringUtc+minuteStringUtc+secondStringUtc+";"+String(VoltageSensorReading)+";"+String(TemperatureSensorReading)+";"+String(HumiditySensorReading)+";"+String(pressureValue)+";"+
                String(tc1)+";"+String(tc2)+";"+String(tc3)+";"+String(tc4)+";"+String(datatc)+";"+ 
                String("1;1;1;1;0;0;0;0;1;1;1;1")+";"+ 
                (pressure1)+";"+(pressure2)+";"+(pressure3)+";"+(pressure4)+";"+(pressure5)+";"+(pressure6)+";"+(pressure7)+";"+(pressure8)+";"+
                (statusRelayFT)+";"+String(comp_1)+";"+String(comp_2)+";"+String(comp_3)+";"+(statusSeatbelt1)+";"+(statusSeatbelt2);
    //delay(100);
    Serial.println(DataToESP);
    mySerial.println(DataToESP);
    //randomSeed(analogRead(A0));
    DataToLogger = yearString+monthString+dayString+hourString+minuteString+secondString+";"+
                String(VoltageSensorReading)+";"+String(TemperatureSensorReading)+";"+String(HumiditySensorReading)+";"+String(pressureValue)+";"+
                String(tc1)+";"+String(tc2)+";"+String(tc3)+";"+String(tc4)+";"+String(datatc)+";"+ 
                String("1;1;1;1;0;0;0;0;1;1;1;1")+";"+ 
                (pressure1)+";"+(pressure2)+";"+(pressure3)+";"+(pressure4)+";"+(pressure5)+";"+(pressure6)+";"+(pressure7)+";"+(pressure8)+";"+
                (statusRelayFT)+";"+String(comp_1)+";"+String(comp_2)+";"+String(comp_3)+";"+(statusSeatbelt1)+";"+(statusSeatbelt2)+";";
    //Serial.println(UniqID + ";" + DataToLogger);
    
    String fileName1 = "GE" + String(DateLogger) + ".txt";
    GeDataFile = SD.open(fileName1.c_str(), FILE_WRITE);
    if (GeDataFile) {
    GeDataFile.print(UniqID); // Tulis string uniq id ke dalam file
    GeDataFile.print(";");
    String encryptedData = enkripsi(DataToLogger); // Enkripsi data
    GeDataFile.println(encryptedData); // Tulis data yang dienkripsi ke dalam file
    GeDataFile.close();
    Serial.println("Data berhasil disimpan GE");
  } else {
    Serial.println("Gagal membuka file");}
  }

    
  if (FlagWriteFile == 1) {
    String fileName2 = "PE" + String(DateLogger) + ".txt";
    PenDataFile = SD.open(fileName2.c_str(), FILE_WRITE);
    PenBefEncrypt = yearString + monthString + dayString + hourString + minuteString + secondString+";"+String(comp_1)+";"+String(comp_2)+";"+String(comp_3); 
    // PenToESP = "PE;" + String(TimeResultutc) + String(comp_1)+";"+String(comp_2)+";"+String(comp_3) ;
    // mySerial.println(PenToESP);
    PenDataFile.print(UniqID); // Tulis string uniq id ke dalam file
    PenDataFile.print(";");
    String penData = enkripsi(PenBefEncrypt);
    PenDataFile.println(penData);
    PenDataFile.close();
    Serial.println("Data berhasil tersimpan PE");
    //Serial.println(TabletDataPenseal);
    //Serial3.println(TabletDataPenseal);
    delay(100);
    FlagWriteFile = 0;
    }
}

