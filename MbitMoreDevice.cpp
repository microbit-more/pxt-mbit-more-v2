
#include "pxt.h"

#if MICROBIT_CODAL

/**
 * Class definition for the Scratch MicroBit More Service.
 * Provides a BLE service to remotely controll Micro:bit from Scratch3.
 */
#include "MbitMoreDevice.h"

#define MBIT_MORE_BUTTON_PIN_A 5
#define MBIT_MORE_BUTTON_PIN_B 11

#define MBIT_MORE_BUTTON_EVT_DOWN 1
#define MBIT_MORE_BUTTON_EVT_UP 2
#define MBIT_MORE_BUTTON_EVT_CLICK 3
#define MBIT_MORE_BUTTON_EVT_LONG_CLICK 4
#define MBIT_MORE_BUTTON_EVT_HOLD 5
#define MBIT_MORE_BUTTON_EVT_DOUBLE_CLICK 6

int gpio[] = {0,  1,  2,
              5, // button A
              8,
              11, // button B
              13, 14, 15, 16};
int analogIn[] = {0, 1, 2};
int digitalIn[] = {
    0, 1,
    2}; // PullUp at connected to be same behaviour as the standard extension.

/**
 * Constructor.
 * Create a representation of the device for Microbit More service.
 * @param _uBit The instance of a MicroBit runtime.
 */
MbitMoreDevice::MbitMoreDevice(MicroBit &_uBit) : uBit(_uBit) {
  // Calibrate the compass before start bluetooth service.
  // if (!uBit.compass.isCalibrated()) {
  //   uBit.compass.calibrate();
  // }

  uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onButtonChanged,
                         MESSAGE_BUS_LISTENER_IMMEDIATE);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onButtonChanged,
                         MESSAGE_BUS_LISTENER_IMMEDIATE);
  uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onGestureChanged,
                         MESSAGE_BUS_LISTENER_IMMEDIATE);
}

MbitMoreDevice::~MbitMoreDevice() {
  uBit.messageBus.ignore(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onButtonChanged);
  uBit.messageBus.ignore(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onButtonChanged);
  uBit.messageBus.ignore(MICROBIT_ID_GESTURE, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onGestureChanged);
  uBit.messageBus.ignore(MICROBIT_ID_ANY, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onPinEvent);
  delete basicService;
}

void MbitMoreDevice::initConfiguration() {
  // Initialize pin configuration.
  for (size_t i = 0; i < sizeof(digitalIn) / sizeof(digitalIn[0]); i++) {
    setPullMode(digitalIn[i], PullMode::Up);
  }
  uBit.display.stopAnimation(); // To stop display friendly name.
  // uBit.display.scrollAsync("v.0.6.0"); // Display version number
  // MicroBitImage smiley(
  //     "0,255,0,255, "
  //     "0\n0,255,0,255,0\n0,0,0,0,0\n255,0,0,0,255\n0,255,255,255,0\n");
  // uBit.display.animateAsync(smiley, 3000, 5);
  uBit.display.print("M");
}

/**
 * Make it listen events of the event type on the pin.
 * Remove listener if the event type is MICROBIT_PIN_EVENT_NONE.
 */
void MbitMoreDevice::listenPinEventOn(int pinIndex, int eventType) {
  int componentID;  // ID of the MicroBit Component that generated the event.
  switch (pinIndex) // Index of pin to set event.
  {
  case 0:
    componentID = MICROBIT_ID_IO_P0;
    break;
  case 1:
    componentID = MICROBIT_ID_IO_P1;
    break;
  case 2:
    componentID = MICROBIT_ID_IO_P2;
    break;
  case 8:
    componentID = MICROBIT_ID_IO_P8;
    break;
  case 13:
    componentID = MICROBIT_ID_IO_P13;
    break;
  case 14:
    componentID = MICROBIT_ID_IO_P14;
    break;
  case 15:
    componentID = MICROBIT_ID_IO_P15;
    break;
  case 16:
    componentID = MICROBIT_ID_IO_P16;
    break;

  default:
    return;
  }
  if (eventType == MbitMorePinEventType::NONE) {
    uBit.messageBus.ignore(componentID, MICROBIT_EVT_ANY, this,
                           &MbitMoreDevice::onPinEvent);
    uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_NONE);
  } else {
    uBit.messageBus.listen(componentID, MICROBIT_EVT_ANY, this,
                           &MbitMoreDevice::onPinEvent,
                           MESSAGE_BUS_LISTENER_DROP_IF_BUSY);
    if (eventType == MbitMorePinEventType::ON_EDGE) {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_EDGE);
    } else if (eventType == MbitMorePinEventType::ON_PULSE) {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_PULSE);
    } else if (eventType == MbitMorePinEventType::ON_TOUCH) {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_TOUCH);
    }
  }
}

