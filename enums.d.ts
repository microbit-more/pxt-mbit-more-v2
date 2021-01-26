// Auto-generated. Do not edit.


    declare const enum MbitMoreCommand
    {
    CMD_CONFIG = 0x00,
    CMD_PIN = 0x01,
    CMD_DISPLAY = 0x02,
    CMD_SHARED_DATA = 0x03,
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
    PIXELS_0 = 0x02,
    PIXELS_1 = 0x03,
    }


    /**
     * @brief Enum for write mode of display pixels.
     */

    declare const enum MbitMoreDisplayWriteMode
    {
    LAYER = 0,
    OVER_WRITE = 1,
    }


    declare const enum MbitMorePullMode
    {
    None = 0,
    Down = 1,
    Up = 2,
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


    declare const enum MbitMorePinEvent
    {
    RISE = 2,
    FALL = 3,
    PULSE_HIGH = 4,
    PULSE_LOW = 5,
    }


    declare const enum SharedDataIndex
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
    }
declare namespace MbitMore {
}

// Auto-generated. Do not edit. Really.
