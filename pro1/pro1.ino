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

// ตัวแปรสำหรับ debounce
int lastButtonState = HIGH; // สถานะก่อนหน้าของปุ่ม (HIGH = ไม่กด)
unsigned long lastDebounceTime = 0; // เวลากดปุ่มล่าสุด
const unsigned long debounceDelay = 50; // ดีเลย์กันการเด้งของปุ่ม (50ms)

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

    // หยุดจับเวลาและคำนวณความเร็ว (เฉพาะเมื่อจับเวลาอยู่)
    if (endSensor == LOW && timing) {
        endTime = millis();
        unsigned long totalTime = endTime - startTime; // เวลาทั้งหมด (ms)
        float timeInSeconds = totalTime / 1000.0; // แปลงเป็นวินาที

        // คำนวณความเร็ว V = s / t (s = 5 cm)
        float distance = 5.0; // ระยะทาง (cm)
        float velocity = distance / timeInSeconds; // ความเร็ว (cm/s)

        Serial.print("🏁 เวลาทั้งหมด: ");
        Serial.print(timeInSeconds);
        Serial.println(" วินาที");

        Serial.print("⚡ ความเร็ว: ");
        Serial.print(velocity);
        Serial.println(" cm/s");

        // คำนวณเวลาที่ไม้กั้นต้องใช้ในการลงจากสูตร s = v*t - (1/2) a * t^2
        float s = 30.0; // ระยะที่ไม้กั้นต้องลง (cm)
        float a = 9.81; // ความเร่งโน้มถ่วง (cm/s^2)
        
        // ใช้สมการ s = vt - (1/2)at^2
        // แปลงเป็นสมการกำลังสอง: 0.5 * a * t^2 - v * t + s = 0
        float t1, t2;
        float discriminant = (velocity * velocity) - (2 * a * s);

        if (discriminant >= 0) {
            t1 = (velocity - sqrt(discriminant)) / a;
            t2 = (velocity + sqrt(discriminant)) / a;

            // เลือกค่า t ที่เป็นบวก
            float servoTime = (t1 > 0) ? t1 : t2;

            // คำนวณเวลาหน่วงที่ชดเชยเวลาตอบสนองของเซอร์โว
            float delayTime = servoTime - servoResponseTime;
            if (delayTime < 0) delayTime = 0; // ป้องกันค่าติดลบ

            Serial.print("⏳ เวลาไม้กั้นลง: ");
            Serial.print(servoTime);
            Serial.println(" วินาที");

            Serial.print("⏳ หน่วงเวลาก่อนสับไม้กั้น: ");
            Serial.print(delayTime);
            Serial.println(" วินาที");

            // หน่วงเวลาก่อนให้ไม้กั้นลง (ชดเชยเวลาตอบสนองของเซอร์โว)
            delay(delayTime * 1000);

            // สับไม้กั้นลง
            myservo.write(90); // ปรับมุมให้ไม้กั้นลง
            Serial.println("🔻 ไม้กั้นลงแล้ว!");
        } else {
            Serial.println("⚠️ ค่าที่คำนวณไม่ได้ (ไม่มีรากจริง)");
        }

        timing = false;  // หยุดจับเวลา
        finished = true; // เซ็ตว่ารอบนี้เสร็จแล้ว
    }

    // กดปุ่ม → รีเซ็ตระบบเพื่อเริ่มรอบใหม่
    if (buttonState == LOW) {
        Serial.println("🔄 รีเซ็ตระบบ!");
        myservo.write(0);  // รีเซ็ตไม้กั้นกลับขึ้น
        finished = false; // พร้อมเริ่มรอบใหม่
        delay(300);  // ป้องกันการกดซ้ำเร็วเกินไป
    }
}
