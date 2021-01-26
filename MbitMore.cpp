#include "pxt.h"

#define UPDATE_PERIOD 11
#define NOTIFY_PERIOD 101

enum SharedDataIndex
{
  //% block="Data0"
  Data0 = 0,
  //% block="Data1"
  Data1 = 1,
  //% block="Data2"
  Data2 = 2,
  //% block="Data3"
  Data3 = 3,
  //% block="Data4"
  Data4 = 4,
  //% block="Data5"
  Data5 = 5,
  //% block="Data6"
  Data6 = 6,
  //% block="Data7"
  Data7 = 7,
};

#if MICROBIT_CODAL
#include "MbitMoreService.h"
#else // MICROBIT_CODAL
#include "MbitMoreServiceDAL.h"
using MbitMoreService = MbitMoreServiceDAL;
#endif // NOT MICROBIT_CODAL

//% color=#FF9900 weight=95 icon="\uf1b0"
namespace MbitMore {
  MbitMoreService *_pService = NULL;

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
   * Set shared data value.
   */
  //%
  void setMbitMoreSharedData(SharedDataIndex index, int value) {
    if (NULL == _pService)
      return;

    _pService->setSharedData((int)index, value);
  }

  /**
   * Get shared data value.
   */
  //%
  int getMbitMoreSharedData(SharedDataIndex index) {
    if (NULL == _pService)
      return 0;

    return _pService->getSharedData((int)index);
  }
} // namespace MbitMore
