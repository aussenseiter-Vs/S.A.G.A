#define BLYNK_TEMPLATE_ID "TMPL6gB3c0_RQ"
#define BLYNK_TEMPLATE_NAME "penggerak servo"
#define BLYNK_AUTH_TOKEN "SrePreNN9hj_OX8Woprkwl9MoSt6fV_s"

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

// ================= I2C PINS =================
#define I2C_SDA 21  // I2C Data pin
#define I2C_SCL 22  // I2C Clock pin

// ================= SERVO CONTROLLER CLASS =================
class ServoController {
private:
  uint8_t pin;              // Servo pin on PCA9685
  uint16_t pwmCW;           // PWM value for clockwise rotation
  uint16_t pwmCCW;          // PWM value for counter-clockwise rotation
  uint16_t pwmStop;         // PWM value to stop servo
  uint16_t rotationTime;    // Time to rotate in milliseconds
  String name;              // Servo name for debugging
  Adafruit_PWMServoDriver* pwmDriver;  // Reference to PWM driver

public:
  // Constructor
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

  // Rotate clockwise for specified duration (or default)
  void rotateCW(int duration = -1) {
    int moveTime = (duration > 0) ? duration : rotationTime;
    terminal.print("   ");
    terminal.print(name);
    terminal.println(": CW ↻");
    terminal.flush();
    
    pwmDriver->setPWM(pin, 0, pwmCW);
    delay(moveTime);
  }

  // Rotate counter-clockwise for specified duration (or default)
  void rotateCCW(int duration = -1) {
    int moveTime = (duration > 0) ? duration : rotationTime;
    terminal.print("   ");
    terminal.print(name);
    terminal.println(": CCW ↺");
    terminal.flush();
    
    pwmDriver->setPWM(pin, 0, pwmCCW);
    delay(moveTime);
  }

  // Stop the servo
  void stop() {
    pwmDriver->setPWM(pin, 0, pwmStop);
  }

  // Set rotation speed (by adjusting PWM values)
  void setSpeed(uint16_t cwValue, uint16_t ccwValue) {
    pwmCW = cwValue;
    pwmCCW = ccwValue;
  }

  // Set rotation duration
  void setRotationTime(uint16_t milliseconds) {
    rotationTime = milliseconds;
  }

  // Get servo information
  String getName() { return name; }
  uint8_t getPin() { return pin; }
  uint16_t getRotationTime() { return rotationTime; }
};

// ================= SERVO OBJECTS =================
// Door servos (0-1)
ServoController* servo0;
ServoController* servo1;

// Roof servos (14-15)
ServoController* servo14;
ServoController* servo15;

// ================= SERVO CONFIGURATION FUNCTIONS =================

// Configure Servo 0 (Door Left)
void configureServo0() {
  servo0 = new ServoController(
    "Servo 0",            // Name
    0,                    // Pin on PCA9685
    395,                  // CW PWM value
    235,                  // CCW PWM value
    315,                  // STOP PWM value
    2600,                 // Rotation time (ms) for 260°
    &pwm                  // PWM driver reference
  );
}

// Configure Servo 1 (Door Right)
void configureServo1() {
  servo1 = new ServoController(
    "Servo 1",            // Name
    1,                    // Pin on PCA9685
    395,                  // CW PWM value
    235,                  // CCW PWM value
    315,                  // STOP PWM value
    2600,                 // Rotation time (ms) for 260°
    &pwm                  // PWM driver reference
  );
}

// Configure Servo 14 (Roof Left)
void configureServo14() {
  servo14 = new ServoController(
    "Servo 14",           // Name
    14,                   // Pin on PCA9685
    395,                  // CW PWM value
    235,                  // CCW PWM value
    315,                  // STOP PWM value
    2600,                 // Rotation time (ms) for 260°
    &pwm                  // PWM driver reference
  );
}

// Configure Servo 15 (Roof Right)
void configureServo15() {
  servo15 = new ServoController(
    "Servo 15",           // Name
    15,                   // Pin on PCA9685
    395,                  // CW PWM value
    235,                  // CCW PWM value
    315,                  // STOP PWM value
    2600,                 // Rotation time (ms) for 260°
    &pwm                  // PWM driver reference
  );
}

// Initialize all servos
void initializeAllServos() {
  configureServo0();
  configureServo1();
  configureServo14();
  configureServo15();
  
  // Stop all servos on initialization
  servo0->stop();
  servo1->stop();
  servo14->stop();
  servo15->stop();
}

// ================= DOOR CONTROL FUNCTIONS =================

