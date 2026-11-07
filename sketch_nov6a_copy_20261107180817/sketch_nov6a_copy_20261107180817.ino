
#include <Wire.h>
#include <BleMouse.h>
#include <math.h>

// ==========================================
// ESP32 GESTURE MOUSE - FAST CURSOR EDITION
// ==========================================

// PIN CONFIGURATION

#define SDA_PIN 21
#define SCL_PIN 22

#define TOUCH_LEFT 25
#define TOUCH_RIGHT 26

#define TOUCH_ACTIVE HIGH

#define MPU_ADDR 0x68

// BLUETOOTH

BleMouse bleMouse("Gesture Mouse", "DIY", 100);

// FAST CURSOR SETTINGS

const float DEAD_ZONE = 4.0;
const float SENSITIVITY = 2.5;
const float SMOOTHING = 0.25;

const int MAX_SPEED = 20;

// SCROLL SETTINGS

const float SCROLL_DEAD_ZONE = 4.5;

const unsigned long SCROLL_INTERVAL = 120;
const unsigned long CHORD_DELAY = 180;

// CALIBRATION

float neutralRoll = 0;
float neutralPitch = 0;

float filteredRoll = 0;
float filteredPitch = 0;

// TOUCH STATES

bool leftPressed = false;
bool rightPressed = false;

unsigned long touchStart = 0;
unsigned long lastScroll = 0;

bool previousConnection = false;

// ==========================================
// MPU6050 RAW I2C FUNCTIONS
// ==========================================

void writeRegister(byte reg, byte value) {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// READ ACCELEROMETER

bool readAcceleration(
  int16_t &ax,
  int16_t &ay,
  int16_t &az
) {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(MPU_ADDR, 6) != 6) {
    return false;
  }

  ax = (int16_t)(
    (Wire.read() << 8) | Wire.read()
  );

  ay = (int16_t)(
    (Wire.read() << 8) | Wire.read()
  );

  az = (int16_t)(
    (Wire.read() << 8) | Wire.read()
  );

  return true;
}

// ==========================================
// ANGLE CALCULATION
// ==========================================

bool readAngles(float &roll, float &pitch) {

  int16_t ax, ay, az;

  if (!readAcceleration(ax, ay, az)) {
    return false;
  }

  roll = atan2(
    (float)ay,
    (float)az
  ) * 180.0 / PI;

  pitch = atan2(
    -(float)ax,
    sqrt(
      (float)ay * ay +
      (float)az * az
    )
  ) * 180.0 / PI;

  return true;
}

// ==========================================
// SENSOR CALIBRATION
// ==========================================

void calibrateSensor() {

  Serial.println();
  Serial.println("Keep your wrist straight and relaxed.");
  Serial.println("Calibrating MPU sensor...");

  float rollSum = 0;
  float pitchSum = 0;

  int count = 0;

  for (int i = 0; i < 150; i++) {

    float roll, pitch;

    if (readAngles(roll, pitch)) {

      rollSum += roll;
      pitchSum += pitch;

      count++;
    }

    delay(20);
  }

  if (count == 0) {

    Serial.println("MPU SENSOR ERROR!");

    while (true) {
      delay(1000);
    }
  }

  neutralRoll = rollSum / count;
  neutralPitch = pitchSum / count;

  filteredRoll = 0;
  filteredPitch = 0;

  Serial.println("Calibration completed!");

  Serial.print("Neutral Roll: ");
  Serial.println(neutralRoll);

  Serial.print("Neutral Pitch: ");
  Serial.println(neutralPitch);
}

// ==========================================
// FAST AND SMOOTH CURSOR MOVEMENT
// ==========================================

void moveCursor() {

  float roll, pitch;

  if (!readAngles(roll, pitch)) {
    return;
  }

  // Calculate tilt relative to neutral position

  float rawX = roll - neutralRoll;
  float rawY = pitch - neutralPitch;

  // Apply exponential smoothing

  filteredRoll +=
    SMOOTHING * (rawX - filteredRoll);

  filteredPitch +=
    SMOOTHING * (rawY - filteredPitch);

  float tiltX = filteredRoll;
  float tiltY = filteredPitch;

  // Apply dead zone

  if (fabs(tiltX) < DEAD_ZONE) {

    tiltX = 0;

  } else {

    tiltX -= copysign(DEAD_ZONE, tiltX);
  }

  if (fabs(tiltY) < DEAD_ZONE) {

    tiltY = 0;

  } else {

    tiltY -= copysign(DEAD_ZONE, tiltY);
  }

  // Convert tilt into mouse movement

  int moveX = round(tiltX * SENSITIVITY);
  int moveY = round(tiltY * SENSITIVITY);

  // Limit cursor speed

  moveX = constrain(
    moveX,
    -MAX_SPEED,
    MAX_SPEED
  );

  moveY = constrain(
    moveY,
    -MAX_SPEED,
    MAX_SPEED
  );

  // Send mouse movement

  if (moveX != 0 || moveY != 0) {

    bleMouse.move(
      moveX,
      -moveY,
      0
    );
  }
}

