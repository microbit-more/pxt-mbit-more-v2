// Auto-generated. Do not edit.


    declare const enum MbitMoreCommand
    {
    CMD_CONFIG = 0x10,
    CMD_PIN = 0x11,
    CMD_DISPLAY = 0x12,
    CMD_SHARED_DATA = 0x13,
    }


    declare const enum MbitMorePinCommand
    {
    SET_OUTPUT = 0x01,
    SET_PWM = 0x02,
    SET_SERVO = 0x03,
    SET_PULL = 0x04,
    SET_EVENT = 0x05,
    SET_TOUCH = 0x06,
    }


    declare const enum MbitMoreDisplayCommand
    {
    CLEAR = 0x00,
    TEXT = 0x01,
    PIXELS = 0x02,
    }


    /**
     * @brief Enum for write mode of display pixels.
     */

    declare const enum MbitMoreDisplayWriteMode
    {
    LAYER = 0,
    OVER_WRITE = 1,
    }


    declare const enum MbitMorePinMode
    {
    MbitMorePullNone = 0,
    MbitMorePullUp = 1,
    MbitMorePullDown = 2,
    }


    declare const enum MbitMoreDataFormat
    {
    PIN_EVENT = 0x10,
    ACTION_EVENT = 0x11,
    SHARED_DATA = 0x13,
    }


    declare const enum MbitMoreActionEvent
    {
    BUTTON = 0x01,
    GESTURE = 0x02,
    }


    declare const enum MbitMoreButtonEvent
    {
    DOWN = 1,
    UP = 2,
    CLICK = 3,
    LONG_CLICK = 4,
    HOLD = 5,
    DOUBLE_CLICK = 6,
    }


    declare const enum MbitMoreGestureEvent
    {
    TILT_UP = 1,
    TILT_DOWN = 2,
    TILT_LEFT = 3,
    TILT_RIGHT = 4,
    FACE_UP = 5,
    FACE_DOWN = 6,
    FREEFALL = 7,
    G3 = 8,
    G6 = 9,
    G8 = 10,
    SHAKE = 11,
    }


    declare const enum MbitMorePinEventType
    {
    NONE = 0,
    ON_EDGE = 1,
    ON_PULSE = 2,
    ON_TOUCH = 3,
    }


    declare const enum SharedDataIndex
    {
    //% block="data0"
    DATA0 = 0,
    //% block="data1"
    DATA1 = 1,
    //% block="data2"
    DATA2 = 2,
    //% block="data3"
    DATA3 = 3,
    }
declare namespace MbitMore {
}

// Auto-generated. Do not edit. Really.
