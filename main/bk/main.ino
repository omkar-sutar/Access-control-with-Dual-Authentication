#include<Keypad.h>
#include<LiquidCrystal.h>

#define rows 4
#define cols 3

byte rowPins[]={2,3,4,5};
byte colPins[]={6,7,8};

char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
Keypad keypad=Keypad(makeKeymap(keys),rowPins,colPins,rows,cols);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  char key=keypad.waitForKey();
  Serial.println(key);

}
