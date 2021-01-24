#include "pxt.h"

#if !MICROBIT_CODAL

#ifndef MBIT_MORE_SERVICE_DAL_H
#define MBIT_MORE_SERVICE_DAL_H

#include "MicroBit.h"

#include "MbitMoreCommon.h"
#include "MbitMoreDevice.h"

// // Forward declaration
class MbitMoreDevice;


/**
 * Class definition for a MicroBitMore Service.
 * Provides a BLE service to remotely read the state of sensors from Scratch3.
 */
class MbitMoreServiceDAL {
public:
  /**
   * Constructor.
   * Create a representation of the MbitMoreService
   */
  MbitMoreServiceDAL();

  void notify();

  /**
   * @brief Notify action event.
   */
  void notifyActionEvent();

  /**
   * @brief Notify action event.
   */
  void notifyPinEvent();

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
  uint8_t commandChBuffer[MM_CH_BUFFER_SIZE_MAX] = {0};

  // Buffer of characteristic for sending data of sensors.
  uint8_t sensorsChBuffer[MM_CH_BUFFER_SIZE_SENSORS] = {0};

  // Buffer of characteristic for sending data about direction.
  uint8_t directionChBuffer[MM_CH_BUFFER_SIZE_DIRECTION] = {0};

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

  // Buffer of characteristic for sending shared data.
  uint8_t sharedDataChBuffer[MM_CH_BUFFER_SIZE_NOTIFY] = {0};

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
