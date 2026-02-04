#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ================= I2C PINS =================
#define I2C_SDA 21  // I2C Data pin
#define I2C_SCL 22  // I2C Clock pin

// ================= ULTRASONIC SENSOR PINS =================
#define TRIG_PIN 5   // Trigger pin - sends ultrasonic pulse
#define ECHO_PIN 18  // Echo pin - receives reflected pulse

// ================= PCA9685 =================
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// ================= SETTINGS =================
#define DETECTION_DISTANCE 20  // Detection threshold in cm

// ================= SERVO CONFIGURATION =================
// Servo 0 configuration
#define SERVO_0_PIN 0
#define SERVO_0_CW  395   // Clockwise PWM value
#define SERVO_0_CCW 235   // Counter-clockwise PWM value
#define SERVO_0_STOP 315  // Stop PWM value

// Servo 1 configuration
#define SERVO_1_PIN 1
#define SERVO_1_CW  395   // Clockwise PWM value
#define SERVO_1_CCW 235   // Counter-clockwise PWM value
#define SERVO_1_STOP 315  // Stop PWM value

// Rotation settings
#define ROTATION_TIME 2600  // Time for ~260° rotation (milliseconds)
#define PAUSE_TIME 500      // Pause at end of rotation

// ================= VARIABLES =================
bool objectDetected = false;
unsigned long lastCheckTime = 0;
#define CHECK_INTERVAL 200  // Check distance every 200ms

// ================= ULTRASONIC FUNCTION =================
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
  // Speed of sound: 343 m/s = 0.0343 cm/µs
  // Distance = (duration / 2) * 0.0343
  float distance = (duration * 0.0343) / 2;
  
  return distance;
}

// ================= SERVO CONTROL FUNCTIONS =================

// Stop both servos
void stopServos() {
  pwm.setPWM(SERVO_0_PIN, 0, SERVO_0_STOP);
  pwm.setPWM(SERVO_1_PIN, 0, SERVO_1_STOP);
}

// Rotate servo 0 clockwise
void rotateServo0_CW() {
  pwm.setPWM(SERVO_0_PIN, 0, SERVO_0_CW);
}

// Rotate servo 0 counter-clockwise
void rotateServo0_CCW() {
  pwm.setPWM(SERVO_0_PIN, 0, SERVO_0_CCW);
}

// Rotate servo 1 clockwise
void rotateServo1_CW() {
  pwm.setPWM(SERVO_1_PIN, 0, SERVO_1_CW);
}

// Rotate servo 1 counter-clockwise
void rotateServo1_CCW() {
  pwm.setPWM(SERVO_1_PIN, 0, SERVO_1_CCW);
}

// ================= 260° ROTATION FUNCTION =================
void rotate260Degrees() {
  Serial.println("🔄 Starting 260° rotation...");
  Serial.println("   Servo 0: CW");
  Serial.println("   Servo 1: CW");
  
  // Rotate both servos clockwise for 260°
  rotateServo0_CW();
  rotateServo1_CW();
  delay(ROTATION_TIME);
  
  // Stop servos
  stopServos();
  Serial.println("✅ Reached 260° - Pausing...");
  delay(PAUSE_TIME);
  
  // Rotate back counter-clockwise to starting position
  Serial.println("↩️  Returning to start...");
  Serial.println("   Servo 0: CCW");
  Serial.println("   Servo 1: CCW");
  
  rotateServo0_CCW();
  rotateServo1_CCW();
  delay(ROTATION_TIME);
  
  // Stop servos at starting position
  stopServos();
  Serial.println("✅ Returned to 0° - Servos stopped");
}

// ================= SETUP =================
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║  ULTRASONIC + PCA9685 SERVO TEST      ║");
  Serial.println("╚═══════════════════════════════════════╝");
  
  // Configure I2C pins
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Configure ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  
  // Initialize PCA9685
  pwm.begin();
  pwm.setPWMFreq(50);  // 50Hz for servos
  
  // Stop all servos on startup
  stopServos();
  delay(100);
  
  Serial.println("\n📌 Configuration:");
  Serial.println("   I2C SDA: GPIO " + String(I2C_SDA));
  Serial.println("   I2C SCL: GPIO " + String(I2C_SCL));
  Serial.println("   TRIG Pin: GPIO " + String(TRIG_PIN));
  Serial.println("   ECHO Pin: GPIO " + String(ECHO_PIN));
  Serial.println("   Controlling: Servo 0 & Servo 1");
  Serial.println("   Detection Distance: " + String(DETECTION_DISTANCE) + " cm");
  Serial.println("   Rotation Time: " + String(ROTATION_TIME) + " ms (~260°)");
  Serial.println("\n🚀 System Ready!");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

// ================= MAIN LOOP =================
void loop() {
  // Check sensor at intervals
  unsigned long currentTime = millis();
  
  if (currentTime - lastCheckTime >= CHECK_INTERVAL) {
    lastCheckTime = currentTime;
    
    // Get distance measurement
    float distance = getDistance();
    
    // Ignore invalid readings
    if (distance == 0 || distance > 400) {
      Serial.println("⚠️  Invalid reading - no object detected");
      return;
    }
    
    // Display distance
    Serial.print("📏 Distance: ");
    Serial.print(distance);
    Serial.print(" cm");
    
    // Check if object is within detection range
    if (distance < DETECTION_DISTANCE && !objectDetected) {
      Serial.println(" 🎯 OBJECT DETECTED!");
      objectDetected = true;
      
      // Perform 260° rotation on both servos
      rotate260Degrees();
      
      // Wait before detecting again
      Serial.println("\n⏳ Cooldown period (3 seconds)...\n");
      delay(3000);
      objectDetected = false;
      
    } else if (distance >= DETECTION_DISTANCE) {
      Serial.println(" ✓ Clear");
      objectDetected = false;
    } else {
      Serial.println();
    }
  }
}
