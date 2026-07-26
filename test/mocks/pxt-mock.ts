// Global mocks for MakeCode/PXT environment in Jest

// MakeCode const enums
(global as any).MbitMoreDataContentType = {
  MM_DATA_NUMBER: 1,
  MM_DATA_TEXT: 2,
};

(global as any).MbitMoreCommand = {
  CMD_CONFIG: 0x00,
  CMD_PIN: 0x01,
  CMD_DISPLAY: 0x02,
  CMD_AUDIO: 0x03,
  CMD_DATA: 0x04,
};

// Mock `control` object
(global as any).control = {
  events: {} as Record<string, Function>,
  onEvent(src: number, value: number, handler: () => void) {
    const key = `${src}:${value}`;
    this.events[key] = handler;
  },
  raiseEvent(src: number, value: number) {
    const key = `${src}:${value}`;
    if (this.events[key]) {
      this.events[key]();
    }
  }
};

// Mock `basic` object
(global as any).basic = {
  showString: jest.fn(),
  showIcon: jest.fn(),
  clearScreen: jest.fn(),
  pause: jest.fn(),
};

// Mock `serial` object
(global as any).serial = {
  writeBuffer: jest.fn(),
  writeString: jest.fn(),
  readLine: jest.fn(),
};
