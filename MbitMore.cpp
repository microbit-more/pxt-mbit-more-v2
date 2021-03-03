#include "pxt.h"

#include "MicroBit.h"
#include "MicroBitConfig.h"

#include "MbitMoreCommon.h"

#define UPDATE_PERIOD 19
#define NOTIFY_PERIOD 101

#if MICROBIT_CODAL
#include "MbitMoreService.h"
#else // NOT MICROBIT_CODAL
#include "MbitMoreServiceDAL.h"
using MbitMoreService = MbitMoreServiceDAL;
#endif // NOT MICROBIT_CODAL

//% color=#FF9900 weight=95 icon="\uf1b0"
namespace MbitMore {
  MbitMoreService *_pService = NULL;
#if MICROBIT_CODAL
#else // NOT MICROBIT_CODAL
  int dummyDataLabelID = 0;
#endif // NOT MICROBIT_CODAL

  void update() {
    while (NULL != _pService) {
      _pService->update();
      fiber_sleep(UPDATE_PERIOD);
    }
  }

  void notifyScratch() {
    while (NULL != _pService) {
      // notyfy data to Scratch
      _pService->notify();
      fiber_sleep(NOTIFY_PERIOD);
    }
  }

  /**
   * @brief Start Microbit More service.
   * 
   */
  //%
  void startMbitMoreService() {
    if (NULL != _pService)
      return;

    _pService = new MbitMoreService();

    create_fiber(update);
    // create_fiber(notifyScratch);
  }

  /**
   * @brief Register a label in waiting data list and return an ID for the label.
   * This starts Microbit More service if it was not available.
   * 
   * @param dataLabel label to register
   * @param dataType type of the data to be received
   * @return int ID for the label
   */
  //%
  int call_registerWaitingDataLabel(String dataLabel, MbitMoreDataContentType dataType) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      startMbitMoreService();

    int labelID = _pService->registerWaitingDataLabel(MSTR(dataLabel), dataType);
    return labelID;
#else // NOT MICROBIT_CODAL
    return ++dummyDataLabelID; // dummy
#endif // NOT MICROBIT_CODAL
  }

  /**
   * @brief Get number which was received with the label.
   * 
   * @param labelID ID in registered labels
   * @return float received data with the label
   */
  //%
  float call_dataContentAsNumber(int labelID) {
#if MICROBIT_CODAL
    return _pService->dataContentAsNumber(labelID);
#else // NOT MICROBIT_CODAL
    return 0.0; // dummy
#endif // NOT MICROBIT_CODAL
  }

  /**
   * @brief Get text which was received with the label.
   * 
   * @param labelID ID in registered labels
   * @return String received data with the label
   */
  //%
  String call_dataContentAsText(int labelID) {
#if MICROBIT_CODAL
    return PSTR(_pService->dataContentAsText(labelID));
#else // NOT MICROBIT_CODAL
    return String(""); // dummy
#endif // NOT MICROBIT_CODAL
  }

  /**
   * @brief Send a float with labele to Scratch.
   * Do nothing if Scratch was not connected.
   * 
   * @param dataLabel - label of the data
   * @param dataContent - content of the data
   */
  //%
  void call_sendNumberWithLabel(String dataLabel, float dataContent) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      return;

    _pService->sendNumberWithLabel(MSTR(dataLabel), dataContent);
#endif // MICROBIT_CODAL
  }

  /**
   * @brief Send a text with label to Scratch.
   * Do nothing if Scratch was not connected.
   * 
   * @param dataLabel - label of the data
   * @param dataContent - content of the data
   */
  //%
  void call_sendTextWithLabel(String dataLabel, String dataContent) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      return;

    _pService->sendTextWithLabel(MSTR(dataLabel), MSTR(dataContent));
#endif // MICROBIT_CODAL
  }

} // namespace MbitMore
