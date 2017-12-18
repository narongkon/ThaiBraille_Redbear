// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEHIDPeripheral.h>
#include <BLEKeyboard.h>
#include <HID-Project.h>

// --------------- Min/Max Buffer Variable ---------------
#define UART_MAX_BUFFER     43
#define UART_MIN_BUFFER     6

// --------------- Bluetooth Variable ---------------
#define BLE_REQ   6
#define BLE_RDY   7
#define BLE_RST   4


// --------------- Serial Line Variable ---------------

byte buf_read2[UART_MIN_BUFFER];
unsigned int uart_index_write2 = 0;


byte buf_setting_calls[4] = {0xfe, 0x02, 0x01, 0x28};

byte uart_read_buff[42] ;
unsigned int uart_index_write = 0;
//unsigned int uart_read_index = 0;
//unsigned int uart_read_start = 0;
//unsigned int uart_read_len = 0;
//unsigned int uart_read_id = 0;
//unsigned int uart_read_end = 0;



// --------------- Tx/Rx Variable ---------------
size_t _rxHead = 0;
size_t _rxTail = 0;
size_t _rxCount = 0;
uint8_t _rxBuffer[BLE_ATTRIBUTE_MAX_VALUE_LENGTH];

// --------------- Create peripheral instance, see pinouts above ---------------
BLEHIDPeripheral bleHIDPeripheral = BLEHIDPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEKeyboard bleKeyboard;
BLEService _uartService = BLEService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLEDescriptor _uartNameDescriptor = BLEDescriptor("2901", "UART");
BLECharacteristic _rxCharacteristic = BLECharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWriteWithoutResponse, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);
BLEDescriptor _rxNameDescriptor = BLEDescriptor("2901", "RX - Receive Data (Write)");


void setup() {
  Keyboard.begin();
  Serial.begin(38400); // Open Serial port of Keyboard
  Serial1.begin(38400);// Open Serial port of Bluetooth
  bleHIDPeripheral.clearBondStoreData(); // Clear bond store data

#ifdef ANDROID_CENTRAL
  bleHIDPeripheral.setReportIdOffset(1);
#endif

  bleHIDPeripheral.setLocalName("Thai braille");
  bleHIDPeripheral.addHID(bleKeyboard);
  bleHIDPeripheral.addAttribute(_uartService);
  bleHIDPeripheral.addAttribute(_uartNameDescriptor);
  bleHIDPeripheral.setAdvertisedServiceUuid(_uartService.uuid());
  bleHIDPeripheral.addAttribute(_rxCharacteristic);
  bleHIDPeripheral.addAttribute(_rxNameDescriptor);
  _rxCharacteristic.setEventHandler(BLEWritten, HID_received);
  bleHIDPeripheral.begin();
}

void loop() {

  // for (int i = 0; i < 40; i++) {
  //   buf_read[i] = 0xff;
  // }
  // Serial1.write(buf_read, 40);

  // --------------- Bluetooth Loop ---------------
  BLECentral central = bleHIDPeripheral.central();
  if (central) {
    while (central.connected()) {
      Serial1Event1();
      if (uart_index_write2 > 0) {
        //  Serial.write(buf_read2, uart_index_write2);
        bleKeyboard.press(buf_read2[2]);
        bleKeyboard.press(buf_read2[3]);
        bleKeyboard.press(buf_read2[4]);
        bleKeyboard.releaseAll();
        uart_index_write2 = 0;
      }
      bleHIDPeripheral.poll();
      uart_read_buff[0]=0xff;
      uart_read_buff[1]=0x28;
      while (HID_available() > 0) {
        delay(100);
        uart_read_buff[uart_index_write+2] = HID_read();
        uart_index_write++;
      }
      while (uart_index_write > 0) {
        Serial1.write(uart_read_buff, 40);
        for (int i = 0; i < 42; i++) {
          uart_read_buff[i] = 0x00;
        }
        uart_index_write = 0;
      }
    }
  }
  else {
    // --------------- Serial Line Loop ---------------
    SerialEvent();
    Serial1Event1();
    //    if (uart_index_write > 0) {
    //      if (uart_read_buff[0] == 0xff) {
    //        if (uart_read_buff[1] == 0x01) {
    //          Serial.write(buf_setting_calls, 4);
    //        }
    //        else{
    //        //if (uart_read_buff[1] == 0x03) {
    //
    //          Serial1.write(uart_read_buff, 40);
    //        }
    //      }else{
    //        Serial1.write(uart_read_buff, 40);
    //      }
    //
    //      uart_index_write = 0;
    //
    //    }
    if (uart_index_write2 > 0) {
      Keyboard.press((KeyboardKeycode)buf_read2[2]);
      Keyboard.press((KeyboardKeycode)buf_read2[3]);
      Keyboard.press((KeyboardKeycode)buf_read2[4]);
      Keyboard.releaseAll();
      uart_index_write2 = 0;
    }
  }
}

