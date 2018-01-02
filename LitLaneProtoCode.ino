/*This code operates a smart bike lane prototype for CPLN571 course at UPenn. With this code, the bike lane can tell 
 *where the location of a biker is and shine lights to protect the biker. The lane activates when the weight sensors sense the weight
 *of a bike. When the weight is too high (indicating a car/bus), the bike lane blinks yellow and after a few seconds blinks red and plays
 *an alarm. This is to notify the car to clear the bikelane.  */


#include <Time.h>
#include <TimeLib.h>
#include <FastLED.h>
#include <NewPing.h>
#include <HX711.h>
#include <NewTone.h>

//inititate settings
#define NUM_LEDS 80 
const int ledLength = 10; 
#define MAX_DISTANCE 80 
float bikeweight = 0.06;
float carweight = 1;
float blinkmax = 30;
int blinkspeed = 350;

//initiate pins
#define LED_PIN 5
#define LED_PIN2 4
#define TRIGGER_PIN  7
#define ECHO_PIN     8
#define TRIGGER_PIN2  11
#define ECHO_PIN2     12
#define buzzerPin     13
#define DOUT  3
#define CLK  2
#define DOUT2  10
#define CLK2  9

//initiate global variables
long distance1, distance2;
long distance1a, distance1b, distance1c, distance2a, distance2b, distance2c, distance1d, distance2d;
float weight1, weight1a, weight1b, weight1c;
float weight2, weight2a, weight2b, weight2c;
float blinkcount1, blinkcount2 = 0;
long previousMillis = 0;   
int ledValue1, ledValue2; 
int sensorValue;
int dimCount; 
int DimArray[10] = {240,210,150,100,0,0,100,150,210,240};

//initiate components
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];

HX711 scale(DOUT, CLK);
HX711 scale2(DOUT2, CLK2);
float calibration_factor = -7050; //-7050 worked for my 440lb max scale setup

NewPing sonar1(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE);


void setup() {
  //set up components
  FastLED.addLeds<WS2811, LED_PIN>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<WS2811, LED_PIN2>(leds2, NUM_LEDS).setCorrection( TypicalLEDStrip );
  
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(9600);

  scale.set_scale();
  scale.tare();

  scale2.set_scale();
  scale2.tare();

  long zero_factor = scale.read_average(100); //Get a baseline reading
  long zero_factor2 = scale2.read_average(100); //Get a baseline reading
  
}