/**
 * Callback. Invoked when a pin event sent.
 */
void MbitMoreDevice::onPinEvent(MicroBitEvent evt) {
  uint8_t pinIndex;
  switch (evt.source) // ID of the MicroBit Component that generated the event.
                      // (uint16_t)
  {
  case MICROBIT_ID_IO_P0:
    eventBuffer[0] = 0;
    break;
  case MICROBIT_ID_IO_P1:
    eventBuffer[0] = 1;
    break;
  case MICROBIT_ID_IO_P2:
    eventBuffer[0] = 2;
    break;
  case MICROBIT_ID_IO_P8:
    eventBuffer[0] = 8;
    break;
  case MICROBIT_ID_IO_P13:
    eventBuffer[0] = 13;
    break;
  case MICROBIT_ID_IO_P14:
    eventBuffer[0] = 14;
    break;
  case MICROBIT_ID_IO_P15:
    eventBuffer[0] = 15;
    break;
  case MICROBIT_ID_IO_P16:
    eventBuffer[0] = 16;
    break;

  default:
    break;
  }

  // event ID is sent as uint16_t little-endian.
  // #define MICROBIT_PIN_EVT_RISE               2
  // #define MICROBIT_PIN_EVT_FALL               3
  // #define MICROBIT_PIN_EVT_PULSE_HI           4
  // #define MICROBIT_PIN_EVT_PULSE_LO           5
  memcpy(&(eventBuffer[1]), &(evt.value), 2);

  // event timestamp is sent as uint32_t little-endian coerced from uint64_t
  // value.
  uint32_t timestamp = (uint32_t)evt.timestamp;
  memcpy(&(eventBuffer[3]), &timestamp, 4);

  moreService->notifyIOEvent((uint8_t *)&eventBuffer,
                             sizeof(eventBuffer) / sizeof(eventBuffer[0]));
}

/**
 * Button update callback
 */
void MbitMoreDevice::onButtonChanged(MicroBitEvent evt) {
  int componentID;
  // Component ID that generated the event.
  switch (evt.source) {
  case MICROBIT_ID_BUTTON_A:
    buttonEvent[0] = MBIT_MORE_BUTTON_PIN_A;
    break;

  case MICROBIT_ID_BUTTON_B:
    buttonEvent[0] = MBIT_MORE_BUTTON_PIN_B;
    break;

  default:
    return;
    break;
  }
  // Event ID.
  switch (evt.value) {
  case MICROBIT_BUTTON_EVT_DOWN:
    buttonEvent[1] = MBIT_MORE_BUTTON_EVT_DOWN;
    break;

  case MICROBIT_BUTTON_EVT_UP:
    buttonEvent[1] = MBIT_MORE_BUTTON_EVT_UP;
    break;

  case MICROBIT_BUTTON_EVT_CLICK:
    buttonEvent[1] = MBIT_MORE_BUTTON_EVT_CLICK;
    break;

  case MICROBIT_BUTTON_EVT_LONG_CLICK:
    buttonEvent[1] = MBIT_MORE_BUTTON_EVT_LONG_CLICK;
    break;

  case MICROBIT_BUTTON_EVT_HOLD:
    buttonEvent[1] = MBIT_MORE_BUTTON_EVT_HOLD;
    break;

  case MICROBIT_BUTTON_EVT_DOUBLE_CLICK:
    buttonEvent[1] = MBIT_MORE_BUTTON_EVT_DOUBLE_CLICK;
    break;

  default:
    break;
  }
  // Timestamp of the event.
  memcpy(&(buttonEvent[3]), &evt.timestamp, 4);
  moreService->notifyButtonEvent((uint8_t *)&buttonEvent,
                                 sizeof(buttonEvent) / sizeof(buttonEvent[0]));
}

