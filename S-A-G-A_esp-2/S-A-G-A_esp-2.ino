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

// ================= SERVO 8 & 9 INSTANCES =================
ServoController* servo8 = nullptr;
ServoController* servo9 = nullptr;

// ================= STATUS TOGGLE =================
bool dirBtn1 = false;   // false = DOWN, true = UP
bool dirBtn2 = false;
bool dirBtn3 = false;   // untuk tombol V2

// ================= CONFIGURE SERVOS 8-9 =================
void configureServo8() {
  servo8 = new ServoController(
    "Servo 8",            // Name
    8,                    // Pin on PCA9685
    420,                  // CW PWM value
    235,                  // CCW PWM value
    315,                  // STOP PWM value
    1500,                 // Rotation time (ms) for 180°
    &pwm                  // PWM driver reference
  );
}

void configureServo9() {
  servo9 = new ServoController(
    "Servo 9",            // Name
    9,                    // Pin on PCA9685
    420,                  // CW PWM value
    235,                  // CCW PWM value
    315,                  // STOP PWM value
    1500,                 // Rotation time (ms) for 180°
    &pwm                  // PWM driver reference
  );
}

// ================= STOP ALL =================
void stopAllServo() {
  for (int i = 0; i < TOTAL_SERVO; i++) {
    pwm.setPWM(servo[i], 0, SERVO_STOP);
  }
}

// ================= GROUP 0–7 (MODE UP/DOWN) =================
// UP   → EVEN first → ODD
// DOWN → ODD first  → EVEN
void moveGroupOddEvenOrder(int start, int end, bool up) {

  if (start < 0 || end >= TOTAL_SERVO || start > end) return;

  // ---------- UP ----------
  if (up) {
    // EVEN FIRST
    for (int i = start; i <= end; i++) {
      if (i % 2 == 0) {
        pwm.setPWM(servo[i], 0, SERVO_CW);
        delay(MOVE_TIME_180);
        pwm.setPWM(servo[i], 0, SERVO_STOP);
        delay(GAP_TIME);
      }
    }

    // ODD SECOND
    for (int i = start; i <= end; i++) {
      if (i % 2 == 1) {
        pwm.setPWM(servo[i], 0, SERVO_CCW);
        delay(MOVE_TIME_90);
        pwm.setPWM(servo[i], 0, SERVO_STOP);
        delay(GAP_TIME);
      }
    }
  }

  // ---------- DOWN ----------
  else {
    // ODD FIRST
    for (int i = start; i <= end; i++) {
      if (i % 2 == 1) {
        pwm.setPWM(servo[i], 0, SERVO_CW);
        delay(MOVE_TIME_90);
        pwm.setPWM(servo[i], 0, SERVO_STOP);
        delay(GAP_TIME);
      }
    }

    // EVEN SECOND
    for (int i = start; i <= end; i++) {
      if (i % 2 == 0) {
        pwm.setPWM(servo[i], 0, SERVO_CCW);
        delay(MOVE_TIME_180);
        pwm.setPWM(servo[i], 0, SERVO_STOP);
        delay(GAP_TIME);
      }
    }
  }
}

// ================= GROUP 8–9 (TOGGLE UP/DOWN, 180deg) =================
// Now uses the ServoController instances
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

  // Configure servos 8 & 9
  configureServo8();
  configureServo9();

  stopAllServo();

  terminal.println("🚀 SYSTEM READY");
  terminal.println("V0 : Servo 0–3 (toggle UP/DOWN)");
  terminal.println("V1 : Servo 4–7 (toggle UP/DOWN)");
  terminal.println("V2 : Servo 8–9 (toggle UP/DOWN, 180deg, CONFIGURABLE)");
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
