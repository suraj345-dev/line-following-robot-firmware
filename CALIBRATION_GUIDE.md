# Line Following Robot - Calibration & Tuning Guide

## 1. SENSOR CALIBRATION

### Step 1: Verify Sensor Type
Your IR sensors are connected to **digital pins 2 & 3**, which means you're using:
- **Digital IR sensors** (output HIGH/LOW) - OR
- **Analog IR sensors with A0/A1 mapped to D2/D3** (need verification)

### Step 2: Test Sensor Output
Upload this test sketch to check sensor readings:

```cpp
void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);  // Left IR
  pinMode(3, INPUT);  // Right IR
}

void loop() {
  Serial.print("Left: ");
  Serial.print(digitalRead(2));
  Serial.print(" | Right: ");
  Serial.println(digitalRead(3));
  delay(200);
}
```

**Expected Output:**
- Place sensor on **WHITE surface**: Should read HIGH (1)
- Place sensor on **BLACK line**: Should read LOW (0)
- Place sensor on **edge of line**: Should transition clearly

### Step 3: Adjust Sensor Positioning
- **Height**: Position sensors 3-5mm above the surface
- **Alignment**: Center sensors on the robot's direction
- **Angle**: Keep sensors perpendicular to the surface
- **Spacing**: Typically 40-50mm apart for optimal line detection

---

## 2. MOTOR CALIBRATION

### Step 1: Check Motor Polarity
Run each motor individually at 200 speed:

```cpp
void setup() {
  pinMode(11, OUTPUT);  // ENA
  pinMode(10, OUTPUT);  // IN1
  pinMode(9, OUTPUT);   // IN2
  pinMode(6, OUTPUT);   // ENB
  pinMode(8, OUTPUT);   // IN3
  pinMode(7, OUTPUT);   // IN4
}

void loop() {
  // Test left motor forward
  digitalWrite(10, HIGH);
  digitalWrite(9, LOW);
  analogWrite(11, 200);
  delay(2000);
  
  // Test right motor forward
  digitalWrite(8, HIGH);
  digitalWrite(7, LOW);
  analogWrite(6, 200);
  delay(2000);
}
```

✓ Both motors should spin **forward at same speed**
✗ If speeds differ → adjust PWM values (BASE_SPEED, CORRECTION_SPEED)

### Step 2: Speed Matching
If motors spin at different speeds naturally:
- **Faster left motor**: Reduce BASE_SPEED for left, or increase for right
- **Faster right motor**: Reduce BASE_SPEED for right, or increase for left

Modify in code:
```cpp
// Alternative: Different speeds per motor
const uint8_t LEFT_BASE_SPEED = 190;
const uint8_t RIGHT_BASE_SPEED = 200;
```

---

## 3. TUNING PARAMETERS

### Parameter Effects Table

| Parameter | Effect | Adjust if... |
|-----------|--------|-------------|
| `BASE_SPEED` (200) | Forward speed | Robot too slow → increase, too shaky → decrease |
| `CORRECTION_SPEED` (150) | Turn speed | Sharp turns → increase, oscillates → decrease |
| `MIN_SPEED` (100) | Motor start threshold | Motors don't start → increase |

### Common Issues & Fixes

#### **Issue: Robot Zig-Zags Excessively**
- **Cause**: Response too aggressive
- **Fix**: 
  1. Reduce `CORRECTION_SPEED` from 150 to 120
  2. Reduce sensor speed difference (e.g., 200 vs 130 instead of 200 vs 100)

#### **Issue: Robot Drifts Off Line Slowly**
- **Cause**: Corrections too weak
- **Fix**:
  1. Increase `CORRECTION_SPEED` from 150 to 180
  2. Widen speed difference (e.g., 200 vs 100)
  3. Check motor speed matching

#### **Issue: Robot Doesn't Start Moving**
- **Cause**: Speed too low to overcome friction
- **Fix**: Increase `BASE_SPEED` to 220-240

#### **Issue: Robot Spins in Place**
- **Cause**: Motor polarity reversed
- **Fix**: Check motor connections and IN1/IN2, IN3/IN4 logic

#### **Issue: Uneven Turns (Pulls Left/Right)**
- **Cause**: Motor speed mismatch
- **Fix**: 
  1. Run motor calibration test
  2. Adjust PWM values individually per motor

---

## 4. ADVANCED TUNING

### PID Control Option (Optional)
For smoother line following, implement PID:

```cpp
float Kp = 100, Ki = 0, Kd = 50;
float error = 0, lastError = 0, integral = 0;

void updateLineFollowingPID() {
  int leftOnLine = (sensorLeftValue == LOW) ? 1 : 0;
  int rightOnLine = (sensorRightValue == LOW) ? 1 : 0;
  
  error = leftOnLine - rightOnLine; // -1, 0, or +1
  integral += error;
  float derivative = error - lastError;
  
  float steering = (Kp * error) + (Ki * integral) + (Kd * derivative);
  lastError = error;
  
  // Apply steering to motors
  uint8_t leftSpeed = BASE_SPEED - steering;
  uint8_t rightSpeed = BASE_SPEED + steering;
  
  setMotorLeft(1, constrain(leftSpeed, 0, 255));
  setMotorRight(1, constrain(rightSpeed, 0, 255));
}
```

Tune `Kp`, `Ki`, `Kd` values for smoother response.

### Sensor Hysteresis
If sensors flicker, add debouncing:

```cpp
const uint8_t DEBOUNCE_COUNT = 3;
uint8_t leftDebounce = 0, rightDebounce = 0;

void readSensorsWithDebounce() {
  int rawLeft = digitalRead(IRSensorLeft);
  int rawRight = digitalRead(IRSensorRight);
  
  if (rawLeft == LOW) leftDebounce++; else leftDebounce = 0;
  if (rawRight == LOW) rightDebounce++; else rightDebounce = 0;
  
  sensorLeftValue = (leftDebounce >= DEBOUNCE_COUNT) ? LOW : HIGH;
  sensorRightValue = (rightDebounce >= DEBOUNCE_COUNT) ? LOW : HIGH;
}
```

---

## 5. TESTING CHECKLIST

- [ ] Sensors detect black line clearly (LOW output)
- [ ] Sensors detect white surface clearly (HIGH output)
- [ ] Both motors move forward at similar speeds
- [ ] Robot doesn't zig-zag when on center of line
- [ ] Robot corrects when drifting left
- [ ] Robot corrects when drifting right
- [ ] Robot smoothly follows curved paths
- [ ] No oscillation at normal speeds

---

## 6. DEBUGGING OUTPUT

Uncomment `debugOutput()` in the main loop to see real-time sensor values and state:

```cpp
// In loop():
// debugOutput();
```

This will print sensor readings and current state every 500ms to Serial Monitor.

---

## 7. FINAL PERFORMANCE CHECKLIST

- **Speed**: 200+ BASE_SPEED for visible movement
- **Accuracy**: Follows line without significant zig-zag
- **Stability**: Smooth, continuous motion
- **Recovery**: Returns to line within 100ms of deviation