void MbitMoreDevice::onGestureChanged(MicroBitEvent e) {
  if (e.value == MICROBIT_ACCELEROMETER_EVT_SHAKE) {
    gesture = gesture | 1;
  }
  if (e.value == MICROBIT_ACCELEROMETER_EVT_FREEFALL) {
    gesture = gesture | 1 << 1;
  }
}

/**
 * Normalize angle in upside down.
 */
int MbitMoreDevice::normalizeCompassHeading(int heading) {
  if (uBit.accelerometer.getZ() > 0) {
    if (heading <= 180) {
      heading = 180 - heading;
    } else {
      heading = 360 - (heading - 180);
    }
  }
  return heading;
}

/**
 * Convert roll/pitch radians to Scratch extension value (-1000 to 1000).
 */
int MbitMoreDevice::convertToTilt(float radians) {
  float degrees = (360.0f * radians) / (2.0f * PI);
  float tilt = degrees * 1.0f / 90.0f;
  if (degrees > 0) {
    if (tilt > 1.0f)
      tilt = 2.0f - tilt;
  } else {
    if (tilt < -1.0f)
      tilt = -2.0f - tilt;
  }
  return (int)(tilt * 1000.0f);
}

void MbitMoreDevice::updateGesture() {
  int old[] = {lastAcc[0], lastAcc[1], lastAcc[2]};
  lastAcc[0] = uBit.accelerometer.getX();
  lastAcc[1] = uBit.accelerometer.getY();
  lastAcc[2] = uBit.accelerometer.getZ();
  if ((gesture >> 2) & 1) {
    gesture = gesture ^ (1 << 2);
    return;
  }
  int threshold = 50;
  if ((abs(lastAcc[0] - old[0]) > threshold) ||
      (abs(lastAcc[1] - old[1]) > threshold) ||
      (abs(lastAcc[2] - old[2]) > threshold)) {
    // Moved
    gesture = gesture | (1 << 2);
  }
}

void MbitMoreDevice::resetGesture() {
  gesture =
      gesture & (1 << 2); // Save moved state to detect continuous movement.
}

void MbitMoreDevice::updateDigitalValues() {
  digitalValues = 0;
  for (size_t i = 0; i < sizeof(gpio) / sizeof(gpio[0]); i++) {
    if (uBit.io.pin[gpio[i]].isDigital()) {
      if (uBit.io.pin[gpio[i]].isInput()) {
        digitalValues =
            digitalValues | (uBit.io.pin[gpio[i]].getDigitalValue() << gpio[i]);
      }
    }
  }
}

void MbitMoreDevice::updateAnalogValues() {
  for (size_t i = 0; i < sizeof(analogIn) / sizeof(analogIn[0]); i++) {
    int samplingCount;
    int prevValue;
    int value;
    if (uBit.io.pin[analogIn[i]].isInput()) {
      uBit.io.pin[analogIn[i]].setPull(PullMode::None);
      // for accuracy, read more than 2 times to get same values continuously
      do {
        prevValue = value;
        value = (uint16_t)uBit.io.pin[analogIn[i]].getAnalogValue();
        samplingCount++;
      } while (prevValue != value || samplingCount < 4);
      analogValues[i] = value;
      setPullMode(analogIn[i], pullMode[analogIn[i]]);
    }
  }

  //// It will cause flickering LED.
  // uBit.display.disable();
  // analogValues[3] = (uint16_t)uBit.io.P3.getAnalogValue();
  // analogValues[4] = (uint16_t)uBit.io.P4.getAnalogValue();
  // analogValues[5] = (uint16_t)uBit.io.P10.getAnalogValue();
  // uBit.display.enable();
}

void MbitMoreDevice::updateLightSensor() {
  if (lightSensingDuration <= 0) {
    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE);
    return;
  }
  lightLevel = uBit.display.readLightLevel();
  if (lightSensingDuration < 255) // over 255 means no-limited.
  {
    lightSensingDuration--;
  }
}

