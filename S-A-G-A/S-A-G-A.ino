#define BLYNK_TEMPLATE_ID "TMPL6gB3c0_RQ"
#define BLYNK_TEMPLATE_NAME "penggerak servo"
#define BLYNK_AUTH_TOKEN "LSPKUZ_qlj_Xe8sNLsDHf2kPmEyuAF6c"

#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_PWMServoDriver.h>

char ssid[] = "alvarooooo";
char pass[] = "99999999";

WidgetTerminal terminal(V10);

Adafruit_PWMServoDriver pwm(0x40);

#define TOTAL_SERVO 16
constexpr uint8_t servo[TOTAL_SERVO] = {
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 9,10,11,12,13,14,15
};

#define SERVO_CW    395
#define SERVO_CCW  235
#define SERVO_STOP 315

#define MOVE_TIME_180 1200
#define MOVE_TIME_90   600
#define GAP_TIME      1200

class ServoController {
public:
  String name;
  uint8_t pin;
  uint16_t cwPWM;
  uint16_t ccwPWM;
  uint16_t stopPWM;
  uint16_t rotationTime;
  Adafruit_PWMServoDriver* pwmDriver;

  ServoController(String n, uint8_t p, uint16_t cw, uint16_t ccw,
                  uint16_t stop, uint16_t rotTime, Adafruit_PWMServoDriver* driver) {
    name = n;
    pin = p;
    cwPWM = cw;
    ccwPWM = ccw;
    stopPWM = stop;
    rotationTime = rotTime;
    pwmDriver = driver;
  }

  void moveCW() {
    pwmDriver->setPWM(pin, 0, cwPWM);
  }

  void moveCCW() {
    pwmDriver->setPWM(pin, 0, ccwPWM);
  }

  void stop() {
    pwmDriver->setPWM(pin, 0, stopPWM);
  }

  void rotate(bool clockwise) {
    if (clockwise) moveCW();
    else moveCCW();
    delay(rotationTime);
    stop();
  }
};

ServoController* servo0 = nullptr;
ServoController* servo1 = nullptr;
ServoController* servo2 = nullptr;
ServoController* servo3 = nullptr;
ServoController* servo4 = nullptr;
ServoController* servo5 = nullptr;
ServoController* servo6 = nullptr;
ServoController* servo7 = nullptr;
ServoController* servo8 = nullptr;
ServoController* servo9 = nullptr;

bool dirBtn1 = false;
bool dirBtn2 = false;
bool dirBtn3 = false;

void configureServo0() {
  servo0 = new ServoController("Servo 0", 0, SERVO_CW, SERVO_CCW, SERVO_STOP, MOVE_TIME_180, &pwm);
}

void configureServo1() {
  servo1 = new ServoController("Servo 1", 1, SERVO_CW, SERVO_CCW, SERVO_STOP, MOVE_TIME_90, &pwm);
}

void configureServo2() {
  servo2 = new ServoController("Servo 2", 2, SERVO_CW, SERVO_CCW, SERVO_STOP, MOVE_TIME_180, &pwm);
}

void configureServo3() {
  servo3 = new ServoController("Servo 3", 3, SERVO_CW, SERVO_CCW, SERVO_STOP, MOVE_TIME_90, &pwm);
}

void configureServo4() {
  servo4 = new ServoController("Servo 4", 4, 405, SERVO_CCW, SERVO_STOP, 1300, &pwm);
}

void configureServo5() {
  servo5 = new ServoController("Servo 5", 5, SERVO_CW, SERVO_CCW, SERVO_STOP, MOVE_TIME_90, &pwm);
}

void configureServo6() {
  servo6 = new ServoController("Servo 6", 6, 405, SERVO_CCW, SERVO_STOP, 1300, &pwm);
}

void configureServo7() {
  servo7 = new ServoController("Servo 7", 7, SERVO_CW, SERVO_CCW, SERVO_STOP, MOVE_TIME_90, &pwm);
}

