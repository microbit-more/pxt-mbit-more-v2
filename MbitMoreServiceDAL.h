#include "pxt.h"

#if !MICROBIT_CODAL

#ifndef MBIT_MORE_SERVICE_DAL_H
#define MBIT_MORE_SERVICE_DAL_H

#include "MicroBit.h"

#include "MbitMoreCommon.h"
#include "MbitMoreDevice.h"

// // Forward declaration
class MbitMoreDevice;

#define MM_CH_BUFFER_SIZE_DEFAULT 20
#define MM_CH_BUFFER_SIZE_DIGITAL_IN 4

/**
 * Class definition for a MicroBitMore Service.
 * Provides a BLE service to remotely read the state of sensors from Scratch3.
 */
class MbitMoreServiceDAL {
public:
  /**
   * Constructor.
   * Create a representation of the MbitMoreService
   * @param _uBit The instance of a MicroBit runtime.
   */
  MbitMoreServiceDAL();

  void notify();

  /**
   * @brief Notify action event.
   *
   * @param data Data to notify.
   * @param length Lenght of the data.
   */
  void notifyActionEvent(uint8_t *data, uint16_t length);

  void notifySharedData();

  /**
   * Set value to Slots.
   */
  void setSharedData(int index, int value);

  /**
   * Get value to Slots.
   */
  int getSharedData(int index);

  /**
   * Callback. Invoked when AnalogIn is read via BLE.
   */
  void onReadAnalogIn(GattReadAuthCallbackParams *authParams);

  /**
   * Callback. Invoked when any of our attributes are written via BLE.
   */
  void onDataWritten(const GattWriteCallbackParams *params);

  /**
   * Invocked when the bluetooth connected.
   */
  void onBLEConnected(MicroBitEvent e);

  /**
   * Invocked when the bluetooth disconnected.
   */
  void onBLEDisconnected(MicroBitEvent e);

  void update();

  // Buffer of characteristic for receiving commands.
  uint8_t commandChBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending data of sensors.
  uint8_t sensorsChBuffer[7] = {0};

  // Buffer of characteristic for sending data about direction.
  uint8_t directionChBuffer[18] = {0};

  // Buffer of characteristic for sending pin events.
  uint8_t pinEventChBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending action events.
  uint8_t actionEventChBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending analog input values of P0.
  uint8_t analogInP0ChBuffer[2] = {0};

  // Buffer of characteristic for sending analog input values of P1.
  uint8_t analogInP1ChBuffer[2] = {0};

  // Buffer of characteristic for sending analog input values of P2.
  uint8_t analogInP2ChBuffer[2] = {0};

  // Buffer of characteristic for sending shared data.
  uint8_t sharedDataChBuffer[8] = {0};

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

  GattCharacteristic *commandCh;
  GattCharacteristic *sensorsCh;
  GattCharacteristic *directionCh;
  GattCharacteristic *pinEventCh;
  GattCharacteristic *actionEventCh;
  GattCharacteristic *analogInP0Ch;
  GattCharacteristic *analogInP1Ch;
  GattCharacteristic *analogInP2Ch;
  GattCharacteristic *sharedDataCh;
};

#endif // MBIT_MORE_SERVICE_DAL_H
#endif // !MICROBIT_CODAL
