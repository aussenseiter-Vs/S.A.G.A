#define BLYNK_TEMPLATE_ID "TMPL6gB3c0_RQ"
#define BLYNK_TEMPLATE_NAME "penggerak servo"
#define BLYNK_AUTH_TOKEN "LSPKUZ_qlj_Xe8sNLsDHf2kPmEyuAF6c"

#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_PWMServoDriver.h>

// ================= WIFI =================
char ssid[] = "alvarooooo";
char pass[] = "99999999";

// ================= TERMINAL =================
WidgetTerminal terminal(V10);

// ================= PCA9685 =================
Adafruit_PWMServoDriver pwm(0x40);

// ================= SERVO =================
#define TOTAL_SERVO 16
constexpr uint8_t servo[TOTAL_SERVO] = {
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 9,10,11,12,13,14,15
};

// ================= CONTINUOUS SERVO (default values for 0-7) =================
#define SERVO_CW    395
#define SERVO_CCW  235
#define SERVO_STOP 315

// ================= TIMING =================
#define MOVE_TIME_180 1200
#define MOVE_TIME_90   600
#define GAP_TIME      1200

// ================= SERVO CONTROLLER CLASS =================
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

// ================= SERVO INSTANCES FOR V0, V1, V2 =================
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

// ================= STATUS TOGGLE =================
bool dirBtn1 = false;   // false = DOWN, true = UP
bool dirBtn2 = false;
bool dirBtn3 = false;   // untuk tombol V2

// ================= CONFIGURE SERVOS 0-9 =================
void configureServo0() {
  servo0 = new ServoController(
    "Servo 0",
    0,
    SERVO_CW,
    SERVO_CCW,
    SERVO_STOP,
    MOVE_TIME_180,  // Even servo uses 180°
    &pwm
  );
}

void configureServo1() {
  servo1 = new ServoController(
    "Servo 1",
    1,
    SERVO_CW,
    SERVO_CCW,
    SERVO_STOP,
    MOVE_TIME_90,   // Odd servo uses 90°
    &pwm
  );
}

void configureServo2() {
  servo2 = new ServoController(
    "Servo 2",
    2,
    SERVO_CW,
    SERVO_CCW,
    SERVO_STOP,
    MOVE_TIME_180,
    &pwm
  );
}

void configureServo3() {
  servo3 = new ServoController(
    "Servo 3",
    3,
    SERVO_CW,
    SERVO_CCW,
    SERVO_STOP,
    MOVE_TIME_90,
    &pwm
  );
}

void configureServo4() {
  servo4 = new ServoController(
    "Servo 4",
    4,
    405,
    SERVO_CCW,
    SERVO_STOP,
    1300,
    &pwm
  );
}

void configureServo5() {
  servo5 = new ServoController(
    "Servo 5",
    5,
    SERVO_CW,
    SERVO_CCW,
    SERVO_STOP,
    MOVE_TIME_90,
    &pwm
  );
}

void configureServo6() {
  servo6 = new ServoController(
    "Servo 6",
    6,
    405,
    SERVO_CCW,
    SERVO_STOP,
    1300,
    &pwm
  );
}

void configureServo7() {
  servo7 = new ServoController(
    "Servo 7",
    7,
    SERVO_CW,
    SERVO_CCW,
    SERVO_STOP,
    MOVE_TIME_90,
    &pwm
  );
}

void configureServo8() {
  servo8 = new ServoController(
    "Servo 8",
    8,
    425,
    235,
    315,
    1375,  // 180° timing
    &pwm
  );
}

void configureServo9() {
  servo9 = new ServoController(
    "Servo 9",
    9,
    415,
    235,
    315,
    1400,  // 180° timing
    &pwm
  );
}

// ================= STOP ALL =================
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

