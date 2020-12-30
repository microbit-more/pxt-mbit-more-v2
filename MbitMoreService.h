#include "pxt.h"

#if MICROBIT_CODAL

#ifndef MBIT_MORE_SERVICE_H
#define MBIT_MORE_SERVICE_H

#include "MicroBitConfig.h"

#if CONFIG_ENABLED(DEVICE_BLE)

#include "MbitMoreCommon.h"
#include "MbitMoreDevice.h"
#include "MicroBit.h"
#include "MicroBitBLEManager.h"
#include "MicroBitBLEService.h"

// // Forward declaration
class MbitMoreDevice;

/**
 * Class definition for the Scratch basic Service.
 * Provides a BLE service for default extension of micro:bit in Scratch3.
 */
class MbitMoreService : public MicroBitBLEService, MicroBitComponent {
public:
  /**
   * Constructor.
   * Create a representation of default extension for Scratch3.
   */
  MbitMoreService();

  /**
   * Invoked when BLE connects.
   */
  void onConnect(const microbit_ble_evt_t *p_ble_evt);

  /**
   * Invoked when BLE disconnects.
   */
  void onDisconnect(const microbit_ble_evt_t *p_ble_evt);

  /**
   * Callback. Invoked when any of our attributes are written via BLE.
   */
  void onDataWritten(const microbit_ble_evt_write_t *params);

  /**
   * Callback. Invoked when any of our attributes are read via BLE.
   * Set  params->data and params->length to update the value
   */
  void onDataRead(microbit_onDataRead_t *params);

  /**
   * Periodic callback from MicroBit idle thread.
   */
  virtual void idleCallback();

  /**
   * @brief Notify data of the basic extension.
   *
   * @param data Data to notify.
   * @param length Length of the data.
   */
  void notifyBasicData(uint8_t *data, uint16_t length);

  /**
   * @brief Notify button event.
   *
   * @param data Data to notify.
   * @param length Lenght of the data.
   */
  void notifyButtonEvent(uint8_t *data, uint16_t length);

  /**
   * @brief Notify IO event.
   *
   * @param data Data to notify.
   * @param length Lenght of the data.
   */
  void notifyIOEvent(uint8_t *data, uint16_t length);

  void notifySharedData();

  void notify();

  void update();

  /**
   * Set value to Slots.
   */
  void setSharedData(int index, int value);

  /**
   * Get value to Slots.
   */
  int getSharedData(int index);

private:
  /**
   * @brief micro:bit runtime object.
   *
   */
  MicroBit &uBit;

  /**
   * @brief Microbit More object.
   *
   */
  MbitMoreDevice *mbitMore;

  // Index for each charactersitic in arrays of handles and UUIDs
  typedef enum mbitmore_cIdx
  {
    mbitmore_cIdx_COMMAND,
    mbitmore_cIdx_DIGITAL_IN,
    mbitmore_cIdx_PIN_EVENT,
    mbitmore_cIdx_BUTTON_EVENT,
    mbitmore_cIdx_LIGHT_LEVEL,
    mbitmore_cIdx_ACCELERATION,
    mbitmore_cIdx_MAGNET,
    mbitmore_cIdx_TEMPERATURE,
    mbitmore_cIdx_MICROPHONE,
    mbitmore_cIdx_ANALOG_IN_P0,
    mbitmore_cIdx_ANALOG_IN_P1,
    mbitmore_cIdx_ANALOG_IN_P2,
    mbitmore_cIdx_SHARED_DATA,
    mbitmore_cIdx_COUNT
  } mbitmore_cIdx;

  // UUIDs for our service and characteristics
  static const uint8_t baseUUID[16];
  static const uint16_t serviceUUID;
  static const uint16_t charUUID[mbitmore_cIdx_COUNT];

  // Data for each characteristic when they are held by Soft Device.
  MicroBitBLEChar chars[mbitmore_cIdx_COUNT];

#define MM_CH_BUFFER_SIZE_DEFAULT 20

  // Buffer of characteristic for receiving commands.
  uint8_t commandBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending digital levels.
  uint8_t digitalInBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending pin events.
  uint8_t pinEventBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending button events.
  uint8_t buttonEventBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending level of light sensor.
  uint8_t lightLevelBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending data of acceleration.
  uint8_t accelerationBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending data of magnet.
  uint8_t magnetBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending data of temperature.
  uint8_t temperatureBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending data of microphone.
  uint8_t microphoneBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending analog input values.
  uint8_t analogIn0Buffer[3][MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending analog input values.
  uint8_t analogIn1Buffer[3][MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending analog input values.
  uint8_t analogIn2Buffer[3][MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending shared data.
  uint8_t sharedDataBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  /**
   * Write IO characteristics.
   */
  void writeDigitalIn();

public:
  int characteristicCount() { return mbitmore_cIdx_COUNT; };
  MicroBitBLEChar *characteristicPtr(int idx) { return &chars[idx]; };
};

#endif // CONFIG_ENABLED(DEVICE_BLE)
#endif // MBIT_MORE_SERVICE_H
#endif // MICROBIT_CODAL
