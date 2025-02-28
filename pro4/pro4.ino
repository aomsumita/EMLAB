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
#define BUTTON_FORCE_PIN 22
#define BUTTON_RESET_PIN 16

WiFiClient net;
MQTTClient client;
Servo myservo;

String mode = "Bot";
bool timing = false;
bool finished = false;
unsigned long startTime, endTime;
const float servoResponseTime = 0.15;

float delayTime = 0;
bool servocheck = false;
unsigned long servoTime = 0;

void connect() {
    Serial.print("Checking WiFi...");
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        Serial.print(".");
        delay(500);
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
    } else {
        Serial.println("\nWiFi Connection Failed!");
        return;
    }

    Serial.print("Connecting to MQTT...");
    startAttemptTime = millis();
    while (!client.connect(mqtt_client_id) && millis() - startAttemptTime < 5000) {
        Serial.print(".");
        delay(500);
    }
    if (client.connected()) {
        Serial.println("\nMQTT Connected!");
        client.subscribe(mqtt_topic);
    } else {
        Serial.println("\nMQTT Connection Failed!");
    }
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
        if (angle >= 0 && angle <= 180) {
            myservo.write(angle);
            Serial.println("Servo moved to " + String(angle) + " degrees");
        } else {
            Serial.println("Invalid servo angle: " + String(angle));
        }
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
    
    static unsigned long lastResetTime = 0;
    static bool lastButtonState = HIGH;
    int buttonState = digitalRead(BUTTON_RESET_PIN);
    
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
            float velocity = 2.5 / timeInSeconds;
            float s = 20.0;
            float a = 9.81 * sin(9 * PI / 180);
            float Timefind = (-velocity + sqrt(velocity * velocity + 2 * a * s)) / a;
            if (Timefind > 0) {
                delayTime = max(0.0f, (Timefind - servoResponseTime) * 1000);
                servoTime = millis() + delayTime;
                servocheck = true;
            }
        }
        if (servocheck && millis() >= servoTime) {
            myservo.write(90);
            servocheck = false;
            finished = true;
        }
    } else if (mode == "Manual") {
        if (digitalRead(BUTTON_FORCE_PIN) == LOW) {
            myservo.write(90);
            Serial.println("Servo triggered manually");
            delay(300);
        }
    }
    
    if (buttonState == LOW && lastButtonState == HIGH && millis() - lastResetTime > 500) { 
        lastResetTime = millis();
        Serial.println("🔄 Reset!");
        myservo.write(0);
        timing = false;
        finished = false;
        servocheck = false;
    }
    lastButtonState = buttonState;
}