int HID_read(void) {
  bleHIDPeripheral.poll();
  if (_rxTail == _rxHead) return -1;
  _rxTail = (_rxTail + 1) % sizeof(_rxBuffer);
  uint8_t byte = _rxBuffer[_rxTail];
  return byte;
}

int HID_available(void) {
  bleHIDPeripheral.poll();
  int retval = (_rxHead - _rxTail + sizeof(_rxBuffer)) % sizeof(_rxBuffer);
  //#ifdef BLE_Serial_DEBUG
  //  //Serial.print(F("BLESerial::available() = "));
  //  //Serial.println(retval);
  //#endif
  return retval;
}

void HID_received(const uint8_t* data, size_t size) {
  for (int i = 0; i < size; i++) {
    _rxHead = (_rxHead + 1) % sizeof(_rxBuffer);
    _rxBuffer[_rxHead] = data[i];
  }
  //#ifdef BLE_Serial_DEBUG
  // // Serial.-print(F("BLESerial::received("));
  //  for (int i = 0; i < size; i++) Serial.print((char) data[i]);
  // // Serial.println(F(")"));
  //#endif
}

void HID_received(BLECentral & central, BLECharacteristic & rxCharacteristic) {
  HID_received(rxCharacteristic.value(), rxCharacteristic.valueLength());
}

// --------------- Function Get Data form Serial Line ---------------

void SerialEvent() {

  delay(100);
  while (Serial.available()) {
    byte d = (byte)Serial.read();
    if (d == 0xff) {
      byte id = (byte)Serial.read();
      byte len = (byte)Serial.read();
      byte buffer[42] ;
      buffer[0]=0xff;
      buffer[1]=0x28;
      for (int i = 0; i < 40; i++) {
        buffer[i+2] = 0x00;
      }
      for (int i = 0; i < len; i++) {
        buffer[i+2] = (byte)Serial.read();
      }
      if (id == 0x01) {
        Serial.write(buf_setting_calls, 4);
      }
      else if (id = 0x03) {
        Serial1.write(buffer, 42);
      }
    }
  }


  
  //    uart_read_buff[uart_index_write++] = (byte)Serial.read();
  //     Serial1.write(  uart_read_buff[uart_index_write]);
  //    if (uart_index_write >= UART_MAX_BUFFER) {
  //      uart_index_write = 0;
  //    }
  //  }
  //  while (Serial.available()) {
  //    byte d = (byte)Serial.read();
  //    if (d == 0xff && uart_read_start == 0) {
  //      //Serial1.write("start read data");
  //      uart_read_start = 1;
  //    } else {
  //      if ( uart_read_start == 1 && uart_read_id == 0) {
  //        uart_read_id = d;
  //        // Serial1.write("get id");
  //      } else {
  //        if (uart_read_len == 0) {
  //          uart_read_len = d;
  //          //Serial1.write("get lend");
  //        } else {
  //          uart_read_buff[uart_read_index++] = d;
  //          if (uart_read_index == uart_read_len) {
  //            uart_read_end = 1;
  //            // Serial1.write("end data");
  //          }
  //        }
  //      }
  //
  //    }
  //  }
}
// --------------- End Function Get Data form Serial Line ---------------

// --------------- Function Get Data form Bluetooth ---------------
void Serial1Event1() {
  delay(100);
  while (Serial1.available()) {
    buf_read2[uart_index_write2++] = (byte)Serial1.read();
    if (uart_index_write2 >= UART_MIN_BUFFER) {
      uart_index_write2 = 0;
    }
  }
}
// --------------- End Function Get Data form Bluetooth ---------------