void MbitMoreDevice::updateAccelerometer() {
  acceleration[0] =
      -uBit.accelerometer.getX(); // Face side is positive in Z-axis.
  acceleration[1] = uBit.accelerometer.getY();
  acceleration[2] =
      -uBit.accelerometer.getZ(); // Face side is positive in Z-axis.
  rotation[0] = uBit.accelerometer.getPitchRadians();
  rotation[1] = uBit.accelerometer.getRollRadians();
}

void MbitMoreDevice::updateMagnetometer() {
  compassHeading = uBit.compass.heading();
  magneticForce[0] = uBit.compass.getX();
  magneticForce[1] = uBit.compass.getY();
  magneticForce[2] = uBit.compass.getZ();
}

void MbitMoreDevice::setPullMode(int pinIndex, PullMode pull) {
  uBit.io.pin[pinIndex].getDigitalValue(pull);
  pullMode[pinIndex] = pull;
}

void MbitMoreDevice::setDigitalValue(int pinIndex, int value) {
  uBit.io.pin[pinIndex].setDigitalValue(value);
}

void MbitMoreDevice::setAnalogValue(int pinIndex, int value) {
  uBit.io.pin[pinIndex].setAnalogValue(value);
}

void MbitMoreDevice::setServoValue(int pinIndex, int angle, int range,
                                   int center) {
  uBit.io.pin[pinIndex].setServoValue(angle, range, center);
}

void MbitMoreDevice::setPinModeTouch(int pinIndex) {
  uBit.io.pin[pinIndex].isTouched(); // Configure to touch mode then the return
                                     // value is not used.
}

void MbitMoreDevice::setLightSensingDuration(int duration) {
  lightSensingDuration = duration;
}

void MbitMoreDevice::composeBasicData(uint8_t *buff) {
  // Tilt value is sent as int16_t big-endian.
  int16_t tiltX = (int16_t)convertToTilt(rotation[1]);
  buff[0] = (tiltX >> 8) & 0xFF;
  buff[1] = tiltX & 0xFF;
  int16_t tiltY = (int16_t)convertToTilt(rotation[0]);
  buff[2] = (tiltY >> 8) & 0xFF;
  buff[3] = tiltY & 0xFF;
  buff[4] = (uint8_t)((digitalValues >> MBIT_MORE_BUTTON_PIN_A) & 1);
  buff[5] = (uint8_t)((digitalValues >> MBIT_MORE_BUTTON_PIN_B) & 1);
  buff[6] = (uint8_t)(((digitalValues >> 0) & 1) ^ 1);
  buff[7] = (uint8_t)(((digitalValues >> 1) & 1) ^ 1);
  buff[8] = (uint8_t)(((digitalValues >> 2) & 1) ^ 1);
  buff[9] = (uint8_t)gesture;
}

/**
 * Notify shared data to Scratch3
 */
void MbitMoreDevice::notifySharedData() {
  for (size_t i = 0; i < sizeof(sharedData) / sizeof(sharedData[0]); i++) {
    memcpy(&(sharedBuffer[(i * 2)]), &sharedData[i], 2);
  }
  // moreService->notifySharedData(
  //     (uint8_t *)&sharedBuffer,
  //     sizeof(sharedBuffer) / sizeof(sharedBuffer[0]));
}

/**
 * Notify default micro:bit data to Scratch.
 */
void MbitMoreDevice::notifyBasicData() {
  composeBasicData(txBuffer);
  basicService->notifyBasicData((uint8_t *)&txBuffer,
                                sizeof(txBuffer) / sizeof(txBuffer[0]));
}


/**
 * @brief Layer pattern on LED.
 * 0 value in the pattern is hold previous brightness.
 *
 * @param pattern Matrix to display 5 columns x 5 rows.
 * @param brightness Brightness level (0-255) of this layer.
 */
void MbitMoreDevice::layerPattern(uint8_t *pattern, uint8_t brightness) {
  uBit.display.stopAnimation();
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      if (pattern[y] & (0x01 << x)) {
        uBit.display.image.setPixelValue(x, y, brightness);
      }
    }
  }
}

/**
 * @brief Display pattern on LED.
 *
 * @param pattern Matrix to display 5 columns x 5 rows.
 */
void MbitMoreDevice::displayPattern(uint8_t *pattern) {
  uBit.display.stopAnimation();
  uBit.display.clear();
  layerPattern(pattern, 255);
}

