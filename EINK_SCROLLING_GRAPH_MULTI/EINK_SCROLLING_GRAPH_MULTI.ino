/**
 *  Draws a scrolling graph of a three inputs on A0, A1, and A2
 *
 *  By Jon E. Froehlich
 *  @jonfroehlich
 *  http://makeabilitylab.io
 *
 */
#include <Arduino.h>

#include <Wire.h>
#include <SPI.h>

#include "SparkFun_SCD30_Arduino_Library.h"  // Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 airSensor;

#include <Adafruit_GFX.h>
#include <Adafruit_EPD.h>


#include "SLGM.h"  // from Makeability Lab Arduino Library

#define POWER_ENABLE WB_IO2
#define EPD_MOSI MOSI
#define EPD_MISO -1  // not use
#define EPD_SCK SCK
#define EPD_CS SS
#define EPD_DC WB_IO1
#define SRAM_CS -1    // not use
#define EPD_RESET -1  // not use
#define EPD_BUSY WB_IO6
#define SCREEN_WIDTH 250   // OLED display width, in pixels
#define SCREEN_HEIGHT 122  // OLED display height, in pixels


// 2.13" EPD with SSD1680
Adafruit_SSD1680 _display(SCREEN_WIDTH, SCREEN_HEIGHT, EPD_MOSI,
                          EPD_SCK, EPD_DC, EPD_RESET,
                          EPD_CS, SRAM_CS, EPD_MISO,
                          EPD_BUSY);


const int ANALOG_INPUT_PIN1 = A0;
const int ANALOG_INPUT_PIN2 = A1;
const int ANALOG_INPUT_PIN3 = A2;

const int MIN_ANALOG_INPUT = 0;
const int MAX_ANALOG_INPUT = 100;
const int DELAY_LOOP_MS = 20000;  // change to slow down how often to read and graph value

// for tracking fps
float _fps = 0;
unsigned long _frameCount = 0;
unsigned long _fpsStartTimeStamp = 0;
boolean _drawFps = false;

const int NUM_GRAPH_LINES = 2;
PointSymbol GRAPH_SYMBOLS[NUM_GRAPH_LINES] = { CIRCLE, SQUARE };  // PICO, it was const PointSymbol ...
ScrollingLineGraphMultiValue _scrollingLineGraph(NUM_GRAPH_LINES, GRAPH_SYMBOLS);

void setup() {
  pinMode(POWER_ENABLE, INPUT_PULLUP);
  digitalWrite(POWER_ENABLE, HIGH);

  // Initialize Serial for debug output
  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial) {
    if ((millis() - timeout) < 2000) {
      delay(100);
    } else {
      break;
    }
  }

  /* WisBLOCK 5801 Power On*/
  pinMode(WB_IO1, OUTPUT);
  digitalWrite(WB_IO1, HIGH);
  /* WisBLOCK 5801 Power On*/
  Wire.begin();

  if (airSensor.begin() == false) {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Air sensor detected...");


  _display.begin();

  // Clear the buffer
  _display.clearBuffer();

  _display.setTextSize(1);
  //    _display.setTextColor(WHITE, EPD_BLACK);
  _display.setTextColor(EPD_BLACK, EPD_WHITE);
  _display.setCursor(15, 15);
  _display.println("Screen ON!");
  _display.display();
  delay(500);
  _display.clearBuffer();

  // if (_drawStatusBar) {  //PICO
  //   _graphHeight = SCREEN_HEIGHT - 10;
  // }

  _fpsStartTimeStamp = millis();
}

void loop() {
  // Clear the display on each frame.
  _display.clearBuffer();

  // Read and store the analog data into a circular buffer
  //int analogVal = analogRead(ANALOG_INPUT_PIN1);
  int analogVal = airSensor.getHumidity();
  _scrollingLineGraph.addData(0, analogVal);
  Serial.println(analogVal);

  delay(100);
  //analogVal = analogRead(ANALOG_INPUT_PIN2);
  analogVal = airSensor.getTemperature();
  _scrollingLineGraph.addData(1, analogVal);
  Serial.println(analogVal);
  delay(100);


  if (_drawFps) {
    drawFps();
  }

  _scrollingLineGraph.drawLegend(_display, 0, 0);
  _scrollingLineGraph.draw(_display);

  _display.display();

  //calcFrameRate();

  delay(DELAY_LOOP_MS);
}

/**
 * Call this every frame to calculate frame rate
 */
void calcFrameRate() {

  unsigned long elapsedTime = millis() - _fpsStartTimeStamp;
  _frameCount++;
  if (elapsedTime > 1000) {
    _fps = _frameCount / (elapsedTime / 1000.0);
    _fpsStartTimeStamp = millis();
    _frameCount = 0;
  }
}

/**
 * Draws the status bar at top of screen with fps
 */
void drawFps() {

  // Draw frame count
  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds("XX.X fps", 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() - w, 0);
  _display.print(_fps, 1);
  _display.print(" fps");
}