// ================= GROUP 0–7 (MODE UP/DOWN) =================
// UP   → EVEN first → ODD
// DOWN → ODD first  → EVEN
void moveGroupOddEvenOrder(int start, int end, bool up) {

  if (start < 0 || end >= 10 || start > end) return;

  // Create array of servo pointers for easy indexing (only 0-9)
  ServoController* servoArray[10] = {
    servo0, servo1, servo2, servo3, servo4, servo5, servo6, servo7, servo8, servo9
  };

  // ---------- UP ----------
  if (up) {
    // EVEN FIRST
    for (int i = start; i <= end; i++) {
      if (i % 2 == 0 && servoArray[i]) {
        servoArray[i]->rotate(true);  // CW
        delay(GAP_TIME);
      }
    }

    // ODD SECOND
    for (int i = start; i <= end; i++) {
      if (i % 2 == 1 && servoArray[i]) {
        servoArray[i]->rotate(false);  // CCW
        delay(GAP_TIME);
      }
    }
  }

  // ---------- DOWN ----------
  else {
    // ODD FIRST
    for (int i = start; i <= end; i++) {
      if (i % 2 == 1 && servoArray[i]) {
        servoArray[i]->rotate(true);  // CW
        delay(GAP_TIME);
      }
    }

    // EVEN SECOND
    for (int i = start; i <= end; i++) {
      if (i % 2 == 0 && servoArray[i]) {
        servoArray[i]->rotate(false);  // CCW
        delay(GAP_TIME);
      }
    }
  }
}

// ================= GROUP 8–9 (TOGGLE UP/DOWN, 180deg) =================
// Uses the ServoController instances
void moveGroup8_9_Toggle180(bool up) {
  
  if (!servo8 || !servo9) return;

  // UP mode: Servo 8 (even) = CW, Servo 9 (odd) = CCW
  if (up) {
    servo8->rotate(true);   // CW
    delay(GAP_TIME);
    servo9->rotate(false);  // CCW
    delay(GAP_TIME);
  }
  // DOWN mode: Servo 8 (even) = CCW, Servo 9 (odd) = CW
  else {
    servo8->rotate(false);  // CCW
    delay(GAP_TIME);
    servo9->rotate(true);   // CW
    delay(GAP_TIME);
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pwm.begin();
  pwm.setPWMFreq(50);

  // Configure servos 0-9
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

  terminal.println("🚀 SYSTEM READY - SERVOS 0-9 CONFIGURED");
  terminal.println("V0 : Servo 0–3 (toggle UP/DOWN)");
  terminal.println("V1 : Servo 4–7 (toggle UP/DOWN)");
  terminal.println("V2 : Servo 8–9 (toggle UP/DOWN, 180deg)");
  terminal.flush();
}

// ================= LOOP =================
void loop() {
  Blynk.run();
}

// ================= BLYNK BUTTONS =================

// Tombol 1 → Servo 0–3
BLYNK_WRITE(V0) {
  if (param.asInt()) {
    dirBtn1 = !dirBtn1;

    terminal.printf(
      "🔘 V0 | %s\n",
      dirBtn1 ? "UP (EVEN FIRST)" : "DOWN (ODD FIRST)"
    );
    terminal.flush();

    moveGroupOddEvenOrder(0, 3, dirBtn1);
  }
}

// Tombol 2 → Servo 4–7
BLYNK_WRITE(V1) {
  if (param.asInt()) {
    dirBtn2 = !dirBtn2;

    terminal.printf(
      "🔘 V1 | %s\n",
      dirBtn2 ? "UP (EVEN FIRST)" : "DOWN (ODD FIRST)"
    );
    terminal.flush();

    moveGroupOddEvenOrder(4, 7, dirBtn2);
  }
}

// Tombol 3 → Servo 8–9
BLYNK_WRITE(V2) {
  if (param.asInt()) {
    dirBtn3 = !dirBtn3;

    terminal.printf(
      "🔘 V2 | Servo 8–9 | %s\n",
      dirBtn3 ? "UP (8=CW, 9=CCW)" : "DOWN (8=CCW, 9=CW)"
    );
    terminal.flush();

    moveGroup8_9_Toggle180(dirBtn3);
  }
}
