#include <Arduino.h>
#line 1 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
#include<Keypad.h>
#include<LiquidCrystal.h>
#include<EEPROM.h>
#include<MFRC522.h>
#include<Servo.h>

/* EEPROM structure

0:5 password
6 level
*/

#define rows 4
#define cols 3
#define rs A0
#define e A1
#define D4 A2
#define D5 A3
#define D6 A4
#define D7 A5
#define SERVO_PIN 5

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

bool isLocked=false;
#define level EEPROM.read(6)
byte rowPins[]={0,1,2,4};
byte colPins[]={6,7,8};

char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
Keypad keypad=Keypad(makeKeymap(keys),rowPins,colPins,rows,cols);
LiquidCrystal lcd(rs,e,D4,D5,D6,D7);
Servo servo;

byte master_password[]={1,1,1,1,1,1,1,1};

#line 44 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void setup();
#line 57 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void loop();
#line 150 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void RESET();
#line 159 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
bool verifyPassword();
#line 189 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
bool verifyRFID();
#line 228 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void show_menu();
#line 399 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void update_password(byte passw[6]);
#line 405 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void update_level(byte lvl);
#line 409 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void unlock_door();
#line 415 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void lock_door();
#line 44 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
void setup() {
  // put your setup code here, to run once:
  lcd.begin(16,2);
  pinMode(3,OUTPUT);
  analogWrite(3,90);
  servo.attach(SERVO_PIN);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  RESET();

}

void loop() {
  // put your main code here, to run repeatedly:
  
  //Homescreen


  //If door is unlocked
  if(isLocked==false){
    char key;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Door unlocked");
    lcd.setCursor(0,1);
    lcd.print("[1]Menu [#]Lock");
    while(1){
      key=keypad.waitForKey();
      if(key=='1' || key=='#') break;
    }
    if(key=='#'){
      lock_door();
      return;
    }
    else if(key=='1'){
      show_menu();
      return;
    }
  }

  //In case door is locked with level 1 security
  if(level==1){
    if(verifyPassword()){
      lcd.clear();
      lcd.print("Access Granted!");
      delay(2000);
      unlock_door();
      return;
    }
    else{
      lcd.clear();
      lcd.print("Access Denied!");
      delay(2000);
      return;
    }
  }
  //If door is locked with level 0 security
  if(level==0){
    if(verifyRFID()){
      
      lcd.clear();
      lcd.print("Access Granted!");
      unlock_door();
      delay(1500);
      return;
    }
    else{
      lcd.clear();
      lcd.print("Access Denied!");
      delay(2000);
      return;
    }
  }

  if(level==2){
    if(verifyRFID()){
      
      lcd.clear();
      lcd.print("RFID accepted!");
      delay(2000);
      if(verifyPassword()){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Access Granted!");
        unlock_door();
        delay(1500);
        return;
      }
      else{
        lcd.clear();
        lcd.print("Access denied!");
        delay(2000);
        return;
      }
    }
    else{
      lcd.clear();
      lcd.print("Access Denied!");
      delay(2000);
      return;
    }
  }

}

void RESET(){
  byte password[]={1,1,1,1,1,1};
  //Reset password
  for(byte i=0;i<6;i++){
    EEPROM.write(i,password[i]);
  }
  //Reset level to level 1 (password only)
  update_level(2);
}
bool verifyPassword(){
  lcd.clear();
  lcd.print("Enter Password");
  lcd.setCursor(4,1);

  byte temp[6]={0};
  char key;
  for(byte i=0;i<6;){
    key=keypad.waitForKey();
    if(key!='*' && key!='#'){
      lcd.print('*');
      temp[i]=(byte)(key-'0');
      i++;
      while(keypad.getKey()==key);
    }
  }
  delay(500);
  lcd.clear();

  //Compare temp with password stored in eeprom
  for(byte i=0;i<6;i++){
    if(temp[i]!=EEPROM.read(i)){
      // lcd.print("Access Denied!");
      return false;
    }
  }
  // lcd.print("Access Granted!");
  return true;
}

bool verifyRFID(){

  lcd.clear();
  lcd.print("Tag RFID");

  byte uid[4];
  byte realUID[4]={0x57,0xF0,0xEC,0x4E};

  while (1)
  {
    if ( ! mfrc522.PICC_IsNewCardPresent()) 
    {
      continue;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      continue;
    }
    else{
      break;
    }
  }
    
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
      uid[i]=mfrc522.uid.uidByte[i];
    }

  
  
  for(byte i=0;i<4;i++){
    if(realUID[i]!=uid[i]){
      return false;
    }
  }
  return true;
}

