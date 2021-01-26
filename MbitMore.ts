namespace MbitMore {
  // dummy for sim
  let sharedData: number[] = [0, 0, 0, 0, 0, 0, 0, 0];

  /**
  * Starts BLE services for Scratch Microbit-More extension.
  */
  //% blockId=MbitMore_startMbitMoreService block="start Microbit-More service"
  //% shim=MbitMore::startMbitMoreService
  export function startService(): void {
    console.log("Microbit-More started");
  }

  /**
   * Set value of the shared data.
   * @param index - Index of shared data.
   * @param value - New value of shared data.
   */
  //% blockId=MbitMore_setMbitMoreSharedData block="set shared %index to %value"
  //% shim=MbitMore::setMbitMoreSharedData
  export function setSharedData(index: SharedDataIndex, value: number): void {
    sharedData[index] = value;
  }

  /**
   * Get value of the shared data.
   */
  //% blockId=MbitMore_getMbitMoreSharedData block="shared %index"
  //% shim=MbitMore::getMbitMoreSharedData
  export function getSharedData(index: SharedDataIndex): number {
    return sharedData[index];
  }
} // namespace MbitMore