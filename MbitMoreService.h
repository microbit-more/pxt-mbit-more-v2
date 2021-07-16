#include "pxt.h"

#if MICROBIT_CODAL

#ifndef MBIT_MORE_SERVICE_H
#define MBIT_MORE_SERVICE_H

#include "MicroBitConfig.h"

#if CONFIG_ENABLED(DEVICE_BLE)

#include "MicroBit.h"
#include "MicroBitBLEManager.h"
#include "MicroBitBLEService.h"

#include "MbitMoreCommon.h"
#include "MbitMoreDevice.h"

// // Forward declaration
class MbitMoreDevice;

/**
 * Class definition for the Scratch basic Service.
 * Provides a BLE service for default extension of micro:bit in Scratch3.
 */
class MbitMoreService : public MicroBitBLEService, MicroBitComponent {
public:
  // Buffer of characteristic for receiving commands.
  uint8_t commandChBuffer[MM_CH_BUFFER_SIZE_COMMAND] = {0};

  // Buffer of characteristic for sending data of GPIO and sensors state.
  uint8_t stateChBuffer[MM_CH_BUFFER_SIZE_STATE] = {0};

  // Buffer of characteristic for sending data about motion.
  uint8_t motionChBuffer[MM_CH_BUFFER_SIZE_MOTION] = {0};

  // Buffer of characteristic for sending pin events.
  uint8_t pinEventChBuffer[MM_CH_BUFFER_SIZE_NOTIFY] = {0};

  // Buffer of characteristic for sending action events.
  uint8_t actionEventChBuffer[MM_CH_BUFFER_SIZE_NOTIFY] = {0};

  // Buffer of characteristic for sending analog input values of P0.
  uint8_t analogInP0ChBuffer[MM_CH_BUFFER_SIZE_ANALOG_IN] = {0};

  // Buffer of characteristic for sending analog input values of P1.
  uint8_t analogInP1ChBuffer[MM_CH_BUFFER_SIZE_ANALOG_IN] = {0};

  // Buffer of characteristic for sending analog input values of P2.
  uint8_t analogInP2ChBuffer[MM_CH_BUFFER_SIZE_ANALOG_IN] = {0};

  // Buffer of characteristic for sending data.
  uint8_t dataChBuffer[MM_CH_BUFFER_SIZE_NOTIFY] = {0};

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
   * @brief Notify action event.
   */
  void notifyActionEvent();

  /**
   * @brief Notify action event.
   */
  void notifyPinEvent();

  /**
   * @brief Notify sending data to Scratch
   * 
   */
  void notifyData();

  void notify();

  void update();

  /**
   * @brief Register data label and retrun ID for the label.
   *
   * @param dataLabel label to register
   * @param dataType type of the data to be received
   * @return int ID for the label
   */
  int registerWaitingDataLabel(ManagedString dataLabel, MbitMoreDataContentType dataType);

  /**
   * @brief Get type of content for the label
   *
   * @param labelID ID for the label
   * @return type of content [number | string]
   */
  MbitMoreDataContentType dataType(int labelID);

  /**
   * @brief Return content of the data as number
   *
   * @param labelID ID for the label
   * @return content of the data
   */
  float dataContentAsNumber(int labelID);

  /**
   * @brief Return content of the data as string
   *
   * @param labelID ID for the label
   * @return content of the data
   */
  ManagedString dataContentAsText(int labelID);

  /**
   * @brief Send a float with labele to Scratch.
   *  
   * @param dataLabel label of the data
   * @param dataContent content of the data
   */
  void sendNumberWithLabel(ManagedString dataLabel, float dataContent);

  /**
   * @brief Send a string with labele to Scratch.
   * 
   * @param dataLabel label of the data
   * @param dataContent content of the data
   */
  void sendTextWithLabel(ManagedString dataLabel, ManagedString dataContent);

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
    mbitmore_cIdx_STATE,
    mbitmore_cIdx_MOTION,
    mbitmore_cIdx_PIN_EVENT,
    mbitmore_cIdx_ACTION_EVENT,
    mbitmore_cIdx_ANALOG_IN_P0,
    mbitmore_cIdx_ANALOG_IN_P1,
    mbitmore_cIdx_ANALOG_IN_P2,
    mbitmore_cIdx_DATA,
    mbitmore_cIdx_COUNT
  } mbitmore_cIdx;

  // UUIDs for our service and characteristics
  static const uint8_t baseUUID[16];
  static const uint16_t serviceUUID;
  static const uint16_t charUUID[mbitmore_cIdx_COUNT];

  // Data for each characteristic when they are held by Soft Device.
  MicroBitBLEChar chars[mbitmore_cIdx_COUNT];

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
