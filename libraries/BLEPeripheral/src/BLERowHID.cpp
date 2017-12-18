// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "BLERowHID.h"

static const PROGMEM unsigned char descriptorValue[] = {
  // From: https://github.com/adafruit/Adafruit-Trinket-USB/blob/master/TrinketHidCombo/TrinketHidComboC.c
  //       permission to use under MIT license by @ladyada (https://github.com/adafruit/Adafruit-Trinket-USB/issues/10)

  // this second multimedia key report is what handles the multimedia keys
  // 0x05, 0x0C,           // USAGE_PAGE (Consumer Devices)
  // 0x09, 0x01,           // USAGE (Consumer Control)
  // 0xA1, 0x01,           // COLLECTION (Application)
  // 0x85, 0x00,           //   REPORT_ID
  // 0x19, 0x00,           //   USAGE_MINIMUM (Unassigned)
  // 0x2A, 0x3C, 0x02,     //   USAGE_MAXIMUM
  // 0x15, 0x00,           //   LOGICAL_MINIMUM (0)
  // 0x26, 0x3C, 0x02,     //   LOGICAL_MAXIMUM
  // 0x95, 0x01,           //   REPORT_COUNT (1)
  // 0x75, 0x10,           //   REPORT_SIZE (16)
  // 0x81, 0x00,           //   INPUT (Data,Ary,Abs)
  // 0xC0                  // END_COLLECTION

  // RAW 16-BYTE I/O

    0x06, 0xAB, 0xFF,   // Usage Page (Vendor-Defined 172)
    0x0A, 0x00, 0x02,   // Usage (Vendor-Defined 512)
    0xA1, 0x01,         // Collection (Application)
    0x85, 0x04,         /* REPORT_ID (4) */
    0x75, 0x08,         // Report Size (8)
    0x15, 0x00,         // Logical Minimum (0)
    0x26, 0xFF, 0x00,   // Logical Maximum (255)
    0x95, 0x10,         // Report Count (16)
    0x09, 0x01,         // Usage (Vendor-Defined 1)
    0x81, 0x02,         // Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
    0x95, 0x10,         // Report Count (16)
    0x09, 0x02,         // Usage (Vendor-Defined 2)
    0x91, 0x02,         // Output (Data,Var,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)

    /* End */
    0xc0                            /* END_COLLECTION */
};

BLERowHID::BLERowHID() :
  BLEHID(descriptorValue, sizeof(descriptorValue), 7),
  _reportCharacteristic("2a4d", BLEWriteWithoutResponse, 32),
  _reportReferenceDescriptor(BLEHIDDescriptorTypeOutput)
{
}

size_t BLERowHID::write(uint8_t k) {
  uint8_t multimediaKeyPress[2]= { 0x00, 0x00 };

  // send key code
  multimediaKeyPress[0] = k;

  for (int i = 0; i < 2; i++) {
    this->sendData(this->_reportCharacteristic, multimediaKeyPress, sizeof(multimediaKeyPress));

    // send cleared code
    multimediaKeyPress[0] = 0x00;
  }

  return 1;
}

void BLERowHID::setReportId(unsigned char reportId) {
  BLEHID::setReportId(reportId);

  this->_reportReferenceDescriptor.setReportId(reportId);
}

unsigned char BLERowHID::numAttributes() {
  return 2;
}

BLELocalAttribute** BLERowHID::attributes() {
  static BLELocalAttribute* attributes[2];

  attributes[0] = &this->_reportCharacteristic;
  attributes[1] = &this->_reportReferenceDescriptor;

  return attributes;
}
