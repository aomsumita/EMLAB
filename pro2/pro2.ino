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

    // หยุดจับเวลาและสับ Servo ลง (เฉพาะเมื่อจับเวลาอยู่)
    if (endSensor == LOW && timing) {
        endTime = millis();
        unsigned long totalTime = endTime - startTime;
        Serial.print("🏁 allTime: ");
        Serial.print(totalTime / 1000.0);
        Serial.println("sec");

        myservo.write(90);  // สับไม้กั้นลง
        timing = false;  // หยุดจับเวลา
        finished = true; // เซ็ตว่ารอบนี้เสร็จแล้ว
    }

    // กดปุ่ม → รีเซ็ตระบบเพื่อเริ่มรอบใหม่
    if (buttonState == LOW) {
        Serial.println("🔄 reset!");
        myservo.write(0);
        finished = false; // พร้อมเริ่มรอบใหม่
        delay(300);  // ป้องกันการกดซ้ำเร็วเกินไป
    }
}