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

#if MICROBIT_CODAL
#define MBIT_MORE_WAITING_MESSAGE_LENGTH 16
#define MBIT_MORE_WAITING_MESSAGE_NOT_FOUND 0xff
#define MBIT_MORE_MESSAGE_LABEL_SIZE 8
#define MBIT_MORE_MESSAGE_CONTENT_SIZE 11
#endif // MICROBIT_CODAL

enum MbitMoreButtonID
{
  P0 = 24,
  P1 = 25,
  P2 = 26,
  A = 27,
  B = 28,
  LOGO = 29, // Pin number of logo is not corresponded with micro:bit pin assign.
};

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

#if MICROBIT_CODAL
  /**
   * @brief Message in MbitMore
   * 
   */
  typedef struct {
    char label[MBIT_MORE_MESSAGE_LABEL_SIZE];            /** label of the message */
    MbitMoreMessageType type;                            /** type of the content */
    uint8_t content[MBIT_MORE_MESSAGE_CONTENT_SIZE + 1]; /** content of the message */
  } MbitMoreMessage;

  MbitMoreMessage receivedMessages[MBIT_MORE_WAITING_MESSAGE_LENGTH] = {{{0}}};
#endif // MICROBIT_CODAL

  /**
   * Samples of Light Level.
   */
  int analogInSamples[3][ANALOG_IN_SAMPLES_SIZE] = {{0}};

#if MICROBIT_CODAL
  bool micInUse = false;
  float soundLevel = 0.0;
#endif // MICROBIT_CODAL

  /**
   * Protocol of microbit more.
   */
  int mbitMoreProtocol;

  /**
   * Current mode of all pins.
   */
  MbitMorePullMode pullMode[sizeof(gpioPin) / sizeof(gpioPin[0])];

  /**
   * @brief Set pin configuration for initial.
   *
   */
  void initializeConfig();

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
   * @brief Update GPIO and sensors state.
   *
   * @param data Buffer for BLE characteristics.
   */
  void updateState(uint8_t *data);

  /**
   * @brief Update data of motion.
   *
   * @param data Buffer for BLE characteristics.
   */
  void updateMotion(uint8_t *data);

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
   * @brief Set PMW signal to the speaker pin for play tone.
   * 
   * @param period  PWM period (1000000 / frequency)[us]
   * @param volume laudness of the sound [0..255]
   */
  void playTone(int period, int volume);

  /**
   * @brief Stop playing tone.
   * 
   */
  void stopTone();

#if MICROBIT_CODAL
  /**
   * @brief Return message ID for the label
   * 
   * @param messageLabelChar message label
   * @param messageType
   * @return int 
   */
  int findWaitingMessageIndex(const char *messageLabel, MbitMoreMessageType messageType);

  /**
   * @brief Register message label and retrun message ID.
   *
   * @param messageLabel
   * @param messageType
   * @return int ID for the message label
   */
  int registerWaitingMessage(ManagedString messageLabel, MbitMoreMessageType messageType);

  /**
   * @brief Get type of content for the message ID
   *
   * @param messageID
   * @return content type
   */
  MbitMoreMessageType messageType(int messageID);

  /**
   * @brief Return content of the message as number
   *
   * @param messageID
   * @return content of the message
   */
  float messageContentAsNumber(int messageID);

  /**
   * @brief Return content of the message as string
   *
   * @param messageID
   * @return content of the message
   */
  ManagedString messageContentAsText(int messageID);

  /**
   * @brief Send a labeled message with content in float.
   * 
   * @param messageLabel 
   * @param messageContent 
   */
  void sendMessageWithNumber(ManagedString messageLabel, float messageContent);

  /**
   * @brief Send a labeled message with content in string.
   * 
   * @param messageLabel 
   * @param messageContent 
   */
  void sendMessageWithText(ManagedString messageLabel, ManagedString messageContent);

#endif // MICROBIT_CODAL

  void onBLEConnected(MicroBitEvent _e);

  void onBLEDisconnected(MicroBitEvent _e);

  /**
   * Callback. Invoked when a pin event sent.
   */
  void onPinEvent(MicroBitEvent evt);

  void displayFriendlyName();

private:
  void listenPinEventOn(int pinIndex, int eventType);

  /**
   * @brief Set pull-mode.
   * 
   * @param pinIndex index to set
   * @param pull pull-mode to set
   */
  void setPullMode(int pinIndex, MbitMorePullMode pull);

  void setDigitalValue(int pinIndex, int value);
  void setAnalogValue(int pinIndex, int value);
  void setServoValue(int pinIndex, int angle, int range, int center);

  void composeBasicData(uint8_t *buff);

  void onButtonChanged(MicroBitEvent);
  void onGestureChanged(MicroBitEvent);

  int normalizeCompassHeading(int heading);
  int convertToTilt(float radians);
};

#endif // MBIT_MORE_DEVICE_H