void configureServo8() {
  servo8 = new ServoController("Servo 8", 8, 425, 235, 315, 1375, &pwm);
}

void configureServo9() {
  servo9 = new ServoController("Servo 9", 9, 415, 235, 315, 1400, &pwm);
}

void stopAllServo() {
  if (servo0) servo0->stop();
  if (servo1) servo1->stop();
  if (servo2) servo2->stop();
  if (servo3) servo3->stop();
  if (servo4) servo4->stop();
  if (servo5) servo5->stop();
  if (servo6) servo6->stop();
  if (servo7) servo7->stop();
  if (servo8) servo8->stop();
  if (servo9) servo9->stop();
}

void moveGroupOddEvenOrder(int start, int end, bool up) {

  if (start < 0 || end >= 10 || start > end) return;

  ServoController* servoArray[10] = {
    servo0, servo1, servo2, servo3, servo4,
    servo5, servo6, servo7, servo8, servo9
  };

  if (up) {

    for (int i = start; i <= end; i++) {
      if (i % 2 == 0 && servoArray[i]) {
        servoArray[i]->rotate(true);
        delay(GAP_TIME);
      }
    }

    for (int i = start; i <= end; i++) {
      if (i % 2 == 1 && servoArray[i]) {
        servoArray[i]->rotate(false);
        delay(GAP_TIME);
      }
    }
  }

  else {

    for (int i = start; i <= end; i++) {
      if (i % 2 == 1 && servoArray[i]) {
        servoArray[i]->rotate(true);
        delay(GAP_TIME);
      }
    }

    for (int i = start; i <= end; i++) {
      if (i % 2 == 0 && servoArray[i]) {
        servoArray[i]->rotate(false);
        delay(GAP_TIME);
      }
    }
  }
}

void moveGroup8_9_Toggle180(bool up) {

  if (!servo8 || !servo9) return;

  if (up) {
    servo8->rotate(true);
    delay(GAP_TIME);
    servo9->rotate(false);
    delay(GAP_TIME);
  }

  else {
    servo8->rotate(false);
    delay(GAP_TIME);
    servo9->rotate(true);
    delay(GAP_TIME);
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pwm.begin();
  pwm.setPWMFreq(50);

  configureServo0();
  configureServo1();
  configureServo2();
  configureServo3();
  configureServo4();
  configureServo5();
  configureServo6();
  configureServo7();
  configureServo8();
  configureServo9();

  stopAllServo();

  terminal.println("SYSTEM READY - SERVOS 0-9 CONFIGURED");
  terminal.println("V0 : Servo 0-3 Toggle");
  terminal.println("V1 : Servo 4-7 Toggle");
  terminal.println("V2 : Servo 8-9 Toggle");
  terminal.flush();
}

void loop() {
  Blynk.run();
}

BLYNK_WRITE(V0) {
  if (param.asInt()) {
    dirBtn1 = !dirBtn1;

    terminal.printf(
      "V0 | %s\n",
      dirBtn1 ? "UP (EVEN FIRST)" : "DOWN (ODD FIRST)"
    );
    terminal.flush();

    moveGroupOddEvenOrder(0, 3, dirBtn1);
  }
}

BLYNK_WRITE(V1) {
  if (param.asInt()) {
    dirBtn2 = !dirBtn2;

    terminal.printf(
      "V1 | %s\n",
      dirBtn2 ? "UP (EVEN FIRST)" : "DOWN (ODD FIRST)"
    );
    terminal.flush();

    moveGroupOddEvenOrder(4, 7, dirBtn2);
  }
}

BLYNK_WRITE(V2) {
  if (param.asInt()) {
    dirBtn3 = !dirBtn3;

    terminal.printf(
      "V2 | Servo 8-9 | %s\n",
      dirBtn3 ? "UP (8=CW, 9=CCW)" : "DOWN (8=CCW, 9=CW)"
    );
    terminal.flush();

    moveGroup8_9_Toggle180(dirBtn3);
  }
}