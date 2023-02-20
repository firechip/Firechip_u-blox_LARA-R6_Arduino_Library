/*

  LARA-R6 Example
  ===============

  Identification
  
  Written by: Paul Clark
  Date: November 18th 2020

  This example demonstrates how to read the LARA's:
    Manufacturer identification
    Model identification
    Firmware version identification
    Product Serial No.
    IMEI identification
    IMSI identification
    SIM CCID
    Subscriber number
    Capabilities
    SIM state

  Feel like supporting open source hardware?
  Buy a board from firechip!

  Licence: MIT
  Please see LICENSE.md for full details

*/

#include <Firechip_u-blox_LARA-R6_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#Firechip_u-blox_LARA-R6_Arduino_Library

// Uncomment the next line to connect to the LARA-R6 using hardware Serial1
#define laraSerial Serial1

// Uncomment the next line to create a SoftwareSerial object to pass to the LARA-R6 library instead
//SoftwareSerial laraSerial(8, 9);

// Create a LARA_R6 object to use throughout the sketch
// Usually we would tell the library which GPIO pin to use to control the LARA power (see below),
// but we can start the LARA without a power pin. It just means we need to manually 
// turn the power on if required! ;-D
LARA_R6 myLARA;

// Create a LARA_R6 object to use throughout the sketch
// We need to tell the library what GPIO pin is connected to the LARA power pin.
// If you're using the MicroMod Asset Tracker and the MicroMod Artemis Processor Board,
// the pin name is G2 which is connected to pin AD34.
// Change the pin number if required.
//LARA_R6 myLARA(34);

// Map SIM states to more readable strings
String simStateString[] =
{
  "Not present",      // 0
  "PIN needed",       // 1
  "PIN blocked",      // 2
  "PUK blocked",      // 3
  "Not operational",  // 4
  "Restricted",       // 5
  "Operational"       // 6
};

// processSIMstate is provided to the LARA-R6 library via a 
// callback setter -- setSIMstateReadCallback. (See setup())
void processSIMstate(LARA_R6_sim_states_t state)
{
  Serial.println();
  Serial.print(F("SIM state:           "));
  Serial.print(String(state));
  Serial.println();
}

void setup()
{
  Serial.begin(115200); // Start the serial console

  // Wait for user to press key to begin
  Serial.println(F("LARA-R6 Example"));
  Serial.println(F("Press any key to begin"));
  
  while (!Serial.available()) // Wait for the user to press a key (send any serial character)
    ;
  while (Serial.available()) // Empty the serial RX buffer
    Serial.read();

  //myLARA.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  // For the MicroMod Asset Tracker, we need to invert the power pin so it pulls high instead of low
  // Comment the next line if required
  myLARA.invertPowerPin(true); 

  // Initialize the LARA
  if (myLARA.begin(laraSerial, 9600) )
  {
    Serial.println(F("LARA-R6 connected!"));
  }
  else
  {
    Serial.println(F("Unable to communicate with the LARA."));
    Serial.println(F("Manually power-on (hold the LARA On button for 3 seconds) on and try again."));
    while (1) ; // Loop forever on fail
  }
  Serial.println();

  Serial.println("Manufacturer ID:     " + String(myLARA.getManufacturerID()));
  Serial.println("Model ID:            " + String(myLARA.getModelID()));
  Serial.println("Firmware Version:    " + String(myLARA.getFirmwareVersion()));
  Serial.println("Product Serial No.:  " + String(myLARA.getSerialNo()));
  Serial.println("IMEI:                " + String(myLARA.getIMEI()));
  Serial.println("IMSI:                " + String(myLARA.getIMSI()));
  Serial.println("SIM CCID:            " + String(myLARA.getCCID()));
  Serial.println("Subscriber No.:      " + String(myLARA.getSubscriberNo()));
  Serial.println("Capabilities:        " + String(myLARA.getCapabilities()));

  // Set a callback to return the SIM state once requested
  myLARA.setSIMstateReportCallback(&processSIMstate);
  // Now enable SIM state reporting for states 0 to 6 (by setting the reporting mode LSb)
  if (myLARA.setSIMstateReportingMode(1) == LARA_R6_SUCCESS)
    Serial.println("SIM state reports requested...");
  // You can disable the SIM staus reports again by calling assetTracker.setSIMstateReportingMode(0)
}

void loop()
{
  myLARA.poll(); // Keep processing data from the LARA so we can extract the SIM status
}
