import fs from 'fs';
import path from 'path';
import ts from 'typescript';

// Evaluate MbitMore.ts in global context for MakeCode namespace simulation
const mbitMoreCode = fs.readFileSync(path.resolve(__dirname, '../MbitMore.ts'), 'utf8');

const transpileResult = ts.transpileModule(mbitMoreCode, {
  compilerOptions: {
    target: ts.ScriptTarget.ES2020,
    module: ts.ModuleKind.CommonJS
  }
});

// Append global export for global scope in Node
const evalCode = transpileResult.outputText + '\n;global.MbitMore = MbitMore;';
eval(evalCode);

describe('MbitMore TypeScript Extension', () => {
  beforeEach(() => {
    jest.clearAllMocks();
    (global as any).control.events = {};
  });

  test('startService prints log', () => {
    const consoleSpy = jest.spyOn(console, 'log');
    (global as any).MbitMore.startService();
    expect(consoleSpy).toHaveBeenCalledWith("Microbit-More started");
    consoleSpy.mockRestore();
  });

  test('registerWaitingDataLabel returns label ID', () => {
    const id = (global as any).MbitMore.registerWaitingDataLabel('label-01', MbitMoreDataContentType.MM_DATA_NUMBER);
    expect(id).toBe(1);
  });

  test('sendNumberWithLabel logs send payload', () => {
    const consoleSpy = jest.spyOn(console, 'log');
    (global as any).MbitMore.sendNumberWithLabel('test-label', 42);
    expect(consoleSpy).toHaveBeenCalledWith("Microbit-More send a number: test-label = 42");
    consoleSpy.mockRestore();
  });

  test('sendTextWithLabel logs send payload', () => {
    const consoleSpy = jest.spyOn(console, 'log');
    (global as any).MbitMore.sendTextWithLabel('test-label', 'hello');
    expect(consoleSpy).toHaveBeenCalledWith("Microbit-More send a text: test-label = hello");
    consoleSpy.mockRestore();
  });

  test('onReceivedNumberWithLabel registers event handler', () => {
    const handler = jest.fn();
    (global as any).MbitMore.onReceivedNumberWithLabel('label-01', handler);

    // Event key 8000 (MBIT_MORE_DATA_RECEIVED) : 1 (labelID)
    const eventKey = '8000:1';
    expect((global as any).control.events[eventKey]).toBeDefined();

    // Trigger event
    (global as any).control.raiseEvent(8000, 1);
    expect(handler).toHaveBeenCalledWith(0.0); // 0.0 is dummy return value
  });

  test('onReceivedTextWithLabel registers event handler', () => {
    const handler = jest.fn();
    (global as any).MbitMore.onReceivedTextWithLabel('label-02', handler);

    // Event key 8000 (MBIT_MORE_DATA_RECEIVED) : 1 (labelID)
    const eventKey = '8000:1';
    expect((global as any).control.events[eventKey]).toBeDefined();

    // Trigger event
    (global as any).control.raiseEvent(8000, 1);
    expect(handler).toHaveBeenCalledWith("text"); // "text" is dummy return value
  });
});
