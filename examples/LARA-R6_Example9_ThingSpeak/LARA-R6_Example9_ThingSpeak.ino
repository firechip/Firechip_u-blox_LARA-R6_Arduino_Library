/*

  LARA-R6 Example
  ===============

  ThingSpeak (HTTP POST / GET)

  Written by: Paul Clark
  Date: November 18th 2020

  This example uses the LARA's mobile data connection to send random temperatures to ThingSpeak using HTTP POST or GET.
  https://thingspeak.com/

  You will need to:
    Create a ThingSpeak User Account – https://thingspeak.com/login
    Create a new Channel by selecting Channels, My Channels, and then New Channel
    Note the Write API Key and copy&paste it into myWriteAPIKey below
  The random temperature reading will be added to the channel as "Field 1"

  Feel like supporting open source hardware?
  Buy a board from firechip!

  Licence: MIT
  Please see LICENSE.md for full details

*/

#include <IPAddress.h>

// ThingSpeak via HTTP POST / GET

String myWriteAPIKey = "PFIOEXW1VF21T7O6"; // Change this to your API key

String serverName = "api.thingspeak.com"; // Domain Name for HTTP POST / GET

// LARA-R6

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

// processHTTPcommandResult is provided to the LARA-R6 library via a 
// callback setter -- setHTTPCommandCallback. (See the end of setup())
void processHTTPcommandResult(int profile, int command, int result)
{
  Serial.println();
  Serial.print(F("HTTP Command Result:  profile: "));
  Serial.print(profile);
  Serial.print(F("  command: "));
  Serial.print(command);
  Serial.print(F("  result: "));
  Serial.print(result);
  if (result == 0)
    Serial.print(F(" (fail)"));
  if (result == 1)
    Serial.print(F(" (success)"));
  Serial.println();

  // Get and print the most recent HTTP protocol error
  int error_class;
  int error_code;
  myLARA.getHTTPprotocolError(0, &error_class, &error_code);
  Serial.print(F("Most recent HTTP protocol error:  class: "));
  Serial.print(error_class);
  Serial.print(F("  code: "));
  Serial.print(error_code);
  if (error_code == 0)
    Serial.print(F(" (no error)"));
  Serial.println();

  // Read and print the HTTP POST result
  String postResult = "";
  myLARA.getFileContents("post_response.txt", &postResult);
  Serial.print(F("HTTP command result was: "));
  Serial.println(postResult);

  Serial.println();
}

void setup()
{
  String currentOperator = "";

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

  // First check to see if we're connected to an operator:
  if (myLARA.getOperator(&currentOperator) == LARA_R6_SUCCESS)
  {
    Serial.print(F("Connected to: "));
    Serial.println(currentOperator);
  }
  else
  {
    Serial.print(F("The LARA is not yet connected to an operator. Please use the previous examples to connect. Or wait and retry. Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // Deactivate the PSD profile - in case one is already active
  if (myLARA.performPDPaction(0, LARA_R6_PSD_ACTION_DEACTIVATE) != LARA_R6_SUCCESS)
  {
    Serial.println(F("Warning: performPDPaction (deactivate profile) failed. Probably because no profile was active."));
  }

  // Load the PSD profile from NVM - these were saved by a previous example
  if (myLARA.performPDPaction(0, LARA_R6_PSD_ACTION_LOAD) != LARA_R6_SUCCESS)
  {
    Serial.println(F("performPDPaction (load from NVM) failed! Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // Activate the PSD profile
  if (myLARA.performPDPaction(0, LARA_R6_PSD_ACTION_ACTIVATE) != LARA_R6_SUCCESS)
  {
    Serial.println(F("performPDPaction (activate profile) failed! Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // Reset HTTP profile 0
  myLARA.resetHTTPprofile(0);
  
  // Set the server name
  myLARA.setHTTPserverName(0, serverName);
  
  // Use HTTPS
  myLARA.setHTTPsecure(0, false); // Setting this to true causes the POST / GET to fail. Not sure why...

  // Set a callback to process the HTTP command result
  myLARA.setHTTPCommandCallback(&processHTTPcommandResult);
}

void loop()
{
  float temperature = ((float)random(2000,3000)) / 100.0; // Create a random temperature between 20 and 30

//---

  // Send data using HTTP POST
  String httpRequestData = "api_key=" + myWriteAPIKey + "&field1=" + String(temperature);

  Serial.print(F("POSTing a temperature of "));
  Serial.print(String(temperature));
  Serial.println(F(" to ThingSpeak"));
        
  // Send HTTP POST request to /update. The reponse will be written to post_response.txt in the LARA's file system
  myLARA.sendHTTPPOSTdata(0, "/update", "post_response.txt", httpRequestData, LARA_R6_HTTP_CONTENT_APPLICATION_X_WWW);

//---

//  // Send data using HTTP GET
//  String path = "/update?api_key=" + myWriteAPIKey + "&field1=" + String(temperature);
//
//  Serial.print(F("Send a temperature of "));
//  Serial.print(String(temperature));
//  Serial.println(F(" to ThingSpeak using HTTP GET"));
//        
//  // Send HTTP POST request to /update. The reponse will be written to post_response.txt in the LARA's file system
//  myLARA.sendHTTPGET(0, path, "post_response.txt");
  
//---

  // Wait for 20 seconds
  for (int i = 0; i < 20000; i++)
  {
    myLARA.poll(); // Keep processing data from the LARA so we can catch the HTTP command result
    delay(1);
  }
}
