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
const uint8_t BASE_SPEED = 200; // Normal forward speed
const uint8_t CORRECTION_SPEED = 150; // Speed during line corrections
const uint8_t MIN_SPEED = 100; // Minimum speed to overcome friction
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
 
 // Uncomment for debugging
 // debugOutput();
 }
}
// ============================================================
// SENSOR READING
// ============================================================
void readSensors() {
 // Read analog values from IR sensors
 // Digital pins 2 & 3 are mapped from analog if using A0, A1
 // If using digital sensors, they'll return HIGH (white) or LOW (black)
 
 sensorLeftValue = digitalRead(IRSensorLeft);
 sensorRightValue = digitalRead(IRSensorRight);
 
 // Invert logic: LOW (0) = black line detected, HIGH (1) = white surface
 // This makes the logic more intuitive: 0 = on line, 1 = off line
}
// ============================================================
// LINE FOLLOWING STATE MACHINE
// ============================================================
void updateLineFollowingState() {
 boolean leftOnLine = (sensorLeftValue == LOW); // Black detected
 boolean rightOnLine = (sensorRightValue == LOW); // Black detected
 if (leftOnLine && rightOnLine) {
 // Both sensors on line - go straight
 currentState = STATE_ON_LINE;
 }
 else if (leftOnLine && !rightOnLine) {
 // Left on line, right off - line curves left
 currentState = STATE_LEFT_CORRECTION;
 }
 else if (!leftOnLine && rightOnLine) {
 // Right on line, left off - line curves right
 currentState = STATE_RIGHT_CORRECTION;
 }
 else {
 // Both sensors off line - lost the line
 // Attempt recovery by turning harder in last known direction
 // (currentState remains unchanged for hysteresis)
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
 // Reduce left motor speed to turn left
 setMotorLeft(1, speed - 50); // Left motor slower
 setMotorRight(1, speed); // Right motor normal
}
void moveRight(uint8_t speed) {
 // Reduce right motor speed to turn right
 setMotorLeft(1, speed); // Left motor normal
 setMotorRight(1, speed - 50); // Right motor slower
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
