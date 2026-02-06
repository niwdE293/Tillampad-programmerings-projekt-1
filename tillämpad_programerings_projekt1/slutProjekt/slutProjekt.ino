/*
  Project: Clock and temperature project
  Author: Edwin Ädel
  Date: 2025-11-07
  Description: Displays the time on a led ring and displays tempreture on a screen and servo.
*/

// Include libraries 
#include "RTClib.h"
#include "U8glib.h"
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// Defines the connected led pin on the arduino and led count(number of lights on the led ring)
#define LED_PIN    6
#define LED_COUNT 24

#define TRIG1 12
#define ECHO1 13
#define TRIG2 11
#define ECHO2 10
#define TRIG3 9
#define ECHO3 8
#define TRIG4 7
#define ECHO4 6
#define TRIG5 2
#define ECHO5 1


// Constructs objects
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);
RTC_DS3231 rtc;
Adafruit_NeoPixel ring(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

const int lightSensorPin = A0;
const int lightThreshHold = 900;

const int rightMotor = 3; // relay controlling motor
const int leftMotor = 5; // relay controlling motor

const int targetDistance = 10; // Target distance in cm.

const float driveMargin = 0.2;
int wallAngleMargin = 15;


const float diagonalSensorOffset = 5.8; //7.0876
const float sideSensorOffset = 6.3805;
const float frontSensorOffset = 7.3805;



void setup(){
  Serial.begin(9600);
  u8g.setFont(u8g_font_6x12);

  pinMode(lightSensorPin, INPUT);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(rightMotor, OUTPUT);
  pinMode(leftMotor, OUTPUT);

  ring.begin();                       
  ring.setBrightness(0); //Brightness: 0 - 255  
}


// Sets the light color depending on the temperature.
void ledRingBrightness(){

  for(int i = 0; i < 24; i++){
        ring.setPixelColor(i, 255, 255, 255);
  }

  int inverted = analogRead(lightSensorPin);
  int lightValue = map(inverted, lightThreshHold, 1023, 0, 100);

  if (inverted < lightThreshHold){
    lightValue = 0;
  }
  
  Serial.println(lightValue);
  ring.setBrightness(lightValue); //Brightness: 0 - 255
  ring.show(); // Turns on the lights.
  ring.clear(); // Clears the lights.
}



// Reads the right distance and converts it to cm.
float rightDistance(){
  digitalWrite(TRIG1, LOW); delayMicroseconds(2);
  digitalWrite(TRIG1, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG1, LOW);
  return (pulseIn(ECHO1, HIGH) * 0.0343 / 2) + sideSensorOffset; // distance in cm
}

// Reads the diagonal right distance and converts it to cm.
float diagonalRightDistance(){
  digitalWrite(TRIG2, LOW); delayMicroseconds(2);
  digitalWrite(TRIG2, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG2, LOW);
  return (pulseIn(ECHO2, HIGH) * 0.0343 / 2) + diagonalSensorOffset; // distance in cm
}

// Reads the front distance and converts it to cm.
/*
float frontDistance(){
  digitalWrite(TRIG3, LOW); delayMicroseconds(2);
  digitalWrite(TRIG3, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG3, LOW);
  return (pulseIn(ECHO3, HIGH) * 0.0343 / 2) + diagonalSensorOffset; // distance in cm
}


// Reads the diagonal left distance and converts it to cm.
float diagonalLeftDistance(){
  digitalWrite(TRIG4, LOW); delayMicroseconds(2);
  digitalWrite(TRIG4, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG4, LOW);
  return (pulseIn(ECHO4, HIGH) * 0.0343 / 2) + diagonalSensorOffset; // distance in cm
}

// Reads the left distance and converts it to cm.
float leftDistance(){
  digitalWrite(TRIG5, LOW); delayMicroseconds(2);
  digitalWrite(TRIG5, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG5, LOW);
  return (pulseIn(ECHO5, HIGH) * 0.0343 / 2) + diagonalSensorOffset; // distance in cm
}
*/


// Calculates the angle of the car compared to the wall.
float getWallAngle(){
  float wallAngle;

  float sin20 = 0.3420201;
  float cos20 = 0.9396926;
  
  float height = diagonalRightDistance() * sin20;
  float base = (diagonalRightDistance() * cos20) - rightDistance();
  
  wallAngle = atan2(base, height) * (180 / PI);
  return wallAngle;
}



void goForward(){
  digitalWrite(leftMotor, HIGH);
  digitalWrite(rightMotor, HIGH);
  Serial.println("Going Forward");
}

void goRight(){
  digitalWrite(leftMotor, HIGH);
  digitalWrite(rightMotor, LOW);
  Serial.println("Turning Right");
}

void goLeft(){
  digitalWrite(leftMotor, LOW);
  digitalWrite(rightMotor, HIGH);
  Serial.println("Turning Left");
}



// This function drives the car.
void drive () {
  float rightDistanceFromCar = rightDistance() - sideSensorOffset;
  //float leftDistanceFromCar = leftDistance() - sideSensorOffset;
  float leftDistanceFromCar = 100000;
  
  Serial.println("Car angle");
  Serial.println(getWallAngle());
  Serial.println("Right distance from car");
  Serial.println(rightDistanceFromCar);
  //Serial.println("Left distance from car");
  //Serial.println(leftDistanceFromCar);
  delay(1000);

  if (rightDistanceFromCar < leftDistanceFromCar){
    // Right Side 
    if (rightDistanceFromCar < (targetDistance * (1 - driveMargin))){ 
      goLeft();
    }

    else if (rightDistanceFromCar > (targetDistance * (1 + driveMargin))){ 
      if (getWallAngle() < -35){
        goForward();
        Serial.println("Under -35 degrees");
      }
      else {
        goRight();
        Serial.println("Over -35 degrees");
      }
    }
    else {
      Serial.println("Else funtion");
      goForward();
    }
  }

  else {
    //Left side
    if (leftDistanceFromCar < (targetDistance * (1 - driveMargin)) || (getWallAngle() < - wallAngleMargin)){ 
      goRight();
    }
    else if (leftDistanceFromCar > (targetDistance * (1 + driveMargin)) || (getWallAngle() > wallAngleMargin)){ 
      if (getWallAngle() < -35){
        goForward();
      }
      else {
        goLeft();
      }
    }
    else {
      goForward();
    }
  }
}



void loop(){
  ledRingBrightness(); // This function updates the background light color using the current temperature.

  drive(); // This function drives the car.
}

