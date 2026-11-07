# ESP32 Gesture-Controlled Wireless Mouse Glove

A wearable mouse prototype that uses wrist movement to control a
computer cursor over Bluetooth. Touch sensors provide left-click,
right-click, and scrolling inputs.

> **Status:** Prototype / development stage. Sensor calibration and
> gesture tuning may be required.

## Features

-   Wireless mouse connection using Bluetooth Low Energy (BLE).
-   Wrist tilt controls cursor movement.
-   Touch sensor 1: left click.
-   Touch sensor 2: right click.
-   Both touch sensors together: scroll mode.
-   Adjustable cursor sensitivity, smoothing, and dead zone.

## Hardware

-   ESP32 DevKit V1 (ESP32-WROOM-32)
-   GY-521 motion sensor module (MPU6050-compatible)
-   Two touch sensor modules (such as TTP223)
-   Jumper wires and breadboard/prototype connections
-   USB cable for programming and initial testing
-   Glove or wrist strap for mounting

Optional for a portable version: 3.7 V LiPo battery, TP4056
charging/protection module, suitable regulated power supply/boost
converter, and ON/OFF switch.

**Power safety:** Use USB power for the current prototype. Do not
connect a bare LiPo battery or raw TP4056 battery output directly to the
ESP32 5V/VIN or 3.3V pin. Build and verify the battery power circuit
separately before use.

## Wiring

  Module           Pin   ESP32
  ---------------- ----- ---------
  Motion sensor    VCC   3V3
  Motion sensor    GND   GND
  Motion sensor    SDA   GPIO 21
  Motion sensor    SCL   GPIO 22
  Touch sensor 1   VCC   3V3
  Touch sensor 1   GND   GND
  Touch sensor 1   OUT   GPIO 25
  Touch sensor 2   VCC   3V3
  Touch sensor 2   GND   GND
  Touch sensor 2   OUT   GPIO 26

All modules must share a common GND. Check the labels on your exact
modules before wiring.

## Software Requirements

-   Arduino IDE
-   ESP32 board support package
-   ESP32 BLE Mouse library (T-vK / ESP32-BLE-Mouse)
-   Wire library (included with Arduino)

The prototype reads motion-sensor registers over I2C because the
Adafruit MPU6050 library did not initialize successfully with the module
tested. The reported I2C address was `0x68`. Verify your own module with
an I2C scanner; compatible sensor modules may behave differently.

## Upload Instructions

1.  Install Arduino IDE and ESP32 board support.
2.  Install the ESP32 BLE Mouse library.
3.  Connect the ESP32 to your computer over USB.
4.  Select the matching ESP32 board and serial port.
5.  Open the project `.ino` sketch, compile, and upload it.
6.  If the sketch prints calibration or diagnostic data, open Serial
    Monitor at **115200 baud**.

## Pairing and Usage

1.  Reset the ESP32 after uploading.
2.  Enable Bluetooth on your computer and scan for **Gesture Mouse**.
3.  Pair/connect to the device.
4.  Keep your wrist steady during startup calibration.
5.  Tilt your wrist gently to move the cursor.

### Gesture Controls

  Input                         Action
  ----------------------------- ----------------------------
  Tilt wrist left/right         Horizontal cursor movement
  Tilt wrist forward/backward   Vertical cursor movement
  Touch sensor 1                Left click
  Touch sensor 2                Right click
  Touch both sensors            Scroll

The exact movement directions depend on sensor orientation and the
mapping in the sketch.

## Calibration and Tuning

-   Keep the glove still during startup calibration.
-   Secure the sensor so it cannot move independently of your wrist.
-   Recalibrate after changing the sensor orientation.
-   If the cursor jitters, increase the dead zone or smoothing slightly.
-   If the cursor is too slow, increase sensitivity gradually.
-   Change one tuning value at a time and test it.

## Troubleshooting

**ESP32 does not appear in Bluetooth** - Reset the ESP32 and confirm the
sketch uploaded successfully. - Make sure Bluetooth is enabled on the
computer. - Remove an old pairing and scan again.

**Bluetooth connects but the cursor does not move** - Confirm the BLE
mouse test sketch works. - Check that the integrated sketch sends
movement only after BLE connects. - Check sensor readings and
calibration.

**Motion sensor is not detected** - Run an I2C scanner. - Check SDA =
GPIO 21, SCL = GPIO 22, VCC = 3V3, and common GND. - Confirm the sensor
module/chip before selecting a sensor library.

**Touch controls do not respond** - Check OUT connections on GPIO 25 and
GPIO 26. - Verify whether the touch module output is active HIGH or
active LOW. - Adjust the `TOUCH_ACTIVE` setting in the sketch if
required.

## Suggested Repository Structure

``` text
ESP32-Gesture-Mouse/
├── README.md
├── gesture_mouse/
│   └── gesture_mouse.ino
└── media/
    ├── wiring.jpg
    ├── prototype.jpg
    └── demo.mp4
```

This is an example structure. Rename the folders and files to match your
actual project.

## Future Improvements

-   Design a compact custom PCB using KiCad.
-   Add a safe, regulated rechargeable battery power system.
-   Improve sensor mounting and glove enclosure.
-   Add a calibration button and configurable sensitivity.
-   Improve gesture recognition and power management.

## Safety

-   Disconnect USB power before changing wiring.
-   Check polarity before connecting a battery or power module.
-   Avoid shorting exposed LiPo battery terminals.
-   Measure the regulated supply with a multimeter before connecting it
    to the ESP32.

## Author

Add your name, course/branch, institution, and GitHub profile here.

## License

Choose a license before allowing others to reuse or modify this project.
The MIT License is a common option for open-source software.
# ESP32-Gesture-Mouse
