#define BLYNK_TEMPLATE_ID "TMPL6gB3c0_RQ"
#define BLYNK_TEMPLATE_NAME "penggerak servo"
#define BLYNK_AUTH_TOKEN "LSPKUZ_qlj_Xe8sNLsDHf2kPmEyuAF6c"

#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_PWMServoDriver.h>

// ================= WIFI =================
char ssid[] = "HardwareLab";
char pass[] = "hardwareonfire2026";

// ================= TERMINAL =================
WidgetTerminal terminal(V10);

// ================= PCA9685 =================
Adafruit_PWMServoDriver pwm(0x40);

// ================= I2C PINS (for PCA9685 communication) =================
#define I2C_SDA 21  // Default SDA pin for ESP32
#define I2C_SCL 22  // Default SCL pin for ESP32

// ================= ULTRASONIC SENSOR PINS =================
#define TRIG_PIN 5   // Trigger pin for ultrasonic sensor
#define ECHO_PIN 18  // Echo pin for ultrasonic sensor

// ================= DOOR CONTROL SETTINGS =================
#define DOOR_DISTANCE_THRESHOLD 50  // Distance in cm to trigger door opening
#define DOOR_SERVO_START 12         // First servo for door (servo 12)
#define DOOR_SERVO_END 15           // Last servo for door (servo 15)

bool doorIsOpen = false;  // Track door state
unsigned long lastDoorCheck = 0;
#define DOOR_CHECK_INTERVAL 500  // Check distance every 500ms

// ================= SERVO CONFIGURATION =================
#define TOTAL_SERVO 16

// Servo state structure
struct ServoState {
  uint8_t pin;           // Servo pin number
  uint16_t cwValue;      // Clockwise PWM value
  uint16_t ccwValue;     // Counter-clockwise PWM value
  uint16_t stopValue;    // Stop PWM value
  uint16_t moveTime;     // Movement duration in ms
  uint16_t gapTime;      // Gap time after movement in ms
  bool currentPosition;  // Current position state (true = up/extended, false = down/retracted)
};

// Initialize servo states with individual configurations
ServoState servos[TOTAL_SERVO] = {
  // pin, CW,  CCW, STOP, moveTime, gapTime, position
  {0,   395, 235, 315, 1200, 1200, false},  // Servo 0
  {1,   395, 235, 315,  600, 1200, false},  // Servo 1
  {2,   395, 235, 315, 1200, 1200, false},  // Servo 2
  {3,   395, 235, 315,  600, 1200, false},  // Servo 3
  {4,   395, 235, 315, 1200, 1200, false},  // Servo 4
  {5,   395, 235, 315,  600, 1200, false},  // Servo 5
  {6,   395, 235, 315, 1200, 1200, false},  // Servo 6
  {7,   395, 235, 315,  600, 1200, false},  // Servo 7
  {8,   410, 235, 315, 1350, 1200, false},  // Servo 8
  {9,   395, 235, 315, 1300, 1200, false},  // Servo 9
  {10,  395, 235, 315, 1200, 1200, false},  // Servo 10
  {11,  395, 235, 315, 1200, 1200, false},  // Servo 11
  {12,  395, 235, 315, 1200, 1200, false},  // Servo 12 (Door)
  {13,  395, 235, 315, 1200, 1200, false},  // Servo 13 (Door - alternates CW/CCW)
  {14,  395, 235, 315, 1200, 1200, false},  // Servo 14 (Door)
  {15,  395, 235, 315, 1200, 1200, false},  // Servo 15 (Door)
};

// Track servo 13's next direction (true = CW, false = CCW)
bool servo13_nextCW = true;

// ================= ULTRASONIC SENSOR FUNCTIONS =================

// Measure distance using ultrasonic sensor
float getDistance() {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read echo pulse
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // Timeout after 30ms
  
  // Calculate distance in cm
  // Speed of sound is 343 m/s or 0.0343 cm/µs
  // Distance = (duration / 2) * 0.0343
  float distance = (duration * 0.0343) / 2;
  
  return distance;
}

// ================= SERVO CONTROL FUNCTIONS =================

// Stop all servos
void stopAllServos() {
  for (int i = 0; i < TOTAL_SERVO; i++) {
    pwm.setPWM(servos[i].pin, 0, servos[i].stopValue);
  }
}

