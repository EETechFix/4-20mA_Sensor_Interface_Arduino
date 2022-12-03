/**************************************************************************
  This interactive device is an electronic circuit that is able to sense
  the environment by using a sensor (electronic components that convert 
  real-world measurements into electrical signals). The device processes 
  the information it gets from the sensors with behaviour that’s
  described in the software (sketch). The device interact with the world 
  by using actuators, electronic components that can convert 
  an electric signal into a physical action.

  The 4-20 mA current loop has been the standard for signal transmission 
  and electronic control in control systems since the 1950's. In a current
  loop, the current signal is drawn from a dc power supply, flows through 
  the transmitter, into the controller and then back to the power supply 
  in a series circuit. The advantage is that the current value does not 
  degrade over long distances, so the current signal remains constant 
  through all components in the loop. As a result, the accuracy of the 
  signal is not affected by a voltage drop in the interconnecting wiring.
**************************************************************************/

#include "arduino_secrets.h"
#include "thingProperties.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_INA260.h> //https://github.com/adafruit/Adafruit_INA260
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // display width, in pixels
#define SCREEN_HEIGHT 64 // display height, in pixels
#define OLED_RESET    4 // set to unused pin
#define SCREEN_ADDRESS 0x3D // OLED I2C address

const float alpha = .97; // Low Pass Filter alpha (0.0 - 1.0 )
const float calCorrection = 0.1; // correct steady state error (ina260.readCurrent()
float currentVal = 0.0;

Adafruit_INA260 ina260 = Adafruit_INA260();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  // Wait until serial port is opened, native USB boards only
  delay(3000);

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }
  Serial.println("Found INA260 chip");

  // set the number of samples to average
  ina260.setAveragingCount(INA260_COUNT_16);

  // set the time over which to measure the current and bus voltage
  ina260.setVoltageConversionTime(INA260_TIME_1_1_ms);
  ina260.setCurrentConversionTime(INA260_TIME_1_1_ms);

  //  SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1); // Don't proceed, wait forever
  }
  display.clearDisplay();
  display.display();

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you'll get.
     The default is 0 (only errors).
     Maximum is 4
  */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {

  ArduinoCloud.update();
  float adcVal = (ina260.readCurrent()) + calCorrection; // Read current
  currentVal = (alpha * currentVal) + ((1.0 - alpha) * adcVal);// Low Pass Filter
  
  psiVal = round((15.0 * currentVal) + (-60.0)); // ideal linearity equation (r2 1.0) "Default"
  //psiVal = round((14.96079 * currentVal) + (-59.45698)); // linear regression calculation, sensor #1 (r2 0.9987)
  //psiVal = round((14.82831 * currentVal) + (-57.77500)); // linear regression calculation, sensor #2 (r2 0.9988)

  Serial.print("Current: ");
  Serial.println(String(currentVal, 1 ) + "mA");

  updateOLED (); // call to upadte OLED display

  delay(200);

}

void updateOLED () {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  display.setCursor(15,5);
  display.print(currentVal,1);
  display.print(" mA");
  display.display();

  display.setCursor(15,40);
  display.print(psiVal, 0);
  display.print(" psi");
  display.display();
}

/*
  set the time over which to measure the current and bus voltage
  INA260_TIME_140_us    Measurement time: 140us.
  INA260_TIME_204_us    Measurement time: 204us.
  INA260_TIME_332_us    Measurement time: 332us.
  INA260_TIME_558_us    Measurement time: 558us.
  INA260_TIME_1_1_ms    Measurement time: 1.1ms (Default)
  INA260_TIME_2_116_ms  Measurement time: 2.116ms.
  INA260_TIME_4_156_ms  Measurement time: 4.156ms.
  INA260_TIME_8_244_ms  Measurement time: 8.224ms.

  set the number of samples to average
  INA260_COUNT_1    Window size: 1 sample (Default)
  INA260_COUNT_4    Window size: 4 samples.
  INA260_COUNT_16   Window size: 16 samples.
  INA260_COUNT_64   Window size: 64 samples.
  INA260_COUNT_128  Window size: 128 samples.
  INA260_COUNT_256  Window size: 256 samples.
  INA260_COUNT_512  Window size: 512 samples.
  INA260_COUNT_1024 Window size: 1024 samples
*/
