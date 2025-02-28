#include <ESP32Servo.h>

#define IR_START_PIN 35  // IR Sensor จุดเริ่มต้น
#define IR_END_PIN 27    // IR Sensor จุดปลาย
#define SERVO_PIN 25     // ขาเชื่อมต่อ Servo
#define BUTTON_PIN 16    // ปุ่มกดรีเซ็ต Servo

Servo myservo;
bool timing = false;      // สถานะการจับเวลา
bool finished = false;    // จบรอบแล้วหรือยัง
unsigned long startTime;  // เวลาที่เริ่มนับ
unsigned long endTime;    // เวลาที่สิ้นสุด
const float servoResponseTime = 0.15; 

float delayTime = 0;      // เก็บค่าที่ต้องใช้หน่วงเวลาก่อนเซอร์โวทำงาน
bool servocheck = false;  // ตรวจสอบว่าเซอร์โวทำงานไปหรือยัง
unsigned long servoTime = 0; // เวลาที่ต้องสั่งเซอร์โวทำงาน

void setup() {
    Serial.begin(115200);
    pinMode(IR_START_PIN, INPUT);
    pinMode(IR_END_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    myservo.attach(SERVO_PIN);
    myservo.write(0);  // เริ่มต้นที่ 0 องศา
}

void loop() {
    int startSensor = digitalRead(IR_START_PIN);
    int endSensor = digitalRead(IR_END_PIN);
    int buttonState = digitalRead(BUTTON_PIN);

    // เริ่มจับเวลา (เฉพาะเมื่อยังไม่ได้เริ่มมาก่อน)
    if (startSensor == LOW && !timing && !finished) {
        startTime = millis();
        timing = true;
        Serial.println("🚀 เริ่มจับเวลา!");
    }

    // หยุดจับเวลาและคำนวณเวลา (เฉพาะเมื่อจับเวลาอยู่)
    if (endSensor == LOW && timing) {
        endTime = millis();
        unsigned long totalTime = endTime - startTime;
        float timeInSeconds = totalTime / 1000.0;
        Serial.print("🏁 allTime: "); 
        Serial.print(timeInSeconds);
        Serial.println(" sec");

        // คำนวณความเร็ว
        float distance = 5.0; // ระยะทางระหว่างเซ็นเซอร์
        float velocity = distance / timeInSeconds;
        Serial.print("⚡ Velocity: ");
        Serial.print(velocity);
        Serial.println(" cm/s");

        // คำนวณเวลา Timefind
        float s = 30.0;  // ระยะทางปลายทาง
        float a = 9.81 * sin(9 * PI / 180); // แรงโน้มถ่วงตามมุม
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
            Serial.print(Timefind, 3);  // แสดงทศนิยม 3 ตำแหน่ง
            Serial.println(" sec");
        } else {
            Serial.println("❌ No real solution for time");
        }

        // ตรวจสอบค่า Timefind ก่อนใช้
        if (Timefind > 0) {
            // คำนวณเวลาหน่วงก่อนสับไม้กั้น
            delayTime = (Timefind - servoResponseTime) * 1000;
            if (delayTime < 0) delayTime = 0;

            // ตั้งเวลาให้เซอร์โวทำงานในอนาคต
            servoTime = millis() + delayTime;
            servocheck = true;
            Serial.print("⏳ Servo will trigger in ");
            Serial.print(delayTime / 1000.0);
            Serial.println(" sec");
        } else {
            Serial.println("⚠️ Invalid Timefind, skipping servo trigger!");
        }

        // หยุดจับเวลา
        timing = false;
        finished = true;
    }

    // ตรวจสอบว่าเวลาถึงหรือยังก่อนสั่งเซอร์โว
    if (servocheck && millis() >= servoTime) {
        myservo.write(90);  // สับไม้กั้นลง
        Serial.println("🔻 Catch you!");
        servocheck = false;
    }

    // กดปุ่ม → รีเซ็ตระบบเพื่อเริ่มรอบใหม่
    if (buttonState == LOW) {
        Serial.println("🔄 Reset!");
        myservo.write(0);
        finished = false;
        servocheck = false;
        delay(300);  // ป้องกันการกดซ้ำเร็วเกินไป
    }
}