// Move a single servo
void moveServo(int index, bool direction) {
  if (index < 0 || index >= TOTAL_SERVO) return;
  
  ServoState &s = servos[index];
  uint16_t pwmValue = direction ? s.cwValue : s.ccwValue;
  
  pwm.setPWM(s.pin, 0, pwmValue);
  delay(s.moveTime);
  pwm.setPWM(s.pin, 0, s.stopValue);
  delay(s.gapTime);
  
  s.currentPosition = direction;
}

// ================= GROUP MOVEMENT FUNCTIONS =================

// Group 0-7: Odd-Even ordering based on direction
void moveGroup_OddEven(int start, int end, bool up) {
  if (start < 0 || end >= TOTAL_SERVO || start > end) return;

  terminal.printf("Moving servos %d-%d: %s\n", start, end, up ? "UP" : "DOWN");
  terminal.flush();

  if (up) {
    // UP: EVEN servos first (CW), then ODD servos (CCW)
    // EVEN FIRST
    for (int i = start; i <= end; i++) {
      if (i % 2 == 0) {
        moveServo(i, true);  // CW
      }
    }
    // ODD SECOND
    for (int i = start; i <= end; i++) {
      if (i % 2 == 1) {
        moveServo(i, false);  // CCW
      }
    }
  }
  else {
    // DOWN: ODD servos first (CW), then EVEN servos (CCW)
    // ODD FIRST
    for (int i = start; i <= end; i++) {
      if (i % 2 == 1) {
        moveServo(i, true);  // CW
      }
    }
    // EVEN SECOND
    for (int i = start; i <= end; i++) {
      if (i % 2 == 0) {
        moveServo(i, false);  // CCW
      }
    }
  }
}

// Group 8-15: Sequential movement with direction toggle
// Exception: Servo 13 alternates between CW and CCW on each click
void moveGroup_Sequential(int start, int end, bool up) {
  if (start < 0 || end >= TOTAL_SERVO || start > end) return;

  terminal.printf("Moving servos %d-%d: %s\n", start, end, up ? "UP" : "DOWN");
  terminal.flush();

  for (int i = start; i <= end; i++) {
    bool direction;
    
    // Special case: Servo 13 alternates CW/CCW
    if (i == 13) {
      direction = servo13_nextCW;  // Use stored direction
      terminal.printf("  Servo 13: %s\n", direction ? "CW" : "CCW");
      terminal.flush();
      moveServo(i, direction);
      servo13_nextCW = !servo13_nextCW;  // Toggle for next time
      continue;  // Skip the normal moveServo call below
    }
    
    // Normal servos: alternate based on up flag and even/odd
    if (up) {
      direction = (i % 2 == 0);  // EVEN=CW, ODD=CCW
    }
    else {
      direction = (i % 2 == 1);  // EVEN=CCW, ODD=CW
    }
    
    moveServo(i, direction);
  }
}

