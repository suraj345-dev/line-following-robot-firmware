/*
 Line Following Robot Firmware v1.0
 Author: Embedded Robotics Engineer
 Purpose: Robust line following with dual IR sensors
 Pin Configuration:
 - Left Motor: ENA (PWM 11), IN1 (10), IN2 (9)
 - Right Motor: ENB (PWM 6), IN3 (8), IN4 (7)
 - IR Sensors: Left (2), Right (3)
*/
// ============================================================
// PIN DEFINITIONS
// ============================================================
// Left Motor
const uint8_t ENA = 11;
const uint8_t IN1 = 10;
const uint8_t IN2 = 9;
// Right Motor
const uint8_t ENB = 6;
const uint8_t IN3 = 8;
const uint8_t IN4 = 7;
// IR Sensors
const uint8_t IRSensorLeft = 2;
const uint8_t IRSensorRight = 3;
// ============================================================
// CONFIGURATION PARAMETERS
// ============================================================
// Motor speed constants (0-255)
const uint8_t MAX_SPEED = 255;
const uint8_t BASE_SPEED = 150;        // Forward speed
const uint8_t CORRECTION_SPEED = 120;  // Turn speed  
const uint8_t MIN_SPEED = 100;         // IMPORTANT: Minimum speed to overcome friction
// Sensor thresholds
const uint16_t SENSOR_THRESHOLD = 500; // Adjust based on your IR sensor calibration
 // Below threshold = on black line (0-1023 ADC)
// Line following states
const uint8_t STATE_ON_LINE = 0;
const uint8_t STATE_LEFT_CORRECTION = 1;
const uint8_t STATE_RIGHT_CORRECTION = 2;
// ============================================================
// GLOBAL VARIABLES
// ============================================================
uint8_t currentState = STATE_ON_LINE;
uint16_t sensorLeftValue = 0;
uint16_t sensorRightValue = 0;
unsigned long lastSensorReadTime = 0;
const unsigned long SENSOR_READ_INTERVAL = 20; // Read sensors every 20ms
// ============================================================
// SETUP
// ============================================================
void setup() {
 // Initialize serial for debugging
 Serial.begin(9600);
 Serial.println("Line Following Robot Initialized");
 delay(2000); // Startup delay for stabilization
 // Configure motor pins
 pinMode(ENA, OUTPUT);
 pinMode(IN1, OUTPUT);
 pinMode(IN2, OUTPUT);
 pinMode(ENB, OUTPUT);
 pinMode(IN3, OUTPUT);
 pinMode(IN4, OUTPUT);
 // Configure sensor pins
 pinMode(IRSensorLeft, INPUT);
 pinMode(IRSensorRight, INPUT);
 // Motor brake (stop)
 stopMotors();
 
 Serial.println("Ready! Place robot on line and press start...");
}
// ============================================================
// MAIN LOOP
// ============================================================
void loop() {
 // Non-blocking sensor reading
 if (millis() - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
 lastSensorReadTime = millis();
 readSensors();
 updateLineFollowingState();
 executeMotorCommands();
 
 // Debug output every 500ms
 debugOutput();
 }
}
// ============================================================
// SENSOR READING
// ============================================================
void readSensors() {
 // Read digital values from IR sensors (with inversion)
 sensorLeftValue = !digitalRead(IRSensorLeft);
 sensorRightValue = !digitalRead(IRSensorRight);
}
// ============================================================
// LINE FOLLOWING STATE MACHINE
// ============================================================
void updateLineFollowingState() {
 // After inversion: 0 = on line (black), 1 = off line (white)
 boolean leftOnLine = (sensorLeftValue == 0);   // 0 = black detected
 boolean rightOnLine = (sensorRightValue == 0); // 0 = black detected

 if (leftOnLine && rightOnLine) {
 // Both sensors on line - go straight
 currentState = STATE_ON_LINE;
 }
 else if (leftOnLine && !rightOnLine) {
 // Left on line, right off - line curves left, turn left
 currentState = STATE_LEFT_CORRECTION;
 }
 else if (!leftOnLine && rightOnLine) {
 // Right on line, left off - line curves right, turn right
 currentState = STATE_RIGHT_CORRECTION;
 }
 else {
 // Both sensors off line - lost the line
 // Keep turning in last known direction (hysteresis)
 }
}
// ============================================================
// MOTOR CONTROL
// ============================================================
void executeMotorCommands() {
 switch (currentState) {
 case STATE_ON_LINE:
 moveForward(BASE_SPEED);
 break;
 case STATE_LEFT_CORRECTION:
 // Line is to the left - turn left (reduce left motor speed)
 moveLeft(CORRECTION_SPEED);
 break;
 case STATE_RIGHT_CORRECTION:
 // Line is to the right - turn right (reduce right motor speed)
 moveRight(CORRECTION_SPEED);
 break;
 default:
 stopMotors();
 break;
 }
}
// ============================================================
// MOTOR MOVEMENT FUNCTIONS
// ============================================================
void moveForward(uint8_t speed) {
 setMotorLeft(1, speed); // 1 = forward
 setMotorRight(1, speed);
}
void moveLeft(uint8_t speed) {
 // Line curves left - reduce LEFT motor but keep it moving
 uint8_t leftSpeed = (speed >= 50) ? (speed - 50) : MIN_SPEED;  // Never go below MIN_SPEED
 setMotorLeft(1, leftSpeed);
 setMotorRight(1, speed);
}

void moveRight(uint8_t speed) {
 // Line curves right - reduce RIGHT motor but keep it moving
 uint8_t rightSpeed = (speed >= 50) ? (speed - 50) : MIN_SPEED;  // Never go below MIN_SPEED
 setMotorLeft(1, speed);
 setMotorRight(1, rightSpeed);
}
void stopMotors() {
 setMotorLeft(0, 0);
 setMotorRight(0, 0);
}
// Motor control helper
// direction: 1 = forward, 0 = backward
void setMotorLeft(uint8_t direction, uint8_t speed) {
 speed = constrain(speed, 0, MAX_SPEED);
 
 if (direction == 1) {
 // Forward
 digitalWrite(IN1, HIGH);
 digitalWrite(IN2, LOW);
 } else {
 // Backward
 digitalWrite(IN1, LOW);
 digitalWrite(IN2, HIGH);
 }
 analogWrite(ENA, speed);
}
void setMotorRight(uint8_t direction, uint8_t speed) {
 speed = constrain(speed, 0, MAX_SPEED);
 
 if (direction == 1) {
 // Forward
 digitalWrite(IN3, HIGH);
 digitalWrite(IN4, LOW);
 } else {
 // Backward
 digitalWrite(IN3, LOW);
 digitalWrite(IN4, HIGH);
 }
 analogWrite(ENB, speed);
}
// ============================================================
// DEBUG FUNCTIONS
// ============================================================
void debugOutput() {
 static unsigned long lastDebugTime = 0;
 
 if (millis() - lastDebugTime >= 500) { // Print every 500ms
 lastDebugTime = millis();
 
 Serial.print("Left: ");
 Serial.print(sensorLeftValue);
 Serial.print(" | Right: ");
 Serial.print(sensorRightValue);
 Serial.print(" | State: ");
 
 switch (currentState) {
 case STATE_ON_LINE:
 Serial.println("FORWARD");
 break;
 case STATE_LEFT_CORRECTION:
 Serial.println("TURN LEFT");
 break;
 case STATE_RIGHT_CORRECTION:
 Serial.println("TURN RIGHT");
 break;
 default:
 Serial.println("UNKNOWN");
 break;
 }
 }
}
