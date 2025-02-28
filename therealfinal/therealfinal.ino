#include <WiFi.h>
#include <MQTT.h>
#include <ESP32Servo.h>

const char ssid[] = ".Aommmmm";
const char pass[] = "12345678.";
const char mqtt_broker[] = "test.mosquitto.org";
const char mqtt_topic[] = "group1.7/command";
const char mqtt_client_id[] = "arduino_group_1.7";
int MQTT_PORT = 1883;

#define IR_START_PIN 35
#define IR_END_PIN 27
#define SERVO_PIN 25
#define BUTTON_FORCE_PIN 17
#define BUTTON_RESET_PIN 16

WiFiClient net;
MQTTClient client;
Servo myservo;

String mode = "Bot";
bool timing = false;
bool finished = false;
unsigned long startTime;
unsigned long endTime;
const float servoResponseTime = 0.15;

float delayTime = 0;
bool servocheck = false;
unsigned long servoTime = 0;

void connect() {
    Serial.print("Checking WiFi...");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWiFi Connected!");

    Serial.print("Connecting to MQTT...");
    while (!client.connect(mqtt_client_id)) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nMQTT Connected!");
    client.subscribe(mqtt_topic);
}

void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
    if (payload == "Manual") {
        mode = "Manual";
        Serial.println("Mode switched to Manual");
    } else if (payload == "Bot") {
        mode = "Bot";
        Serial.println("Mode switched to Bot");
    } else if (payload.startsWith("servo:")) {
        int angle = payload.substring(6).toInt();
        myservo.write(angle);
        Serial.println("Servo moved to " + String(angle) + " degrees");
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(IR_START_PIN, INPUT);
    pinMode(IR_END_PIN, INPUT);
    pinMode(BUTTON_FORCE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_RESET_PIN, INPUT_PULLUP);
    myservo.attach(SERVO_PIN);
    myservo.write(0);
    client.begin(mqtt_broker, MQTT_PORT, net);
    client.onMessage(messageReceived);
    connect();
}

void loop() {
    client.loop();
    if (!client.connected()) {
        connect();
    }

    int buttonState = digitalRead(BUTTON_RESET_PIN);
    int startSensor = digitalRead(IR_START_PIN);
    int endSensor = digitalRead(IR_END_PIN);

    if (mode == "Bot") {
        if (startSensor == LOW && !timing && !finished) {
            startTime = millis();
            timing = true;
            Serial.println("🚀 เริ่มจับเวลา!");
        }
        
        if (endSensor == LOW && timing) {
            endTime = millis();
            float timeInSeconds = (endTime - startTime) / 1000.0;
            Serial.print("🏁 allTime: ");
            Serial.print(timeInSeconds);
            Serial.println(" sec");
            
            float distance = 2.5;
            float velocity = distance / timeInSeconds;
            Serial.print("⚡ Velocity: ");
            Serial.print(velocity);
            Serial.println(" cm/s");
            
            float s = 30;
            float a = 9.81 * sin(6.87 * PI / 180);
            float v = velocity;
            float A = 0.5 * a;
            float B = v;
            float C = -s;
            float discriminant = (B * B - 4 * A * C);
            float Timefind = -1;
            
            if (discriminant >= 0) {
                float t1 = (-B - sqrt(discriminant)) / (2 * A);
                float t2 = (-B + sqrt(discriminant)) / (2 * A);
                Timefind = (t1 > 0) ? t1 : t2;
                Serial.print("⏳ END Time: ");
                Serial.print(Timefind, 3);
                Serial.println(" sec");
            } else {
                Serial.println("❌ No real solution for time");
            }
            
            if (Timefind > 0) {
                delayTime = (Timefind - servoResponseTime) * 1000;
                if (delayTime < 0) delayTime = 0;
                servoTime = millis() + delayTime;
                servocheck = true;
                Serial.print("⏳ Servo will trigger in ");
                Serial.print(delayTime / 1000.0);
                Serial.println(" sec");
            }
            
            timing = false;
            finished = true;
        }
        
        if (servocheck && millis() >= servoTime) {
            myservo.write(90);
            Serial.println("🔻 Catch you!");
            servocheck = false;
        }
    } else if (mode == "Manual") {
        if (digitalRead(BUTTON_FORCE_PIN) == LOW) {
            myservo.write(90);
            Serial.println("🔻 Manual Trigger!");
            delay(300);
        }
    }
    
    if (buttonState == LOW) {
        Serial.println("🔄 Reset!");
        myservo.write(0);
        finished = false;
        servocheck = false;
        delay(300);
    }
}