#include "pxt.h"

#include "MicroBit.h"
#include "MicroBitConfig.h"

#if !MICROBIT_CODAL

#ifndef MBIT_MORE_SERVICE_DAL_H
#define MBIT_MORE_SERVICE_DAL_H

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

  /**
   * Invoked when BLE connected.
   */
  void onBLEConnected(MicroBitEvent _e);

  void notify();

  /**
   * @brief Notify action event.
   */
  void notifyActionEvent();

  /**
   * @brief Notify action event.
   */
  void notifyPinEvent();

  /**
   * Callback. Invoked when AnalogIn is read via BLE.
   */
  void onReadAnalogIn(GattReadAuthCallbackParams *authParams);

  /**
   * Callback. Invoked when any of our attributes are written via BLE.
   */
  void onDataWritten(const GattWriteCallbackParams *params);

  void update();

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
  GattCharacteristic *stateCh;
  GattCharacteristic *directionCh;
  GattCharacteristic *pinEventCh;
  GattCharacteristic *actionEventCh;
  GattCharacteristic *analogInP0Ch;
  GattCharacteristic *analogInP1Ch;
  GattCharacteristic *analogInP2Ch;
};

#endif // MBIT_MORE_SERVICE_DAL_H
#endif // !MICROBIT_CODAL
