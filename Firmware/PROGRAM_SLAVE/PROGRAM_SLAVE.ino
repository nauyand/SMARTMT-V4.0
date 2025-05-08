#include "max6675.h"
#define SlaveEn 49
// Definisi pin untuk masing-masing thermocouple
int tc1_so = 42, tc1_cs = 38, tc1_sck = 43;
int tc2_so = 44, tc2_cs = 45, tc2_sck = 41;
int tc3_so = 36, tc3_cs = 37, tc3_sck = 40;
int tc4_so = 35, tc4_cs = 34, tc4_sck = 33;
// Inisialisasi sensor thermocouple
MAX6675 tc1(tc1_sck, tc1_cs, tc1_so);
MAX6675 tc2(tc2_sck, tc2_cs, tc2_so);
MAX6675 tc3(tc3_sck, tc3_cs, tc3_so);
MAX6675 tc4(tc4_sck, tc4_cs, tc4_so);

const int M1A = 8; //MOTOR 1 in
const int M1B = 9; //PWM 1 en
const int M2A = 6; //MOTOR 2 in 
const int M2B = 7; //PWM 2 en
const int M3A = 2; //MOTOR 3 in 
const int M3B = 3; //PWM 3 en
const int M4A = 4; //MOTOR 4 in 
const int M4B = 5; //PWM 4 en

void Pn1Close(){ //-24V
  delay(50);
  digitalWrite(M1B, HIGH);
  digitalWrite(M1A, LOW);
  Serial.println("panseal 1 tertutup");
  delay(1500);
  digitalWrite(M1B, LOW);
  delay(1500);
}
void Pn1Open(){ //+24V
  delay(50);
  digitalWrite(M1B, HIGH);
  digitalWrite(M1A, HIGH); 
  Serial.println("panseal 1 terbuka");
  delay(1500);
  digitalWrite(M1B, LOW);
  delay(1500);
}
void Pn2Close(){ //-24V
  delay(50);
  digitalWrite(M2B, HIGH);
  digitalWrite(M2A, LOW);
  Serial.println("panseal 2 tertutup");
  delay(1500);
  digitalWrite(M2B, LOW);
  delay(1500);
}
void Pn2Open(){ //+24V
  delay(50);
  digitalWrite(M2B, HIGH);
  digitalWrite(M2A, HIGH); 
  Serial.println("panseal 2 terbuka");
  delay(1500);
  digitalWrite(M2B, LOW);
  delay(1500);
}
void Pn3Close(){ //-24V
  delay(50);
  digitalWrite(M3B, HIGH);
  digitalWrite(M3A, HIGH); 
  Serial.println("panseal 3 tertutup");
  delay(1500);
  digitalWrite(M3B, LOW);
  delay(1500);
}
void Pn3Open(){ //+24V
  delay(50);
  digitalWrite(M3B, HIGH);
  digitalWrite(M3A, LOW);
  Serial.println("panseal 3 terbuka");
  delay(1500);
  digitalWrite(M3B, LOW);
  delay(1500);
}
void Pn4Close(){ //-24V
  delay(50);
  digitalWrite(M4B, HIGH);
  digitalWrite(M4A, HIGH); 
  Serial.println("panseal 4 tertutup");
  delay(1500);
  digitalWrite(M4B, LOW);
  delay(1500);
}
void Pn4Open(){ //+24V
  delay(50);
  digitalWrite(M4B, HIGH);
  digitalWrite(M4A, LOW);
  Serial.println("panseal 4 terbuka");
  delay(1500);
  digitalWrite(M4B, LOW);
  delay(1500);
}
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
void setup() {
  pinMode(SlaveEn, OUTPUT);
  digitalWrite(SlaveEn, LOW); // Default ke mode menerima
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);

}

////////// FOR DEBUGGING SENSORS ////////////

void loop(){
   SensorDebug();
   
  if (Serial1.available()) {
    String input = Serial1.readString(); // Membaca seluruh string yang dikirim
        String inputdata = "";
        for (char incomingChar : input) {
            // Cek apakah karakter termasuk yang diperbolehkan
            if ((incomingChar >= '0' && incomingChar <= '9') ||
                incomingChar == ',' || incomingChar == ';' || incomingChar == '#') {
                inputdata += incomingChar; // Menambahkan karakter yang valid ke string
            }
        }
        
        if (inputdata.length() > 0) {
            Serial.println(inputdata); // Menampilkan string yang sudah difilter
        }

    if (inputdata == "#1") {
      Pn1Close();
    } 
    else if (inputdata == "#2") {
      Pn1Open();
    }
    else if (inputdata == "#3") {
      Pn2Close();
    }
    else if (inputdata == "#4") {
      Pn2Open();
    }
    else if (inputdata == "#5") {
      Pn3Close();
    }
    else if (inputdata == "#6") {
      Pn3Open();
    }
    else if (inputdata == "#0") {
      Pn1Close();
      delay(500);
      Pn2Close();
      delay(500);
      Pn3Close();
      delay(500);
    }
    else if (inputdata == "#9") {
      Pn1Open();
      delay(500);
      Pn2Open();
      delay(500);
      Pn3Open();
      delay(500);
    }    
  }
  }

////////////////////////////////////////////
void SensorDebug(){
 
     unsigned long currentMillis = millis(); // Waktu saat ini
     static unsigned long previousMillis = 0;

      if (currentMillis - previousMillis >= 5000) {
    previousMillis = currentMillis; // Mengupdate waktu sebelumnya
    digitalWrite(SlaveEn, HIGH);
    delay(100);
    //Kirim data ke Serial2
  float temp1 = tc1.readCelsius();
  float temp2 = tc2.readCelsius();
  float temp3 = tc3.readCelsius();
  float temp4 = tc4.readCelsius();
  
 if (isnan(temp1)){
    temp1 = 0.00;
  }
  if(isnan(temp2)){
    temp2 = 0.00;
  }
  if(isnan(temp3)){
    temp3 = 0.00;
  }    
  if(isnan(temp4)){
    temp4 = 0.00;
  } 

  
  float tc1 = kalibrasiSuhu(temp1);   // Panggil fungsi kalibrasi
  float tc2 = kalibrasiSuhu(temp2);   // Panggil fungsi kalibrasi
  float tc3 = kalibrasiSuhu(temp3);   // Panggil fungsi kalibrasi
  float tc4 = kalibrasiSuhu(temp4);   // Panggil fungsi kalibrasi

  String dataResult = String(tc1)+";"+
                      String(tc2)+";"+
                      String(tc3)+";"+
                      String(tc4);
    Serial.println("Send data to master: "+ dataResult);
    Serial1.print(dataResult);
    Serial1.flush(); // Pastikan semua data terkirim
    Serial2.print(dataResult);
    delay(100);
    digitalWrite(SlaveEn, LOW);
  //  // delay(5000);
    }
  
  }