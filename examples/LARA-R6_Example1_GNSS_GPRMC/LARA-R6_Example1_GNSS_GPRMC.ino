/*

  LARA-R6 Example
  ===============

  GNSS GPRMC
  
  Written by: Paul Clark
  Date: November 18th 2020

  This example enables the LARA-R6nnM8S' built-in GNSS receiver and reads the GPRMC message to
  get position, speed and time data.

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

PositionData gps;
SpeedData spd;
ClockData clk;
boolean valid;

#define GPS_POLL_RATE 5000 // Read GPS every 5 seconds
unsigned long lastGpsPoll = 0;

void setup()
{
  Serial.begin(115200); // Start the serial console

  // Wait for user to press key to begin
  Serial.println(F("LARA-R6 Example"));
  Serial.println(F("Press any key to begin GNSS'ing"));
  
  while (!Serial.available()) // Wait for the user to press a key (send any serial character)
    ;
  while (Serial.available()) // Empty the serial RX buffer
    Serial.read();

  //myLARA.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  // For the MicroMod Asset Tracker, we need to invert the power pin so it pulls high instead of low
  // Comment the next line if required
  myLARA.invertPowerPin(true); 

  // Initialize the LARA
  if (myLARA.begin(laraSerial, 9600) ) {
    Serial.println(F("LARA-R6 connected!"));
  }

  // Enable power for the GNSS active antenna
  // On the MicroMod Asset Tracker, the LARA GPIO2 pin is used to control power for the antenna.
  // We need to pull GPIO2 (Pin 23) high to enable the power.
  myLARA.setGpioMode(myLARA.GPIO2, myLARA.GPIO_OUTPUT, 1);

  // From the u-blox LARA-R6 Positioning Implementation Application Note UBX-20012413 - R01
  // To enable the PPS output we need to:
  // Configure GPIO6 for TIME_PULSE_OUTPUT - .init does this
  // Enable the timing information request with +UTIMEIND=1 - setUtimeIndication()
  // Reset the time offset configuration with +UTIMECFG=0,0 - setUtimeConfiguration()
  // Request PPS output using GNSS+LTE (Best effort) with +UTIME=1,1 - setUtimeMode()
  // The bits that don't seem to be mentioned in the documentation are:
  //   +UTIME=1,1 also enables the GNSS module and so we don't need to call gpsPower.
  //   +UTIME=1,1 only works when the GNSS module if off. It returns an ERROR if the GNSS is already on.

  // Enable the timing information request (URC)
  //myLARA.setUtimeIndication(); // Use default (LARA_R6_UTIME_URC_CONFIGURATION_ENABLED)
  
  // Clear the time offset
  myLARA.setUtimeConfiguration(); // Use default offset (offsetNanoseconds = 0, offsetSeconds = 0)
  
  // Set the UTIME mode to pulse-per-second output using a best effort from GNSS and LTE
  myLARA.setUtimeMode(); // Use defaults (mode = LARA_R6_UTIME_MODE_PPS, sensor = LARA_R6_UTIME_SENSOR_GNSS_LTE)

  myLARA.gpsEnableRmc(); // Enable GPRMC messages
}

void loop()
{
  if ((lastGpsPoll == 0) || (lastGpsPoll + GPS_POLL_RATE < millis()))
  {
    // Call (myLARA.gpsGetRmc to get coordinate, speed, and timing data
    // from the GPS module. Valid can be used to check if the GPS is
    // reporting valid data
    if (myLARA.gpsGetRmc(&gps, &spd, &clk, &valid) == LARA_R6_SUCCESS)
    {
      printGPS();
      lastGpsPoll = millis();
    }
    else
    {
      delay(1000); // If RMC read fails, wait a second and try again
    }
  }
}

void printGPS(void)
{
  Serial.println();
  Serial.println("UTC: " + String(gps.utc));
  Serial.print("Time: ");
  if (clk.time.hour < 10) Serial.print('0'); // Print leading 0
  Serial.print(String(clk.time.hour) + ":");
  if (clk.time.minute < 10) Serial.print('0'); // Print leading 0
  Serial.print(String(clk.time.minute) + ":");
  if (clk.time.second < 10) Serial.print('0'); // Print leading 0
  Serial.print(String(clk.time.second) + ".");
  if (clk.time.ms < 10) Serial.print('0'); // Print leading 0
  Serial.println(String(clk.time.ms));
  Serial.println("Latitude: " + String(gps.lat, 7));
  Serial.println("Longitude: " + String(gps.lon, 7));
  Serial.println("Speed: " + String(spd.speed, 4) + " @ " + String(spd.cog, 4));
  Serial.println("Date (MM/DD/YY): " + String(clk.date.month) + "/" + 
    String(clk.date.day) + "/" + String(clk.date.year));
  Serial.println("Magnetic variation: " + String(spd.magVar));
  Serial.println("Status: " + String(gps.status));
  Serial.println("Mode: " + String(gps.mode));
  Serial.println();
}