// ==========================================
// RELEASE MOUSE BUTTONS
// ==========================================

void releaseButtons() {

  if (leftPressed) {

    bleMouse.release(MOUSE_LEFT);
    leftPressed = false;
  }

  if (rightPressed) {

    bleMouse.release(MOUSE_RIGHT);
    rightPressed = false;
  }
}

// ==========================================
// TOUCH SENSOR CONTROL
// ==========================================

void handleTouch() {

  bool touch1 =
    digitalRead(TOUCH_LEFT) == TOUCH_ACTIVE;

  bool touch2 =
    digitalRead(TOUCH_RIGHT) == TOUCH_ACTIVE;

  // BOTH TOUCHES = SCROLL MODE

  if (touch1 && touch2) {

    releaseButtons();
    touchStart = 0;

    if (
      millis() - lastScroll >=
      SCROLL_INTERVAL
    ) {

      float roll, pitch;

      if (readAngles(roll, pitch)) {

        float tilt = pitch - neutralPitch;

        if (tilt > SCROLL_DEAD_ZONE) {

          bleMouse.move(0, 0, -1);
        }

        else if (tilt < -SCROLL_DEAD_ZONE) {

          bleMouse.move(0, 0, 1);
        }
      }

      lastScroll = millis();
    }

    return;
  }

  // NO TOUCH = RELEASE BUTTONS

  if (!touch1 && !touch2) {

    releaseButtons();
    touchStart = 0;

    return;
  }

  // WAIT FOR SECOND TOUCH

  if (touchStart == 0) {

    touchStart = millis();
  }

  if (
    millis() - touchStart <
    CHORD_DELAY
  ) {

    return;
  }

  // TOUCH 1 = LEFT CLICK

  if (touch1 && !touch2) {

    if (!leftPressed) {

      bleMouse.press(MOUSE_LEFT);
      leftPressed = true;

      Serial.println("LEFT CLICK");
    }
  }

  // TOUCH 2 = RIGHT CLICK

  if (touch2 && !touch1) {

    if (!rightPressed) {

      bleMouse.press(MOUSE_RIGHT);
      rightPressed = true;

      Serial.println("RIGHT CLICK");
    }
  }
}

// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("ESP32 GESTURE MOUSE");
  Serial.println("FAST CURSOR EDITION");
  Serial.println("==============================");

  // Touch sensor configuration

  pinMode(TOUCH_LEFT, INPUT);
  pinMode(TOUCH_RIGHT, INPUT);

  // Initialize I2C

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  // Check sensor connection

  Wire.beginTransmission(MPU_ADDR);

  if (Wire.endTransmission() != 0) {

    Serial.println("MPU SENSOR NOT FOUND!");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("MPU I2C connected!");

  // Wake up sensor

  writeRegister(0x6B, 0x00);
  delay(100);

  // Set accelerometer range to +/- 2g

  writeRegister(0x1C, 0x00);
  delay(100);

  // Calibrate sensor

  calibrateSensor();

  // Start Bluetooth mouse

  Serial.println("Starting Bluetooth...");

  bleMouse.begin();

  Serial.println("Bluetooth initialized!");
  Serial.println("Device name: Gesture Mouse");
}

// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

  bool connected = bleMouse.isConnected();

  // Detect Bluetooth connection changes

  if (connected != previousConnection) {

    previousConnection = connected;

    if (connected) {

      Serial.println("Bluetooth CONNECTED!");
    }

    else {

      Serial.println("Bluetooth DISCONNECTED!");

      releaseButtons();
    }
  }

  if (connected) {

    bool bothTouched =
      digitalRead(TOUCH_LEFT) == TOUCH_ACTIVE &&
      digitalRead(TOUCH_RIGHT) == TOUCH_ACTIVE;

    // Stop cursor movement in scroll mode

    if (!bothTouched) {

      moveCursor();
    }

    handleTouch();
  }

  delay(20);
}
