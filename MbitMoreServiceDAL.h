#include "pxt.h"

#if !MICROBIT_CODAL

#ifndef MBIT_MORE_SERVICE_DAL_H
#define MBIT_MORE_SERVICE_DAL_H

#include "MicroBit.h"

#include "MbitMoreCommon.h"

#define SCRATCH_MORE_ID 2000

#define SCRATCH_MORE_EVT_NOTIFY 1

/**
 * Position of data format in a value holder.
 */
#define DATA_FORMAT_INDEX 19

// UUIDs for our service and characteristics
extern const uint16_t MBIT_MORE_BASIC_SERVICE;
extern const uint8_t MBIT_MORE_BASIC_TX[];
extern const uint8_t MBIT_MORE_BASIC_RX[];
extern const uint8_t MBIT_MORE_SERVICE[];
extern const uint8_t MBIT_MORE_EVENT[];
extern const uint8_t MBIT_MORE_IO[];
extern const uint8_t MBIT_MORE_SENSORS[];
extern const uint8_t MBIT_MORE_SHARED_DATA[];
extern const uint8_t MBIT_MORE_ANALOG_IN[];

/**
  * Class definition for a MicroBitMore Service.
  * Provides a BLE service to remotely read the state of sensors from Scratch3.
  */
class MbitMoreServiceDAL
{
public:
  /**
    * Constructor.
    * Create a representation of the MbitMoreService
    * @param _uBit The instance of a MicroBit runtime.
    */
  MbitMoreServiceDAL();

  void initConfiguration();

  /**
    * Notify data to Scratch3.
    */
  void notify();
  void notifyDefaultData();
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
   * Callback. Invoked when a pin event sent.
   */
  void onPinEvent(MicroBitEvent evt);

  void update();

  void updateDigitalValues();
  void updatePowerVoltage();
  void updateAnalogValues();
  void updateLightSensor();
  void updateAccelerometer();
  void updateMagnetometer();

  void writeIo();
  void writeAnalogIn();
  void writeSensors();
  void writeSharedData();

private:

  // Sending data to Scratch3.
  uint8_t txData[20];

  // Recieving buffer from Scratch3.
  uint8_t rxBuffer[10];

  // Config buffer set by Scratch.
  uint8_t eventBuffer[20];

  // Sending data of IO to Scratch.
  uint8_t ioBuffer[4];

  // Sending data of analog input to Scratch.
  uint8_t analogInBuffer[20];

  // Sending data of all sensors to Scratch.
  uint8_t sensorsBuffer[20];

  // Shared data with Scratch.
  uint8_t sharedBuffer[20];

  /**
   * Button state.
   */
  int buttonAState;
  int buttonBState;

  /**
   * Hold gesture state until next nofification.
   */
  int gesture;

  /**
   * Save the last accelerometer values to conpaire current for detecting moving.
   */
  int lastAcc[3];

  /**
   * Heading angle of compass.
   */
  int compassHeading;

  uint32_t digitalValues;

  uint16_t analogValues[6];

  /**
   * Light level value from 0 to 255.
   */
  int lightLevel;

  int lightSensingDuration;

  /**
   * Acceleration value [x, y, z] in milli-g.
   */
  int acceleration[6];

  /**
   * Rotation value [pitch, roll] in radians.
   */
  float rotation[2];

  /**
   * Magnetic force [x, y, z] in 1000 * micro-teslas.
   */
  int magneticForce[3];

  /**
   * Shared data
   */
  int16_t sharedData[4];

  /**
   * Protocol of microbit more.
   */
  int mbitMoreProtocol;

  /**
   * Current mode of all pins.
   */
  PinMode pullMode[21];
  
  /**
   * Voltage of the power supply in [mV]
   */
  int powerVoltage;


  void listenPinEventOn(int pinIndex, int eventType);
  void setPullMode(int pinIndex, PinMode pull);
  void setDigitalValue(int pinIndex, int value);
  void setAnalogValue(int pinIndex, int value);
  void setServoValue(int pinIndex, int angle, int range, int center);
  void setPinModeTouch(int pinIndex);
  void setLightSensingDuration(int duration);

  void onButtonChanged(MicroBitEvent);
  void onGestureChanged(MicroBitEvent);

  void updateGesture(void);
  void resetGesture(void);

  int normalizeCompassHeading(int heading);
  int convertToTilt(float radians);

  void composeDefaultData(uint8_t *buff);
  void composeTxBuffer01(void);
  void composeTxBuffer02(void);
  void composeTxBuffer03(void);

  void displayFriendlyName();

  // microbit runtime instance
  MicroBit &uBit;

  // Handles to access each characteristic when they are held by Soft Device.
  GattAttribute::Handle_t txCharacteristicHandle;
  GattAttribute::Handle_t rxCharacteristicHandle;

  GattCharacteristic *eventChar;
  GattCharacteristic *ioChar;
  GattCharacteristic *analogInChar;
  GattCharacteristic *sensorsChar;
  GattCharacteristic *sharedDataChar;

  GattAttribute::Handle_t eventCharHandle;
  GattAttribute::Handle_t ioCharHandle;
  GattAttribute::Handle_t sensorsCharHandle;
  GattAttribute::Handle_t sharedDataCharHandle;

};

#endif  // MBIT_MORE_SERVICE_DAL_H
#endif // !MICROBIT_CODAL
