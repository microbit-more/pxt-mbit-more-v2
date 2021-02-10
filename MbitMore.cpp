#include "pxt.h"

#include "MicroBit.h"
#include "MicroBitConfig.h"

#include "MbitMoreCommon.h"

#define UPDATE_PERIOD 11
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
  int dummyMessageID = 0;
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
   * Starts a Scratch extension service.
   */
  //%
  void startMbitMoreService() {
    if (NULL != _pService)
      return;

    _pService = new MbitMoreService();

    create_fiber(update);
    create_fiber(notifyScratch);
  }

  /**
   * Events can have arguments before the handler
   */
  //%
  int call_registerWaitingMessage(String messageLabel, MbitMoreMessageType messageType) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      startMbitMoreService();

    int messageID = _pService->registerWaitingMessage(MSTR(messageLabel), messageType);
    return messageID;
#else // NOT MICROBIT_CODAL
    return ++dummyMessageID; // dummy
#endif // NOT MICROBIT_CODAL
  }

  //%
  float call_messageContentAsNumber(int messageID) {
#if MICROBIT_CODAL
    return _pService->messageContentAsNumber(messageID);
#else // NOT MICROBIT_CODAL
    return 0.0; // dummy
#endif // NOT MICROBIT_CODAL
  }

  //%
  String call_messageContentAsText(int messageID) {
#if MICROBIT_CODAL
    return PSTR(_pService->messageContentAsText(messageID));
#else // NOT MICROBIT_CODAL
    return String(""); // dummy
#endif // NOT MICROBIT_CODAL
  }

  /**
   * Send a labeled message with content in float.
   * @param label - label of the message
   * @param content - number for content
   */
  //%
  void call_sendMessageWithNumber(String messageLabel, float messageContent) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      return;

    _pService->sendMessageWithNumber(MSTR(messageLabel), messageContent);
#endif // MICROBIT_CODAL
  }

  /**
   * Send a labeled message with content in string.
   * @param label - label of the message
   * @param content - string for content
   */
  //%
  void call_sendMessageWithText(String messageLabel, String messageContent) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      return;

    _pService->sendMessageWithText(MSTR(messageLabel), MSTR(messageContent));
#endif // MICROBIT_CODAL
  }

} // namespace MbitMore
