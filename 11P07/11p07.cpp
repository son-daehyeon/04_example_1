#include <Arduino.h>
#include <Servo.h>

Servo myServo;

#define PIN_LED  9
#define PIN_SERVO 10
#define PIN_TRIG 12
#define PIN_ECHO 13

#define SND_VEL 346.0
#define INTERVAL 50
#define PULSE_DURATION 10
#define _DIST_MIN 180.0
#define _DIST_MAX 360.0

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

float alpha = 0.1;
float dist_ema = 0.0;

unsigned long last_sampling_time;

float USS_measure(int, int);

void setup() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    myServo.attach(PIN_SERVO);

    digitalWrite(PIN_TRIG, LOW);

    Serial.begin(115200);
}

void loop() {
    if (millis() < (last_sampling_time + INTERVAL)) return;

    float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

    // EMA 적용
    dist_ema = (alpha * dist_raw) + ((1 - alpha) * dist_ema);

    // 서보와 LED 제어
    if ((dist_ema == 0.0) || (dist_ema > _DIST_MAX)) {
        dist_ema = _DIST_MAX + 10.0;
        digitalWrite(PIN_LED, LOW);
        myServo.write(180);
    } else if (dist_ema < _DIST_MIN) {
        dist_ema = _DIST_MIN - 10.0;
        digitalWrite(PIN_LED, LOW);
        myServo.write(0);
    } else {
        myServo.write(dist_ema - 180);
        digitalWrite(PIN_LED, HIGH);
    }

    Serial.print("Min:");        Serial.print(_DIST_MIN);
    Serial.print(",dist:");      Serial.print(dist_raw);
    Serial.print(",ema:");       Serial.print(dist_ema);
    Serial.print(",Servo:");     Serial.print(myServo.read());
    Serial.print(",Max:");       Serial.print(_DIST_MAX);
    Serial.println("");

    last_sampling_time += INTERVAL;
}

float USS_measure(int TRIG, int ECHO) {
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(PULSE_DURATION);
    digitalWrite(TRIG, LOW);

    return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}