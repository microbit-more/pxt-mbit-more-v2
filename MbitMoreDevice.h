#ifndef MBIT_MORE_DEVICE_H
#define MBIT_MORE_DEVICE_H

#include "pxt.h"

#include "MicroBit.h"
#include "MicroBitConfig.h"

#include "MbitMoreCommon.h"

#if MICROBIT_CODAL
#include "MbitMoreService.h"
class MbitMoreService;
#else // MICROBIT_CODAL
#include "MbitMoreServiceDAL.h"
class MbitMoreServiceDAL;
using MbitMoreService = MbitMoreServiceDAL;
#endif // NOT MICROBIT_CODAL

#if MICROBIT_CODAL
#define LIGHT_LEVEL_SAMPLES_SIZE 11
#define ANALOG_IN_SAMPLES_SIZE 5
#else // NOT MICROBIT_CODAL
#define LIGHT_LEVEL_SAMPLES_SIZE 5
#define ANALOG_IN_SAMPLES_SIZE 5
#endif // NOT MICROBIT_CODAL

/**
 * Class definition for the Scratch basic Service.
 * Provides a BLE service for default extension of micro:bit in Scratch3.
 */
class MbitMoreDevice {
private:
  /**
   * Constructor.
   * Create a representation of default extension for Scratch3.
   * @param _uBit The instance of a MicroBit runtime.
   */
  MbitMoreDevice(MicroBit &_uBit);

  /**
   * @brief Destroy the MbitMoreDevice object
   *
   */
  ~MbitMoreDevice();

public:
  MbitMoreDevice(const MbitMoreDevice &) = delete;
  MbitMoreDevice &operator=(const MbitMoreDevice &) = delete;
  MbitMoreDevice(MbitMoreDevice &&) = delete;
  MbitMoreDevice &operator=(MbitMoreDevice &&) = delete;

  static MbitMoreDevice &getInstance() {
    static MbitMoreDevice instance(pxt::uBit);
    return instance;
  }

  MicroBit &uBit;

  /**
   * @brief BLE service for basic micro:bit extension.
   *
   */
  MbitMoreService *basicService;

  /**
   * @brief BLE service for Microbit More extension.
   *
   */
  MbitMoreService *moreService;

  // ---------------------

  int gpioPin[9] = {0, 1, 2, 8, 12, 13, 14, 15, 16};
  int initialPullUp[3] = {0, 1, 2};

  /**
   * @brief Shadow screen to display on the LED.
   *
   */
  uint8_t shadowPixcels[5][5] = {{0}};

  /**
   * Samples of Light Level.
   */
  int lightLevelSamples[LIGHT_LEVEL_SAMPLES_SIZE] = {0};

  /**
   * @brief Last index of the Light Level Samples.
   *
   */
  size_t lightLevelSamplesLast = 0;

  /**
   * Samples of Light Level.
   */
  int analogInSamples[3][ANALOG_IN_SAMPLES_SIZE] = {{0}};

  /**
   * Shared data
   */
  int16_t sharedData[16];

  /**
   * Protocol of microbit more.
   */
  int mbitMoreProtocol;

  /**
   * Current mode of all pins.
   */
#if MICROBIT_CODAL
  PullMode pullMode[sizeof(gpioPin) / sizeof(gpioPin[0])];
#else // MICROBIT_CODAL
  PinMode pullMode[sizeof(gpioPin) / sizeof(gpioPin[0])];
#endif // NOT MICROBIT_CODAL

  /**
   * @brief Set pin configuration for initial.
   *
   */
  void initialConfiguration();

  /**
   * @brief Set pin configuration for release.
   *
   */
  void releaseConfiguration();

  /**
   * @brief Call when a command was received.
   *
   * @param data
   * @param length
   */
  void onCommandReceived(uint8_t *data, size_t length);

  /**
   * @brief Set the pattern on the line of the shadow pixels.
   *
   * @param line Index of the lines to set.
   * @param pattern Array of brightness(0..255) according columns.
   */
  void setPixelsShadowLine(int line, uint8_t *pattern);

  /**
   * @brief Display the shadow pixels on the LED.
   *
   */
  void displayShadowPixels();

  /**
   * @brief Display text on LED.
   *
   * @param text Contents to display with null termination.
   * @param delay The time to delay between characters, in milliseconds.
   */
  void displayText(char *text, int delay);

  /**
   * @brief Update data of sensors.
   *
   * @param data Buffer for BLE characteristics.
   */
  void updateSensors(uint8_t *data);

  /**
   * @brief Update data of direction.
   *
   * @param data Buffer for BLE characteristics.
   */
  void updateDirection(uint8_t *data);

  /**
   * @brief Get data of analog input of the pin.
   *
   * @param data Buffer for BLE characteristics.
   * @param pinIndex Index of the pin [0, 1, 2].
   */
  void updateAnalogIn(uint8_t *data, size_t pinIndex);

  /**
   * @brief Sample current light level and return filtered value.
   *
   * @return int Filtered light level.
   */
  int sampleLigthLevel();

  /**
   * Set value to Slots.
   */
  void setSharedData(int index, int value);

  /**
   * Get value to Slots.
   */
  int getSharedData(int index);

  void onBLEConnected(MicroBitEvent _e);

  void onBLEDisconnected(MicroBitEvent _e);

  /**
   * Callback. Invoked when a pin event sent.
   */
  void onPinEvent(MicroBitEvent evt);

  void displayFriendlyName();

private:
  void listenPinEventOn(int pinIndex, int eventType);

#if MICROBIT_CODAL
  void setPullMode(int pinIndex, PullMode pull);
#else // MICROBIT_CODAL
  void setPullMode(int pinIndex, PinMode pull);
#endif // NOT MICROBIT_CODAL

  void setDigitalValue(int pinIndex, int value);
  void setAnalogValue(int pinIndex, int value);
  void setServoValue(int pinIndex, int angle, int range, int center);
  void setPinModeTouch(int pinIndex);

  void composeBasicData(uint8_t *buff);

  void onButtonChanged(MicroBitEvent);
  void onGestureChanged(MicroBitEvent);

  int normalizeCompassHeading(int heading);
  int convertToTilt(float radians);
};

#endif // MBIT_MORE_DEVICE_H