// ================= DOOR FUNCTION =================
void doorFunc() {
  // Only check at intervals to avoid overwhelming the sensor
  unsigned long currentMillis = millis();
  if (currentMillis - lastDoorCheck < DOOR_CHECK_INTERVAL) {
    return;
  }
  lastDoorCheck = currentMillis;
  
  float distance = getDistance();
  
  // Ignore invalid readings (0 or very large values)
  if (distance == 0 || distance > 400) {
    return;
  }
  
  // Check if someone is close to the door
  if (distance < DOOR_DISTANCE_THRESHOLD && !doorIsOpen) {
    // Open door
    terminal.printf("🚪 Person detected: %.1f cm - Opening door\n", distance);
    terminal.flush();
    
    moveGroup_Sequential(DOOR_SERVO_START, DOOR_SERVO_END, true);
    doorIsOpen = true;
    
    terminal.println("✅ Door opened");
    terminal.flush();
  }
  else if (distance >= DOOR_DISTANCE_THRESHOLD && doorIsOpen) {
    // Close door
    terminal.printf("🚪 Person left: %.1f cm - Closing door\n", distance);
    terminal.flush();
    
    moveGroup_Sequential(DOOR_SERVO_START, DOOR_SERVO_END, false);
    doorIsOpen = false;
    
    terminal.println("✅ Door closed");
    terminal.flush();
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // ========== PIN MODE CONFIGURATION ==========
  
  // Configure I2C pins for PCA9685 communication
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Configure ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize trigger pin to LOW
  digitalWrite(TRIG_PIN, LOW);
  
  // Initialize PCA9685 (uses I2C communication)
  pwm.begin();
  pwm.setPWMFreq(50);
  
  // Stop all servos immediately on startup
  stopAllServos();
  delay(100);  // Give servos time to receive stop signal
  
  // Extra stop for servo 9 if it's problematic
  pwm.setPWM(9, 0, servos[9].stopValue);
  delay(50);

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Stop all servos again after WiFi/Blynk init
  stopAllServos();

  // Print welcome message
  terminal.clear();
  terminal.println("🚀 SERVO CONTROLLER READY");
  terminal.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  terminal.println("V0: Servos 0-3 (Toggle)");
  terminal.println("V1: Servos 4-7 (Toggle)");
  terminal.println("V2: Servos 8-11 (Toggle)");
  terminal.println("V3: Stairs Control");
  terminal.println("V4: Roof Control");
  terminal.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  terminal.println("🚪 Auto Door: Servos 12-15");
  terminal.println("    Sensor on pins 5 & 18");
  terminal.println("    ⚠️ Servo 13 alternates CW/CCW");
  terminal.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  terminal.println("I2C: SDA=21, SCL=22");
  terminal.println("━━━━━━━━━━━━━━━━━━━━━━━━");
  terminal.flush();
}

// ================= MAIN LOOP =================
void loop() {
  Blynk.run();
  
  // Continuously monitor ultrasonic sensor for automatic door
  doorFunc();
}

// ================= BLYNK VIRTUAL PIN HANDLERS =================

// V0 → Servos 0-3 (Odd-Even pattern)
BLYNK_WRITE(V0) {
  static bool direction = false;
  
  if (param.asInt()) {
    direction = !direction;
    terminal.printf("🔘 V0 → %s\n", direction ? "UP ⬆️" : "DOWN ⬇️");
    terminal.flush();
    
    moveGroup_OddEven(0, 3, direction);
    
    terminal.println("✅ Movement complete");
    terminal.flush();
  }
}

// V1 → Servos 4-7 (Odd-Even pattern)
BLYNK_WRITE(V1) {
  static bool direction = false;
  
  if (param.asInt()) {
    direction = !direction;
    terminal.printf("🔘 V1 → %s\n", direction ? "UP ⬆️" : "DOWN ⬇️");
    terminal.flush();
    
    moveGroup_OddEven(4, 7, direction);
    
    terminal.println("✅ Movement complete");
    terminal.flush();
  }
}

// V2 → Servos 8-11 (Sequential with toggle)
BLYNK_WRITE(V2) {
  static bool direction = false;
  
  if (param.asInt()) {
    direction = !direction;
    terminal.printf("🔘 V2 → %s\n", direction ? "UP ⬆️" : "DOWN ⬇️");
    terminal.flush();
    
    moveGroup_Sequential(8, 11, direction);
    
    terminal.println("✅ Movement complete");
    terminal.flush();
  }
}

// V3 → Stairs control (you can customize this)
BLYNK_WRITE(V3) {
  static bool direction = false;
  
  if (param.asInt()) {
    direction = !direction;
    terminal.printf("🔘 V3 (Stairs) → %s\n", direction ? "UP ⬆️" : "DOWN ⬇️");
    terminal.flush();
    
    // Add your stairs logic here
    // Example: moveGroup_OddEven(0, 7, direction);
    
    terminal.println("✅ Stairs movement complete");
    terminal.flush();
  }
}

// V4 → Roof control (you can customize this)
BLYNK_WRITE(V4) {
  static bool direction = false;
  
  if (param.asInt()) {
    direction = !direction;
    terminal.printf("🔘 V4 (Roof) → %s\n", direction ? "UP ⬆️" : "DOWN ⬇️");
    terminal.flush();
    
    // Add your roof logic here
    // Example: moveGroup_Sequential(8, 15, direction);
    
    terminal.println("✅ Roof movement complete");
    terminal.flush();
  }
}

// V5 → Manual door control override
BLYNK_WRITE(V5) {
  static bool direction = false;
  
  if (param.asInt()) {
    direction = !direction;
    terminal.printf("🔘 V5 (Door Manual) → %s\n", direction ? "OPEN 🚪" : "CLOSE 🚪");
    terminal.flush();
    
    moveGroup_Sequential(DOOR_SERVO_START, DOOR_SERVO_END, direction);
    doorIsOpen = direction;
    
    terminal.println("✅ Door manual control complete");
    terminal.flush();
  }
}