void show_menu(){
  byte currentOption=0;
  char key;
  
  while(1){
    //Update Password option
    if(currentOption==0){
      lcd.clear();
      lcd.print("Update Password");
      lcd.setCursor(0,1);
      lcd.print("[*]< [0]o [#]>");
      while(1){
        key=keypad.waitForKey();
        if(key=='*'||key=='0'||key=='#') break;
      }
      if(key=='*') return;
      else if(key=='0'){
        bool match=verifyPassword();
        if(match){
          lcd.clear();
          lcd.print("Enter New Passw");

          lcd.setCursor(4,1);

          byte temp[6]={0};
          char key;
          for(byte i=0;i<6;){
            key=keypad.waitForKey();
            if(key!='*' && key!='#'){
              lcd.print('*');
              temp[i]=(byte)(key-'0');
              i++;
              while(keypad.getKey()==key);
            }
          }
          delay(200);
          update_password(temp);
          lcd.clear();
          lcd.print("Passw Updated!");
          delay(1000);
          continue;
        }
        else{
          lcd.clear();
          lcd.print("Wrong Password!");
          delay(1000);
          continue;
        }
      }
      else if(key=='#'){
        currentOption++;
        continue;
      }
    }
    //Change Level option
    else if(currentOption==1){
      lcd.clear();
      lcd.print("Change Level");
      lcd.setCursor(0,1);
      lcd.print("[*]< [0]o [#]>");
      while(1){
        key=keypad.waitForKey();
        if(key=='*'||key=='0'||key=='#') break;
      }
      if(key=='*'){
        currentOption--;
        continue;
      }
      else if(key=='0'){
        bool match=verifyPassword();
        if(match==false){
          lcd.clear();
          lcd.print("Wrong Password!");
          delay(1000);
          continue;
        }
        if(level==2){
          //Ask for rfid. If valid, then only allow to access this setting. (for level 2 both passw and rfid is req to change settings)
          match=verifyRFID();
          if(match==false){
            lcd.clear();
            lcd.print("Invalid RFID!");
            delay(2000);
            continue;
          }
        }
        //If password is correct
        lcd.clear();
        lcd.print("Select Level");
        lcd.setCursor(0,1);
        lcd.print("Level: 0/1/2");
        while(1){
          key=keypad.waitForKey();
          if(key=='0'||key=='1'||key=='2') break;
        }
        if(key=='0') update_level(0);
        else if(key=='1') update_level(1);
        else if(key=='2') update_level(2);
        lcd.clear();
        lcd.print("Success!");
        delay(2000);
        continue;
      }
      else if(key=='#'){
        currentOption++;
        continue;
      }
    }
    //Reset device
    else if(currentOption==2){
      lcd.clear();
      lcd.print("Factory Reset");
      lcd.setCursor(0,1);
      lcd.print("[*]< [0]o [#]>");
      while(1){
        key=keypad.waitForKey();
        if(key=='*'||key=='0'||key=='#') break;
      }
      if(key=='*'){
        currentOption--;
        continue;
      }
      else if(key=='0'){
        lcd.clear();
        lcd.print("Master passw:");

        lcd.setCursor(4,1);

        byte temp[8]={0};
        char key;
        for(byte i=0;i<8;){
          key=keypad.waitForKey();
          if(key!='*' && key!='#'){
            lcd.print('*');
            temp[i]=(byte)(key-'0');
            i++;
            while(keypad.getKey()==key);
            }
        }

        //Check if temp matches with master password
        bool match=true;
        for(byte i=0;i<8;i++){
          if(temp[i]!=master_password[i]){
            match=false;
            break;
          }
        }
        if(match==false){
          lcd.clear();
          lcd.print("Access Denied!");
          delay(2000);
          continue;
        }
        else{
          lcd.clear();
          RESET();
          lcd.print("Reset Success!");
          delay(2000);
          continue;
        }
      }
      else if(key=='#'){
        //currentOption++;
        continue;
      }
    }
  }
}


void update_password(byte passw[6]){
  for(byte i=0;i<6;i++){
    EEPROM.write(i,passw[i]);
  }
}

void update_level(byte lvl){
  EEPROM.write(6,lvl);
}

void unlock_door(){
  isLocked=false;
  servo.write(0);
  delay(500);
  return;
}
void lock_door(){
  isLocked=true;
  servo.write(180);
  delay(500);
  return;
}
