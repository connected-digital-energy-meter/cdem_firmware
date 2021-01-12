#include "digital_meter.h"
#include "crc_checker.h"

namespace SmartMeter {

  #define Debug(...) if(this->debugSerial) this->debugSerial->print(__VA_ARGS__); 
  #define DebugLn(...) if(this->debugSerial) this->debugSerial->println(__VA_ARGS__); 

  DigitalMeter::DigitalMeter(int requestPin, DeviceStatus * deviceStatus, HardwareSerial * serial, HardwareSerial * debugSerial) {\
    this->deviceStatus = deviceStatus;
    this->requestPin = requestPin;
    this->serial = serial;
    this->debugSerial = debugSerial;
    pinMode(requestPin, OUTPUT);
    deviceStatus->meter_awaiting();
  }

  void DigitalMeter::enable(void) {
    DebugLn("Enabling the digital meter data request");
    digitalWrite(requestPin, HIGH);
    readPointer = 0;
    startDetected = false;
  }

  void DigitalMeter::disable(void) {
    DebugLn("Disabling the digital meter data request");
    digitalWrite(requestPin, LOW);    
  }

  void DigitalMeter::timeout(void) {
    digitalWrite(requestPin, LOW);
    deviceStatus->meter_error();
    SerialDebug.println("Communication with the smartmeter timed out!");        
  }

  // Read a new datagram from the P1 port
  bool DigitalMeter::read_datagram(char * buffer, size_t bufferLength) {
    if (serial->available() > 0) {
            
      // Get next byte for the P1 port
      char incomingByte = serial->read();
            
      // Look for the start of the datagram
      if (!startDetected && incomingByte == '/') {
        readPointer = 0;
        startDetected = true;
        DebugLn("Detected start of a datagram");
      }

      // Ignore all data on serial port if start was not detected
      if (startDetected) {
        buffer[readPointer++] = incomingByte;
        Debug(incomingByte);

        // Look for the end of the datagram
        if (incomingByte == '\n' && buffer[readPointer-7] == '!') {
          DebugLn("Read in full datagram");
          startDetected = false;

          DebugLn("Checking datagram CRC");
          if (CrcChecker::check_crc(buffer, readPointer)) {
            DebugLn("Datagram is valid");
            deviceStatus->meter_data_ready();
            return true;
          } else {
            DebugLn("Datagram is invalid - CRC Check Failed");
            return false;
          }

        } else if (readPointer >= bufferLength) {    // End of datagram not found
          DebugLn("Invalid Datagram > No end detected");
          clear_buffer(buffer, bufferLength);
          readPointer = 0;
          startDetected = false;
        }
      }
    }

    return false;
  }

  void DigitalMeter::clear_buffer(char * buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
      buffer[i] = 0;
    }
  }

};