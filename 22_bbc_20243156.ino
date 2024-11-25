#include <Servo.h>

#define PIN_LED   9
#define PIN_SERVO 10
#define PIN_IR    A0

#define _INTERVAL_DIST    20
#define _INTERVAL_SERVO   20
#define _INTERVAL_SERIAL  20

#define _EMA_ALPHA 0.1

#define _DUTY_MAX 2200
#define _DUTY_NEU (2200+800)/2
#define _DUTY_MIN 800

#define _SERVO_ANGLE_DIFF  25
#define _SERVO_SPEED       100

#define _BANGBANG_RANGE    400

#define _DIST_TARGET    150

Servo myservo;

float dist_ema;

int duty_change_per_interval;
int duty_target;
int duty_current;
int duty_adj;

unsigned long last_sampling_time_dist;
unsigned long last_sampling_time_servo;
unsigned long last_sampling_time_serial;

bool event_dist, event_servo, event_serial;

void setup()
{
  pinMode(PIN_LED,OUTPUT);
  myservo.attach(PIN_SERVO);

  duty_target = duty_current = _DUTY_NEU;
  myservo.writeMicroseconds(duty_current);

  Serial.begin(1000000);

  duty_change_per_interval =
    (float)(_DUTY_MAX - _DUTY_MIN) * ((float)_SERVO_SPEED / _SERVO_ANGLE_DIFF) * (_INTERVAL_SERVO / 1000.0);
}

void loop()
{
  unsigned long time_curr = millis();

  if (time_curr >= (last_sampling_time_dist + _INTERVAL_DIST)) {
        last_sampling_time_dist += _INTERVAL_DIST;
        event_dist = true;
  }
  if (time_curr >= (last_sampling_time_servo + _INTERVAL_SERVO)) {
        last_sampling_time_servo += _INTERVAL_SERVO;
        event_servo = true;
  }
  if (time_curr >= (last_sampling_time_serial + _INTERVAL_SERIAL)) {
        last_sampling_time_serial += _INTERVAL_SERIAL;
        event_serial = true;
  }

  if (event_dist) {
    float dist_filtered;
    event_dist = false;

    dist_filtered = volt_to_distance(ir_sensor_filtered(10, 0.5, 0));
    dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;

    if (dist_ema > _DIST_TARGET) {
      duty_target = _DUTY_NEU + _BANGBANG_RANGE;
      digitalWrite(PIN_LED, 0);
    } else if (dist_ema < _DIST_TARGET) {
      duty_target = _DUTY_NEU - _BANGBANG_RANGE;
      digitalWrite(PIN_LED, 1);
    }
  }

  if (event_servo) {
    event_servo = false;

    if (duty_target > duty_current) {
      duty_current += duty_change_per_interval;
      if (duty_current > duty_target)
          duty_current = duty_target;
    } else if (duty_target < duty_current) {
      duty_current -= duty_change_per_interval;
      if (duty_current < duty_target)
        duty_current = duty_target;
    }

    if (duty_current < _DUTY_MIN)
      duty_current = _DUTY_MIN;
    else if (duty_current > _DUTY_MAX)
      duty_current = _DUTY_MAX;

    myservo.writeMicroseconds(duty_current);
  }

  if (event_serial) {
    event_serial = false;

    Serial.print("TARGET:"); Serial.print(_DIST_TARGET);
    Serial.print(",DIST:"); Serial.print(dist_ema);
    Serial.print(",duty_target:"); Serial.print(duty_target);
    Serial.print(",duty_current:"); Serial.println(duty_current);
  }
}

float volt_to_distance(int a_value)
{
  return (545 - 1.78*a_value + 1.46*pow(10,-3)*pow(a_value,2));
}

int compare(const void *a, const void *b) {
  return (*(unsigned int *)a - *(unsigned int *)b);
}

unsigned int ir_sensor_filtered(unsigned int n, float position, int verbose)
{
  unsigned int *ir_val, ret_val;
  unsigned int start_time;

  if (verbose >= 2)
    start_time = millis();

  if ((n == 0) || (n > 100) || (position < 0.0) || (position > 1))
    return 0;

  if (position == 1.0)
    position = 0.999;

  if (verbose == 1) {
    Serial.print("n: "); Serial.print(n);
    Serial.print(", position: "); Serial.print(position);
    Serial.print(", ret_idx: ");  Serial.println((unsigned int)(n * position));
  }

  ir_val = (unsigned int *)malloc(sizeof(unsigned int) * n);
  if (ir_val == NULL)
    return 0;

  if (verbose == 1)
    Serial.print("IR:");

  for (int i = 0; i < n; i++) {
    ir_val[i] = analogRead(PIN_IR);
    if (verbose == 1) {
        Serial.print(" ");
        Serial.print(ir_val[i]);
    }
  }

  if (verbose == 1)
    Serial.print  ("  => ");

  qsort(ir_val, n, sizeof(unsigned int), compare);
  ret_val = ir_val[(unsigned int)(n * position)];

  if (verbose == 1) {
    for (int i = 0; i < n; i++) {
        Serial.print(" ");
        Serial.print(ir_val[i]);
    }
    Serial.print(" :: ");
    Serial.println(ret_val);
  }
  free(ir_val);

  if (verbose >= 2) {
    Serial.print("Elapsed time: "); Serial.print(millis() - start_time); Serial.println("ms");
  }

  return ret_val;
}