#include <WString.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <WideTextFinder.h>
#include <Streaming.h>
#include <LOG.h>
#include <inetGSM.h>
#include <HWSerial.h>
#include <GSM.h>
#include <gps.h>
#include <call.h>
#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"

//--------{ CONSTANS }-------------------------------------------
#define LED_TIMEOUT 1000
#define SERIAL_BAUDRATE 9600
#define STX "\x02"
#define ETX "\x03"
#define RS  "$"

#define SIM900_BAUDRATE 2400

//-------{ WARNING, ERROR AND STATUS CODE }------------------
#define MSG_METHOD_SUCCESS 0
#define WRG_NO_SERIAL_DATA_AVAILABLE 250
#define ERR_SERIAL_IN_COMMAND_NOT_TERMINATED -1

//--------{ KONFIGURASI SIM900A }-------------
SMSGSM sms;

//-------{ METHOD DECLARATIONS }---------------
int readSerialInputCommand(String *command);

//-------{ VARIABLE GLOBAL }------------
byte menu_pilihan = 255;

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial);

  //----------{ GSM MODULE INIT }--------
  if(gsm.begin(SIM900_BAUDRATE))
  {
    //ready
    Serial.print(STX);
    Serial.print("1");
    Serial.print(ETX);
  }else{
    
    Serial.print(STX);
    Serial.print("0");
    Serial.print(ETX);
    
  }

}

void loop() {
  String command = "";  //Used to store the latest received command
  byte serialResult = 0; //return value for reading operation method on serial in put buffer
  
  serialResult = readSerialInputCommand(&command);
  delay(500);
  if(serialResult == MSG_METHOD_SUCCESS){
    //char json[] = "{\"i\":\"151610005\",\"t\":\"081313309219\",\"k\":\"XI IPA B\",\"n\":\"Mukti Muhammad Ramdhan\"}";
    //{"t":"081313309219","n":"Mukti Muhammad Ramdhan"}#
    Serial.println("MSG_METHOD_SUCCESS");
    Serial.print("Jumlah Kata :");Serial.println(command.length());
    byte jlm_kata = command.length(); 
    command.remove(jlm_kata-1);
    Serial.print("Json :");Serial.println(command);
    
    StaticJsonBuffer<100> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(command);
    
    String nama = root["n"];
    Serial.println(nama);
    char number[20];
    String telepon = root["t"];
    Serial.println(telepon);
    telepon.toCharArray(number, 30);
    Serial.println(number);
    String bufferPesan = "Anak Anda Yang Bernama "+nama+" Masuk Sekolah \n\nSMAN1 CIMAHI";
    char pesan[120];
    bufferPesan.toCharArray(pesan, 150);
    
    Serial.println(pesan);
   
    if (sms.SendSMS(number, pesan)){
      Serial.println(STX);
      Serial.println("SMS sent OK");
      Serial.println(ETX);
    }else{
      Serial.println(STX);
      Serial.println("SMS Not sent OK");
      Serial.println(ETX);
    }
      
      
    /*
    byte jlm_kata = command.length(); 
    //buang #
    command.remove(jlm_kata-1);
    char number[20];
    command.toCharArray(number, 50);
    /*
    if (sms.SendSMS(number, "Arduino SMS"))
      Serial.println("\nSMS sent OK");
      */
  }
  
  if(serialResult == WRG_NO_SERIAL_DATA_AVAILABLE){//If there is no data AVAILABLE at the serial port, let the LED blink
    //Serial.println("WRG_NO_SERIAL_DATA_AVAILABLE");
    //Serial.println(serialResult);
  }
  else{
    if(serialResult == ERR_SERIAL_IN_COMMAND_NOT_TERMINATED){//If the command format was invalid, the led is turned off for two seconds
      //Serial.println("ERR_SERIAL_IN_COMMAND_NOT_TERMINATED");
      //Serial.println(serialResult);
    }
  }
}

int readSerialInputCommand(String *command){
  
  int operationStatus = MSG_METHOD_SUCCESS;//Default return is MSG_METHOD_SUCCESS reading data from com buffer.
  
  //check if serial data is available for reading
  if (Serial.available()) {
     char serialInByte;//temporary variable to hold the last serial input buffer character
     
     do{//Read serial input buffer data byte by byte 
       serialInByte = Serial.read();
       *command = *command + serialInByte;//Add last read serial input buffer byte to *command pointer
     }while(serialInByte != '#' && Serial.available());//until '#' comes up or no serial data is available anymore
     //Serial.println("Pointer");
     //Serial.println(serialInByte);
     if(serialInByte != '#') {
       operationStatus = ERR_SERIAL_IN_COMMAND_NOT_TERMINATED;
     }
  }
  else{//If not serial input buffer data is AVAILABLE, operationStatus becomes WRG_NO_SERIAL_DATA_AVAILABLE (= No data in the serial input buffer AVAILABLE)
    operationStatus = WRG_NO_SERIAL_DATA_AVAILABLE;
  }
  
  return operationStatus;
}
