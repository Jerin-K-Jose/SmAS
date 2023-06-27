#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include<DHT.h>
#include <LiquidCrystal_I2C.h>

int lcdColumns = 16;
int lcdRows = 2;
bool functionExecuted = false;
const int sensor_pin = A0;
const int pirPin = 12; 
String messageStatic = "SMAS";
String messageToScroll = "Farming through Technology";
String pump_status = "";

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
DHT dht(26, DHT11);

// WiFi credentials
char auth[] = "UPosKYIbfjiyCjz4peG7tSg4TlTp1D9F";
char ssid[] = "iPhone";
char password[] = "9895524301";

void setup()
{
  Serial.begin(115200);
  lcd.init();     
  lcd.backlight();
  
  WiFi.begin(ssid, password);
  lcd.clear(); lcd.setCursor(0,0);
  lcd.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
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
  pinMode(pirPin, INPUT);
  delay(500);
}

void loop()
{
  // Run Blynk background tasks
  Blynk.run();
  
  int temp = dht.readTemperature();
  int humidity = dht.readHumidity();
  delay(1000);
  int sensor_analog = analogRead(sensor_pin);
  int moisture = ( 100 - ( (sensor_analog/4095.000) * 100 ) );
  disp(temp, humidity, moisture, pump_status);
  Serial.print("Temp: "); Serial.print(temp); Serial.print(" c"); Serial.print("  Humidity: "); Serial.print(humidity); Serial.print("%"); 
  Serial.print("  Moisture: "); Serial.print(moisture); Serial.print("%"); Serial.println();
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, moisture);
  if(pump_status == "ON")
    Blynk.virtualWrite(V4, 1);
  else
    Blynk.virtualWrite(V4, 0);
  int pirValue = digitalRead(pirPin); 

  if (pirValue == HIGH) {
    Serial.println("Motion detected!");
  }
  else{
    Serial.println("NO Motion");
  }
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
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T:"); lcd.print(temp); lcd.print("C");
  lcd.print(" H:"); lcd.print(humidity); lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print("M:"); lcd.print(moisture); lcd.print("%");
  lcd.print(" P:"); lcd.print(pump_status);
}

BLYNK_WRITE(V3){
  int value = param.asInt();
  if(value)
    pump_status = "ON";
  else
    pump_status = "OFF";
}
