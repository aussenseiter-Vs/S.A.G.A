#define BLYNK_TEMPLATE_ID "TMPL6gB3c0_RQ"
#define BLYNK_TEMPLATE_NAME "penggerak servo"
#define BLYNK_AUTH_TOKEN "SrePreNN9hj_OX8Woprkwl9MoSt6fV_s"

#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_PWMServoDriver.h>

char ssid[] = "alvarooooo";
char pass[] = "99999999";

WidgetTerminal terminal(V10);

Adafruit_PWMServoDriver pwm(0x40);

#define I2C_SDA 21
#define I2C_SCL 22

class ServoController {
private:
  uint8_t pin;
  uint16_t pwmCW;
  uint16_t pwmCCW;
  uint16_t pwmStop;
  uint16_t rotationTime;
  String name;
  Adafruit_PWMServoDriver* pwmDriver;

public:
  ServoController(String servoName, uint8_t servoPin,
                  uint16_t cwValue, uint16_t ccwValue, uint16_t stopValue,
                  uint16_t rotTime, Adafruit_PWMServoDriver* driver) {
    name = servoName;
    pin = servoPin;
    pwmCW = cwValue;
    pwmCCW = ccwValue;
    pwmStop = stopValue;
    rotationTime = rotTime;
    pwmDriver = driver;
  }

  void rotateCW(int duration = -1) {
    int moveTime = (duration > 0) ? duration : rotationTime;
    terminal.print("   ");
    terminal.print(name);
    terminal.println(": CW");
    terminal.flush();

    pwmDriver->setPWM(pin, 0, pwmCW);
    delay(moveTime);
  }

  void rotateCCW(int duration = -1) {
    int moveTime = (duration > 0) ? duration : rotationTime;
    terminal.print("   ");
    terminal.print(name);
    terminal.println(": CCW");
    terminal.flush();

    pwmDriver->setPWM(pin, 0, pwmCCW);
    delay(moveTime);
  }

  void stop() {
    pwmDriver->setPWM(pin, 0, pwmStop);
  }

  void setSpeed(uint16_t cwValue, uint16_t ccwValue) {
    pwmCW = cwValue;
    pwmCCW = ccwValue;
  }

  void setRotationTime(uint16_t milliseconds) {
    rotationTime = milliseconds;
  }

  String getName() { return name; }
  uint8_t getPin() { return pin; }
  uint16_t getRotationTime() { return rotationTime; }
};

ServoController* servo0;
ServoController* servo1;
ServoController* servo14;
ServoController* servo15;

void configureServo0() {
  servo0 = new ServoController("Servo 0", 0, 395, 235, 315, 2600, &pwm);
}

void configureServo1() {
  servo1 = new ServoController("Servo 1", 1, 395, 235, 315, 2600, &pwm);
}

void configureServo14() {
  servo14 = new ServoController("Servo 14", 14, 395, 235, 315, 2600, &pwm);
}

void configureServo15() {
  servo15 = new ServoController("Servo 15", 15, 395, 235, 315, 2600, &pwm);
}

void initializeAllServos() {
  configureServo0();
  configureServo1();
  configureServo14();
  configureServo15();

  servo0->stop();
  servo1->stop();
  servo14->stop();
  servo15->stop();
}

void stopDoorServos() {
  servo0->stop();
  servo1->stop();
}

void openDoor() {
  terminal.println("Opening door...");
  terminal.flush();

  servo0->rotateCCW();
  servo1->rotateCW();

  stopDoorServos();

  terminal.println("Door opened");
  terminal.flush();

  delay(500);
}

void closeDoor() {
  terminal.println("Closing door...");
  terminal.flush();

  servo0->rotateCW();
  servo1->rotateCCW();

  stopDoorServos();

  terminal.println("Door closed");
  terminal.flush();

  delay(500);
}

void stopRoofServos() {
  servo14->stop();
  servo15->stop();
}

void openRoof() {
  terminal.println("Opening roof...");
  terminal.flush();

  servo14->rotateCCW();
  servo15->rotateCW();

  stopRoofServos();

  terminal.println("Roof opened");
  terminal.flush();

  delay(500);
}

void closeRoof() {
  terminal.println("Closing roof...");
  terminal.flush();

  servo14->rotateCW();
  servo15->rotateCCW();

  stopRoofServos();

  terminal.println("Roof closed");
  terminal.flush();

  delay(500);
}

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  pwm.begin();
  pwm.setPWMFreq(50);

  delay(100);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  initializeAllServos();

  terminal.clear();
  terminal.println("DOOR & ROOF CONTROLLER");
  terminal.println("V0: Door Toggle");
  terminal.println("V5: Roof Toggle");
  terminal.flush();
}

void loop() {
  Blynk.run();
}

BLYNK_WRITE(V0) {
  static bool doorOpen = false;

  if (param.asInt()) {
    doorOpen = !doorOpen;

    if (doorOpen) {
      terminal.println("V0 pressed -> OPEN");
      terminal.flush();
      openDoor();
    } else {
      terminal.println("V0 pressed -> CLOSE");
      terminal.flush();
      closeDoor();
    }
  }
}

BLYNK_WRITE(V1) {
  static bool roofOpen = false;

  if (param.asInt()) {
    roofOpen = !roofOpen;

    if (roofOpen) {
      terminal.println("V1 pressed -> OPEN");
      terminal.flush();
      openRoof();
    } else {
      terminal.println("V1 pressed -> CLOSE");
      terminal.flush();
      closeRoof();
    }
  }
}