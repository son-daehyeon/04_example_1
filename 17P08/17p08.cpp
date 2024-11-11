#include <Arduino.h>
#include <Servo.h>

#define PIN_IR    0
#define PIN_LED   9
#define PIN_SERVO 10

#define _DUTY_MIN 500
#define _DUTY_NEU 1750
#define _DUTY_MAX 3000

#define _DIST_MIN  100
#define _DIST_MAX  250

#define EMA_ALPHA 0.1
#define LOOP_INTERVAL 20

Servo myservo;
uint32_t last_loop_time;

float dist_prev = _DIST_MIN;
float ema_distance = _DIST_MIN;

void setup() {
    pinMode(PIN_LED, OUTPUT);

    myservo.attach(PIN_SERVO);
    myservo.writeMicroseconds(_DUTY_NEU);

    Serial.begin(1000000);
}

void loop() {
    uint32_t time_curr = millis();
    float ir_value, raw_distance;
    int duty;

    if (time_curr < (last_loop_time + LOOP_INTERVAL)) return;
    last_loop_time += LOOP_INTERVAL;

    ir_value = analogRead(PIN_IR);
    raw_distance = ((6762.0 / (ir_value - 9.0)) - 4.0) * 10.0;

    if (raw_distance < _DIST_MIN || raw_distance > _DIST_MAX) {
        digitalWrite(PIN_LED, HIGH);
    } else {
        digitalWrite(PIN_LED, LOW);
    }

    ema_distance = EMA_ALPHA * raw_distance + (1 - EMA_ALPHA) * ema_distance;

    duty = _DUTY_MIN + (ema_distance - _DIST_MIN) * (_DUTY_MAX - _DUTY_MIN) / (_DIST_MAX - _DIST_MIN);
    myservo.writeMicroseconds(duty);

    Serial.print("_DUTY_MIN:");  Serial.print(_DUTY_MIN);
    Serial.print(",_DIST_MIN:"); Serial.print(_DIST_MIN);
    Serial.print(",IR:");        Serial.print(ir_value);
    Serial.print(",raw_distance:");  Serial.print(raw_distance);
    Serial.print(",ema:");       Serial.print(ema_distance);
    Serial.print(",servo:");     Serial.print(duty);
    Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
    Serial.print(",_DUTY_MAX:"); Serial.println(_DUTY_MAX);
    Serial.println("");
}
