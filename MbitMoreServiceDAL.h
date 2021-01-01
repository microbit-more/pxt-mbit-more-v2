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
class MbitMoreServiceDAL : public MicroBitComponent {
public:
  /**
   * Constructor.
   * Create a representation of the MbitMoreService
   * @param _uBit The instance of a MicroBit runtime.
   */
  MbitMoreServiceDAL();

  void notify();

  /**
   * @brief Notify button event.
   *
   * @param data Data to notify.
   * @param length Lenght of the data.
   */
  void notifyButtonEvent(uint8_t *data, uint16_t length);

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

  /**
   * Periodic callback from MicroBit idle thread.
   */
  virtual void idleCallback();

  void update();

  // Buffer of characteristic for receiving commands.
  uint8_t commandBuffer[MM_CH_BUFFER_SIZE_DEFAULT] = {0};

  // Buffer of characteristic for sending digital levels.
  uint8_t digitalInBuffer[MM_CH_BUFFER_SIZE_DIGITAL_IN] = {0};

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

  uint32_t digitalValues = 0;

  GattCharacteristic *commandCh;
  GattCharacteristic *digitalInCh;
  GattCharacteristic *buttonEventCh;
  GattCharacteristic *lightLevelCh;
  GattCharacteristic *analogIn0Ch;
};

#endif // MBIT_MORE_SERVICE_DAL_H
#endif // !MICROBIT_CODAL
