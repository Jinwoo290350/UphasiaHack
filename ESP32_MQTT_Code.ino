
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

Servo myservo;// servo onject 
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "119.8.188.243";
const char* mqtt_username = ""; // Leave empty if not using authentication
const char* mqtt_password = ""; // Leave empty if not using authentication
const char* mqtt_topic = "Uphasia";

WiFiClient espClient;
PubSubClient client(espClient);

const int buttonPin = 18;  // Pin for button input
const int outputPin = 22;  // Pin for output control
int buttonState = 0;       // Variable to store button state
const int buzzerPin = 5; 

char inputBuffer[256];     // Input buffer for accumulating MQTT messages
int inputIndex = 0;        // Index to track buffer position

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Initialize LCD using I2C address 0x27

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
}

void addToBuffer(char* message, unsigned int length) {
  for (int i = 0; i < length; i++) {
    if (inputIndex < sizeof(inputBuffer) - 1) {
      inputBuffer[inputIndex++] = message[i];
    }
  }
  inputBuffer[inputIndex] = '\0'; // Null-terminate the buffer
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  char receivedMessage[100];
  // Clear the receivedMessage buffer before appending new data
  memset(receivedMessage, 0, sizeof(receivedMessage));

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    // Append each character to the receivedMessage buffer
    receivedMessage[i] = (char)message[i];
  }
  Serial.println(); // Print a newline after the message
  
  addToBuffer(receivedMessage, length); // Add message to input buffer
  

  // Check if the received message is "switchon" in the input buffer
  if (strstr(receivedMessage, "switchon") != nullptr) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hello");
  }
  // Check if the received message is "switchoff" in the input buffer
  else if (strstr(receivedMessage, "switchoff") != nullptr) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("World");
    tone(buzzerPin, 262, 2500);  // เล่นเสียงที่ความถี่ 262Hz นาน 2500 มิลลิวินาที
    delay(1000);
    int pos = 0;  
    myservo.write(pos); 
    delay(150);
    
  }
  
  memset(inputBuffer, 0, sizeof(inputBuffer)); // Clear the input buffer
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(buttonPin, INPUT);
  pinMode(outputPin, OUTPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  myservo.attach(2);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read button state
  buttonState = digitalRead(buttonPin);

  // Publish message if button is pressed
  if (buttonState == HIGH) {
    client.publish(mqtt_topic, "Button Pressed");
    Serial.println("Button Pressed, Message sent to MQTT server");
    delay(1000);  // Delay to avoid multiple rapid button presses
  }

  delay(100);  // Delay for stability
}
