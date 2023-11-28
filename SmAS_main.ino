#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include<DHT.h>
#include <LiquidCrystal_I2C.h>

int lcdColumns = 16;
int lcdRows = 2;
bool functionExecuted = false;

const int sensor_pin = A0;
const int pirPin = 27; 
const int dhtPin = 14;
const int pumpPin = 32;
const int pwm1 = 12;
const int pwm2 = 13;
String messageStatic = "SMAS";
String messageToScroll = "Farming through Technology";
String pump_status = "";
int pump_flag = 0;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
DHT dht(dhtPin, DHT11);

// WiFi credentials
char auth[] = "";
char ssid[] = "";
char password[] = "";

void setup()
{
  //Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(pwm1, OUTPUT);
  pinMode(pwm2, OUTPUT);
  digitalWrite(pumpPin, LOW);
  delay(500);
  lcd.init();     
  lcd.backlight();
  
  WiFi.begin(ssid, password);
  lcd.clear(); lcd.setCursor(0,0);
  lcd.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    //Serial.println("Connecting to WiFi...");
  }
  //Serial.println("Connected to WiFi!");
  lcd.clear(); lcd.setCursor(0,0);
  lcd.print("Connected!");
  delay(1000);
  lcd.clear();
  
  // Connect to Blynk server
  Blynk.begin(auth, ssid, password);
  Blynk.syncVirtual(V3);
  dht.begin();

  lcd.setCursor(0, 0);
  lcd.print("SMAS");
  scrollText(messageToScroll, 300, lcdColumns);
  
  ledcAttachPin(pwm1,0); ledcAttachPin(pwm2,1);
  ledcSetup(0, 50, 8); ledcSetup(1, 50, 8);
  ledcWrite(0,128); ledcWrite(1, 128);
}

void loop()
{
  // Run Blynk background tasks
  Blynk.run();
  delay(10);
  int temp = dht.readTemperature();
  int humidity = dht.readHumidity();
  delay(1000);
  int sensor_analog = analogRead(sensor_pin);
  int moisture = map(sensor_analog,4095,850,0,100);
  delay(100);
  disp(temp, humidity, moisture, pump_status);
  int pirValue = digitalRead(pirPin); 
//  Serial.print("Temp: "); Serial.print(temp); Serial.print(" c"); Serial.print("  Humidity: "); Serial.print(humidity); Serial.print("%"); 
//  Serial.print("  Moisture: "); Serial.print(moisture); Serial.print("%"); Serial.println();
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, moisture);
  if(pump_flag == 0){
    if(moisture < 50 && moisture > 10){
    digitalWrite(pumpPin, HIGH);
    pump_status = "ON";
    delay(10);
  }
  else{
    digitalWrite(pumpPin, LOW);
    pump_status = "OFF";
    delay(10);
  }
  }
  delay(10);
  if(pump_status == "ON")
    Blynk.virtualWrite(V4, 1);
  else
    Blynk.virtualWrite(V4, 0);
  if (pirValue == HIGH) {
    Serial.println("Motion detected!");
    Blynk.virtualWrite(V5,1);
  }
  else{
    Serial.println("NO Motion");
    Blynk.virtualWrite(V5, 0);
  }
  delay(100);
}

// Function to scroll text
void scrollText(String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, 1);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
  functionExecuted = true;
}

void disp(int temp, int humidity, int moisture, String pump_status){
  delay(100);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T:"); delay(10); lcd.print(temp); delay(10); lcd.print("C");
  delay(10);
  lcd.print(" H:"); delay(10); lcd.print(humidity); delay(10); lcd.print("%");
  delay(10);
  lcd.setCursor(0,1);
  lcd.print("M:"); delay(10); lcd.print(moisture); delay(10); lcd.print("%");
  delay(10);
  lcd.print(" P:"); delay(10); lcd.print(pump_status);
}

BLYNK_WRITE(V3){
  int value = param.asInt();
  if(value){
    delay(10);
    pump_status = "ON";
    digitalWrite(pumpPin, HIGH);
    pump_flag = 1;
    delay(10);
  }
  else{
    delay(10);
    pump_status = "OFF";
    digitalWrite(pumpPin, LOW);
    pump_flag = 0;
    delay(10);
  }
}