void loop() {
    //obtain and smooth distance from distance sensor 1
    distance1d = distance1c;
    distance1c = distance1b;
    distance1b = distance1a;
    distance1a = sonar1.ping_cm();
    distance1 = (distance1a + distance1b + distance1c)/3;

    //obtain and smooth distance from distance sensor 2
    distance2d = distance2c;
    distance2c = distance2b;
    distance2b = distance2a;
    distance2a = sonar2.ping_cm();
    distance2 = (distance2a + distance2b + distance2c)/3;

    //obtain and smooth weight from load sensor 1
    scale.set_scale(calibration_factor); //Adjust to this calibration factor
    weight1a = weight1b;
    weight1b = weight1c;
    weight1c = scale.get_units();
    weight1 = (weight1a + weight1b + weight1c)/3;

    //obtain and smooth weight from load sensor 2
    scale2.set_scale(calibration_factor); //Adjust to this calibration factor
    weight2a = weight2b;
    weight2b = weight2c;
    weight2c = scale2.get_units();
    weight2 = (weight2a + weight2b + weight2c)/3;
    
    //Map distance values to reflect correct location on the LED strips
    ledValue1 = map(distance1, 0, 55, 0, 35);
    ledValue1 = abs(ledValue1 - 35);
    
    ledValue2 = map(distance2, 0, 55, 42, 70);

    Serial.println(weight1);
    Serial.println(weight2);

    //delay(250);
    
  
    //Clear past LED configuration and reset fade index
    FastLED.clear();
    dimCount = 0;
    
    
    //configure lights in first section of LED strip depending on weight and times blinked
    if(weight1 > bikeweight){

      //condition for yellow lights
      if(weight1 >= carweight && blinkcount1 < blinkmax){

          //start time to configure yellow lights blinking 
          unsigned long currentMillis = millis();

          //turn yellow lights on when millis is between 500 and 1000
          if((currentMillis - previousMillis >= blinkspeed) && (currentMillis - previousMillis < (blinkspeed*2))){
            for(int led = ledValue1-3; led < ledValue1+7; led++) { 
              leds[led] = CRGB(100,255,0); 
              leds[led].fadeToBlackBy(DimArray[dimCount]);
              leds2[led + 1] = CRGB(100,255,0); 
              leds2[led + 1].fadeToBlackBy(DimArray[dimCount]);
              dimCount = dimCount + 1;}
              
              blinkcount1 = blinkcount1 + 1;}

          //turn lights off when millis is smaller then 500
          else if(currentMillis - previousMillis < blinkspeed){
            for(int led = ledValue1-3; led < ledValue1+7; led++) { 
              leds[led] = CRGB::Black; 
              leds2[led + 1] = CRGB::Black;} 
              } 

          //When greater than a 100, reset millis 
          else{
              previousMillis = currentMillis;
              }}

       //condition for red lights
       else if(weight1 >= carweight && blinkcount1 >= blinkmax){
          //start time to configure red lights blinking 
          unsigned long currentMillis = millis();
          if((currentMillis - previousMillis >= blinkspeed) && (currentMillis - previousMillis < (blinkspeed*2))){
            for(int led = ledValue1-3; led < ledValue1+7; led++) { 

              //unlike the yellow lights, red lights include a sound response 
              NewTone(buzzerPin, 523, 100);
              
              leds[led] = CRGB(0,255,0); 
              leds[led].fadeToBlackBy(DimArray[dimCount]);
              leds2[led + 1] = CRGB(0,255,0); 
              leds2[led + 1].fadeToBlackBy(DimArray[dimCount]);
              dimCount = dimCount + 1;}
              dimCount = 0;
              }
              
           else if(currentMillis - previousMillis < blinkspeed){
             for(int led = ledValue1-3; led < ledValue1+7; led++) { 
              leds[led] = CRGB::Black; 
              leds2[led + 1] = CRGB::Black;} 
              } 
           else{
              previousMillis = currentMillis;
              }}
            

        else{
          for(int led = ledValue1-3; led < ledValue1+7; led++) { 
            leds[led] = CRGB::White; 
            leds[led].fadeToBlackBy(DimArray[dimCount]);
            leds2[led + 1] = CRGB::White; 
            leds2[led + 1].fadeToBlackBy(DimArray[dimCount]);
            dimCount = dimCount + 1;
            blinkcount1 = 0;}}
          }

     //reset dim index in preparation for activating the second portion of the LED strip (the same way as the first) 
     dimCount = 0;
     
     if(weight2 >= bikeweight){
        if(weight2 >= carweight && blinkcount2 < blinkmax){
          unsigned long currentMillis = millis();
          if((currentMillis - previousMillis >= blinkspeed) && (currentMillis - previousMillis < (blinkspeed*2))){
            for(int led = ledValue2-3; led < ledValue2+7; led++) { 
              leds[led] = CRGB(100,255,0); 
              leds[led].fadeToBlackBy(DimArray[dimCount]);
              leds2[led + 1] = CRGB(100,255,0); 
              leds2[led + 1].fadeToBlackBy(DimArray[dimCount]);
              dimCount = dimCount + 1;}
              
              blinkcount2 = blinkcount2 + 1;}
          else if(currentMillis - previousMillis < blinkspeed){
            for(int led = ledValue2-3; led < ledValue2+7; led++) { 
              leds[led] = CRGB::Black; 
              leds2[led + 1] = CRGB::Black;} 
              }
          else{
              previousMillis = currentMillis;
              }}
              
        else if(weight2 >= carweight && blinkcount2 >= blinkmax){
          unsigned long currentMillis = millis();
          if((currentMillis - previousMillis >= blinkspeed) && (currentMillis - previousMillis < (blinkspeed*2))){
            for(int led = ledValue2-3; led < ledValue2+7; led++) {
              NewTone(buzzerPin, 523, 100); 
              leds[led] = CRGB(0,255,0); 
              leds[led].fadeToBlackBy(DimArray[dimCount]);
              leds2[led + 1] = CRGB(0,255,0); 
              leds2[led + 1].fadeToBlackBy(DimArray[dimCount]);
              dimCount = dimCount + 1;}
              dimCount = 0;
              }//Think about changing the location of this to stop the flickering.
          else if(currentMillis - previousMillis < blinkspeed){
            for(int led = ledValue2-3; led < ledValue2+7; led++) { 
              leds[led] = CRGB::Black; 
              leds2[led + 1] = CRGB::Black;} 
              }//Think about changing the location of this to stop the flickering. 
          else{
              previousMillis = currentMillis;
              }}
              
        else{
          for(int led = ledValue2-3; led < ledValue2+7; led++) { 
            leds[led] = CRGB::White; 
            leds[led].fadeToBlackBy(DimArray[dimCount]);
            leds2[led + 1] = CRGB::White; 
            leds2[led + 1].fadeToBlackBy(DimArray[dimCount]);
            dimCount = dimCount + 1;
            blinkcount2 = 0;}}
     }

    
FastLED.show();

}
