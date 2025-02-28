#include <ESP32Servo.h>

Servo myServo;  // สร้างอ็อบเจ็กต์ Servo

void setup() {
    myServo.attach(25);  // กำหนดขาที่ใช้ควบคุมเซอร์โว (ขา 9 บน ESP32)
}

void loop() {
    unsigned long startTime = millis();
    while (millis() - startTime < 60000) { // ทำซ้ำเป็นเวลา 1 นาที
        myServo.write(90);  // หมุนไปที่ 90 องศา
        delay(5000);        // รอ 5 วินาที
        myServo.write(0);   // หมุนกลับไปที่ 0 องศา
        delay(5000);        // รอ 5 วินาที
    }
}