/**
 * @brief Display text on LED.
 *
 * @param text Contents to display with null termination.
 * @param delay The time to delay between characters, in milliseconds.
 */
void MbitMoreDevice::displayText(char *text, int delay) {
  ManagedString mstr(text);
  uBit.display.stopAnimation();
  // Interval=120 is corresponding with the standard extension.
  uBit.display.scrollAsync(mstr, delay);
}

/**
 * Set value to shared data.
 * shared data (0, 1, 2, 3)
 */
void MbitMoreDevice::setSharedData(int index, int value) {
  // value (-32768 to 32767) is sent as int16_t little-endian.
  int16_t data = (int16_t)value;
  sharedData[index] = data;
  notifySharedData();
}

/**
 * Get value of a shared data.
 * shared data (0, 1, 2, 3)
 */
int MbitMoreDevice::getSharedData(int index) {
  return (int)(sharedData[index]);
}

/**
 * Update sensors.
 */
void MbitMoreDevice::update() {
  updateDigitalValues();
  updateLightSensor();
  updateAccelerometer();
  updateMagnetometer();
  writeSensors();
}

/**
 * Write shared data characteristics.
 */
void MbitMoreDevice::writeSharedData() {
  for (size_t i = 0; i < sizeof(sharedData) / sizeof(sharedData[0]); i++) {
    memcpy(&(sharedBuffer[(i * 2)]), &sharedData[i], 2);
  }

  // moreService->writeSharedData((uint8_t *)&sharedBuffer,
  //                              sizeof(sharedBuffer) /
  //                              sizeof(sharedBuffer[0]));
}

/**
 * Write data of all sensors to the characteristic.
 */
void MbitMoreDevice::writeSensors() {
  // Accelerometer
  int16_t acc;
  // Acceleration X [milli-g] is sent as int16_t little-endian.
  acc = (int16_t)acceleration[0];
  memcpy(&(sensorsBuffer[0]), &acc, 2);
  // Acceleration Y [milli-g] is sent as int16_t little-endian.
  acc = (int16_t)acceleration[1];
  memcpy(&(sensorsBuffer[2]), &acc, 2);
  // Acceleration Z [milli-g] is sent as int16_t little-endian.
  acc = (int16_t)acceleration[2];
  memcpy(&(sensorsBuffer[4]), &acc, 2);

  int16_t rot;
  // Pitch (radians / 1000) is sent as int16_t little-endian [6..7].
  rot = (int16_t)(rotation[0] * 1000);
  memcpy(&(sensorsBuffer[6]), &rot, 2);
  // Roll (radians / 1000) is sent as int16_t little-endian [8..9].
  rot = (int16_t)(rotation[1] * 1000);
  memcpy(&(sensorsBuffer[8]), &rot, 2);

  // Magnetometer
  uint16_t heading = (uint16_t)normalizeCompassHeading(compassHeading);
  memcpy(&(sensorsBuffer[10]), &heading, 2);

  int16_t force;
  // Magnetic force X (micro-teslas) is sent as uint16_t little-endian [2..3].
  force = (int16_t)(magneticForce[0] / 1000);
  memcpy(&(sensorsBuffer[12]), &force, 2);
  // Magnetic force Y (micro-teslas) is sent as uint16_t little-endian [4..5].
  force = (int16_t)(magneticForce[1] / 1000);
  memcpy(&(sensorsBuffer[14]), &force, 2);
  // Magnetic force Z (micro-teslas) is sent as uint16_t little-endian [6..7].
  force = (int16_t)(magneticForce[2] / 1000);
  memcpy(&(sensorsBuffer[16]), &force, 2);

  // Light sensor
  sensorsBuffer[18] = (uint8_t)lightLevel;

  // Temperature
  sensorsBuffer[19] = (uint8_t)(uBit.thermometer.getTemperature() + 128);

  // moreService->writeSensor((uint8_t *)&sensorsBuffer,
  //                          sizeof(sensorsBuffer) / sizeof(sensorsBuffer[0]));
}

void MbitMoreDevice::displayFriendlyName() {
  ManagedString sign(" -MORE 0.6.0- ");
  uBit.display.scrollAsync(ManagedString(microbit_friendly_name()) + sign, 120);
}

#endif // MICROBIT_CODAL
