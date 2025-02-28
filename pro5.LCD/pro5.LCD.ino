#include <WiFi.h>
#include <MQTT.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char ssid[] = ".Aommmmm";
const char pass[] = "12345678.";
const char mqtt_broker[] = "test.mosquitto.org";
const char mqtt_topic[] = "group1.7/command";
const char mqtt_client_id[] = "arduino_group_1.7";
int MQTT_PORT = 1883;

#define LED_PIN 32
#define IR_START_PIN 35
#define IR_END_PIN 27
#define SERVO_PIN 25
#define BUTTON_PIN 16
#define BUTTON_FORCE_PIN 17

WiFiClient net;
MQTTClient client;
LiquidCrystal_I2C lcd(0x27, 16, 2); // ที่อยู่ 0x27, จอ LCD 16x2
Servo myservo;

String mode = "Bot";  // โหมดเริ่มต้น
bool timing = false;
bool finished = false;
unsigned long startTime, endTime;
const float servoResponseTime = 0.15;
float delayTime = 0;
bool servocheck = false;
unsigned long servoTime = 0;

void connect() {
    Serial.print("checking wifi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.print("\nconnecting...");
    while (!client.connect(mqtt_client_id)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nconnected!");
    client.subscribe(mqtt_topic);
}

void updateLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mode: " + mode);
}

void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
    if (payload == "on") {
        digitalWrite(LED_PIN, HIGH);
    } else if (payload == "off") {
        digitalWrite(LED_PIN, LOW);
    } else if (payload == "Manual") {
        mode = "Manual";
        Serial.println("Mode switched to Manual");
        updateLCD();  // อัปเดตจอ LCD
    } else if (payload == "Bot") {
        mode = "Bot";
        Serial.println("Mode switched to Bot");
        updateLCD();  // อัปเดตจอ LCD
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(IR_START_PIN, INPUT);
    pinMode(IR_END_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUTTON_FORCE_PIN, INPUT_PULLUP);
    
    myservo.attach(SERVO_PIN);
    myservo.write(0);
    
    lcd.begin();
    lcd.backlight();
    updateLCD();  // แสดงโหมดเริ่มต้น

    WiFi.begin(ssid, pass);
    client.begin(mqtt_broker, MQTT_PORT, net);
    client.onMessage(messageReceived);
    connect();
}

void loop() {
    client.loop();
    if (!client.connected()) {
        connect();
    }

    if (mode == "Bot") {
        int startSensor = digitalRead(IR_START_PIN);
        int endSensor = digitalRead(IR_END_PIN);
        if (startSensor == LOW && !timing && !finished) {
            startTime = millis();
            timing = true;
        }
        if (endSensor == LOW && timing) {
            endTime = millis();
            float timeInSeconds = (endTime - startTime) / 1000.0;
            float velocity = 2.0 / timeInSeconds;
            float s = 36.5;
            float a = 9.81 * sin(9 * PI / 180);
            float Timefind = (-velocity + sqrt(velocity * velocity + 2 * a * s)) / a;
            if (Timefind > 0) {
                delayTime = max(0, (Timefind - servoResponseTime) * 1000);
                servoTime = millis() + delayTime;
                servocheck = true;
            }
            timing = false;
            finished = true;
        }
        if (servocheck && millis() >= servoTime) {
            myservo.write(90);
            servocheck = false;
        }
    } else if (mode == "Manual") {
        if (digitalRead(BUTTON_PIN) == LOW) {
            myservo.write(90);
            Serial.println("Servo triggered manually");
            delay(300);
        }
    }
    
    if (digitalRead(BUTTON_FORCE_PIN) == LOW) {
        myservo.write(0);
        finished = false;
        servocheck = false;
        Serial.println("System reset");
        mode = "Bot"; // รีเซ็ตกลับไปโหมด Bot
        updateLCD();  // อัปเดตจอ LCD
        delay(300);
    }
}
