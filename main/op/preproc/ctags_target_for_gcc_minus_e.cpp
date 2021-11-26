# 1 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
# 2 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino" 2
# 3 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino" 2
# 4 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino" 2
# 5 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino" 2
# 6 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino" 2

/* EEPROM structure



0:5 password

6 level

*/
# 25 "c:\\Users\\omkar\\Documents\\Arduino\\Access Control With Dual Authentication\\main\\main.ino"
MFRC522 mfrc522(10, 9); // Create MFRC522 instance.

bool isLocked=false;

byte rowPins[]={0,1,2,4};
byte colPins[]={6,7,8};

char keys[4][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
Keypad keypad=Keypad(((char*)keys),rowPins,colPins,4,3);
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);
Servo servo;

byte master_password[]={1,1,1,1,1,1,1,1};

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16,2);
  pinMode(3,0x1);
  analogWrite(3,90);
  servo.attach(5);
  SPI.begin(); // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522

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
  if(EEPROM.read(6)==1){
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
  if(EEPROM.read(6)==0){
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

  if(EEPROM.read(6)==2){
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
        if(EEPROM.read(6)==2){
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
