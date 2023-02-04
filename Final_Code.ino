#include <Keypad.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define limit_sw 1
#define led 4
#define relay 5
#define push_sw 2
#define ChangeDelay 3

LiquidCrystal_I2C lcd(0x27, 16, 4);

int tmp_epf=0;
int tmp_index=0;
int epf_no[]={ 1000, 1010, 1020, 1030, 1040, 1050};
String names[]={ "Kasun", "Shavin", "Vasula", "Asiri", "Kisal", "Nalin"};
volatile int buttonFlag;
volatile int delayFlag;
volatile int keyFlag;
float turnOffTime= 5000;
unsigned long time_now = 0;

int get_index(int epf){
int n=sizeof(epf_no);
for(int i=0; i<n; i++){
tmp_epf=epf_no[i];
if(tmp_epf==epf){
tmp_index=i; }}
return tmp_index; }

String get_name(int epf){
int index;
String tmp_name;
index=get_index(epf);
tmp_name=names[index];
return tmp_name; }

void display(){
lcd.setCursor(0,0);
lcd.print("Last User: ");
lcd.setCursor(0,1);
lcd.print("");
lcd.setCursor (-4,2);
lcd.print("Enter EPF:"); }

float MSToMin(int num){
return num*1000; }

void change_delay(float new_time){
turnOffTime= MSToMin(new_time);
}

void clearLine(int lineNum){
lcd.setCursor(-4,lineNum);
lcd.print("                ");
}

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {13, 12, 11, 10};
byte colPins[COLS] = {9, 8, 7, 6};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
String inputString="";
long inputInt=0;
int i=6;
int n=7;
int j=0;

boolean checkEpfNo(int epf){
boolean found=false;
for(int i=0;i<((sizeof(epf_no)/sizeof(int)));i++){
if(epf_no[i] == epf){
found=true;
}
}
return found;
}

int keypadEntry(int line){
char key = customKeypad.getKey();
if (key){
i++;
if(key>='0' && key<='9' && inputString.length()<4){
inputString+=key;
lcd.setCursor(i,line);
lcd.print(key);
}else if (key =='#'){
if(inputString.length()>0){
inputInt=inputString.toInt();
inputString ="";
lcd.setCursor(7,line);
lcd.print("     ");
i=6;
}
}else if(key=='*'){
inputString="";
lcd.setCursor(7,line);
lcd.print("     ");
i=6;
}
}
return inputInt;
}

void WriteLong(int address, long number){
EEPROM.write(address, (number >>24) & 0xFF);
EEPROM.write(address + 1, (number >>16) & 0xFF);
EEPROM.write(address + 2, (number >>8) & 0xFF);
EEPROM.write(address + 3, number & 0xFF);
}

long ReadLong(int address){
return ((long)EEPROM.read(address)<<24) + ((long)EEPROM.read(address +1)<<16) +
((long)EEPROM.read(address +2)<<8) + (long)EEPROM.read(address + 3);
}

long prevEpfNo;

void setup() {
pinMode(limit_sw, INPUT);
pinMode(led, OUTPUT);
pinMode(relay, OUTPUT);
digitalWrite(led, LOW);
digitalWrite(relay,LOW);
attachInterrupt(digitalPinToInterrupt(push_sw),ISR_Button, FALLING);
attachInterrupt(digitalPinToInterrupt(ChangeDelay),newDelay, FALLING);
lcd.init();
lcd.backlight();
}

void loop() {
display();

if(delayFlag==1){
lcd.setCursor(-4,3);
lcd.print( "New Delay:");
keyFlag=0;
while(keyFlag==0){
char key = customKeypad.getKey();
if (key){
n++;
if(key>='0' && key<='9' && inputString.length()<2){
inputString+=key;
lcd.setCursor(n,3);
lcd.print(key);
}else if (key =='#'){
if(inputString.length()>0){
inputInt=inputString.toInt();
turnOffTime=MSToMin(inputInt);
inputString ="";
clearLine(3);
n=15;
delayFlag=0;
keyFlag=1;
}
}else if(key=='*'){
inputString="";
clearLine(3);
n=15;
delayFlag=0;
keyFlag=1; }}
}
}else{
inputInt=keypadEntry(2); }

if(inputInt ==0){
prevEpfNo= ReadLong(10);
lcd.setCursor(10,0);
lcd.print("      ");
lcd.setCursor(10,0);
lcd.print( get_name(prevEpfNo)); }

if(checkEpfNo(inputInt)){
WriteLong(10,inputInt);
lcd.setCursor(10,0);
lcd.print("      ");
lcd.setCursor(10,0);
lcd.print( get_name(inputInt));

int limitsw_read=digitalRead(limit_sw);

if (limitsw_read== 1){
digitalWrite(relay,HIGH);
digitalWrite(led, HIGH);
j=0;
}else if(limitsw_read ==0){
j++;
if(j==1){
time_now=millis();
}else{
if(millis()>= turnOffTime+time_now){
digitalWrite(relay,LOW);
digitalWrite(led, LOW);
inputInt=0;
}
}
}
}else{
digitalWrite(led, LOW);
digitalWrite(relay,LOW);
}
 
if(buttonFlag==1){
digitalWrite(relay,LOW);
digitalWrite(led, LOW);
inputInt=1;
buttonFlag=0;
}
}

void ISR_Button(){
buttonFlag=1;
}

void newDelay(){
delayFlag=1;
}