// Stop all door servos
void stopDoorServos() {
  servo0->stop();
  servo1->stop();
}

// Open door (260° rotation)
void openDoor() {
  terminal.println("🚪 Opening door (260° rotation)...");
  terminal.flush();
  
  // Servo 0: Counter-Clockwise
  // Servo 1: Clockwise
  servo0->rotateCCW();   // Uses default rotation time
  servo1->rotateCW();    // Moves simultaneously in code, but delay is sequential
  
  // Stop both servos
  stopDoorServos();
  
  terminal.println("✅ Door opened (260°)");
  terminal.flush();
  
  delay(500);  // Pause at end position
}

// Close door (return 260°)
void closeDoor() {
  terminal.println("🚪 Closing door (returning 260°)...");
  terminal.flush();
  
  // Reverse direction to return
  // Servo 0: Clockwise
  // Servo 1: Counter-Clockwise
  servo0->rotateCW();
  servo1->rotateCCW();
  
  // Stop both servos
  stopDoorServos();
  
  terminal.println("✅ Door closed (back to 0°)");
  terminal.flush();
  
  delay(500);  // Pause at end position
}

// ================= ROOF CONTROL FUNCTIONS =================

// Stop all roof servos
void stopRoofServos() {
  servo14->stop();
  servo15->stop();
}

// Open roof (260° rotation)
void openRoof() {
  terminal.println("🏠 Opening roof (260° rotation)...");
  terminal.flush();
  
  // Servo 14: Counter-Clockwise
  // Servo 15: Clockwise
  servo14->rotateCCW();   // Uses default rotation time
  servo15->rotateCW();    // Moves simultaneously in code, but delay is sequential
  
  // Stop both servos
  stopRoofServos();
  
  terminal.println("✅ Roof opened (260°)");
  terminal.flush();
  
  delay(500);  // Pause at end position
}

// Close roof (return 260°)
void closeRoof() {
  terminal.println("🏠 Closing roof (returning 260°)...");
  terminal.flush();
  
  // Reverse direction to return
  // Servo 14: Clockwise
  // Servo 15: Counter-Clockwise
  servo14->rotateCW();
  servo15->rotateCCW();
  
  // Stop both servos
  stopRoofServos();
  
  terminal.println("✅ Roof closed (back to 0°)");
  terminal.flush();
  
  delay(500);  // Pause at end position
}

// ================= UTILITY FUNCTIONS =================

// Stop ALL servos (door and roof)
void stopAllServos() {
  stopDoorServos();
  stopRoofServos();
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  
  // Configure I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize PCA9685
  pwm.begin();
  pwm.setPWMFreq(50);  // 50Hz for servos
  
  delay(100);
  
  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  
  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Initialize all servo objects
  initializeAllServos();
  
  // Welcome message
  terminal.clear();
  terminal.println("🚀 DOOR & ROOF CONTROLLER (OOP)");
  terminal.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  terminal.println("V0: Door Control (Toggle)");
  terminal.println("    Servos 0-1");
  terminal.println("    260° rotation");
  terminal.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  terminal.println("V5: Roof Control (Toggle)");
  terminal.println("    Servos 14-15");
  terminal.println("    260° rotation");
  terminal.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  terminal.println("Open: S0/14=CCW ↺, S1/15=CW ↻");
  terminal.println("Close: Reverse direction");
  terminal.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  terminal.flush();
}

// ================= MAIN LOOP =================
void loop() {
  Blynk.run();
}

// ================= BLYNK VIRTUAL PIN HANDLERS =================

// V0 → Door Control (Toggle)
BLYNK_WRITE(V0) {
  static bool doorOpen = false;
  
  if (param.asInt()) {
    doorOpen = !doorOpen;
    
    if (doorOpen) {
      terminal.println("🔘 V0 pressed → OPEN 🚪");
      terminal.flush();
      openDoor();
    }
    else {
      terminal.println("🔘 V0 pressed → CLOSE 🚪");
      terminal.flush();
      closeDoor();
    }
  }
}

// V1 → Roof Control (Toggle)
BLYNK_WRITE(V1) {
  static bool roofOpen = false;
  
  if (param.asInt()) {
    roofOpen = !roofOpen;
    
    if (roofOpen) {
      terminal.println("🔘 V5 pressed → OPEN 🏠");
      terminal.flush();
      openRoof();
    }
    else {
      terminal.println("🔘 V5 pressed → CLOSE 🏠");
      terminal.flush();
      closeRoof();
    }
  }
}
