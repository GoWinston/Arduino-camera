#include <Wire.h>
#define  CAM_ADDRESS   0x29

#include "SoftwareSerial.h"
SoftwareSerial directPrinter(5, 6);

const int FrameReadyPin =  7;
const int buttonShutter = 8;     // the number of the pushbutton pin
const int ledRed =  13;      // the number of the LED pin
const int ledGreen =  12;      // the number of the LED pin

boolean debugging = false;

const byte ImageWidth = 128;
const byte ImageHeight = 96;

const int RecieveBufferSize = 16;  // 128 bits / 8 bits per byte = 16 bytes
byte RecieveBuffer [RecieveBufferSize];  // storage of recieved image in bytes 128/8 =16

const int OutputBufferSize = (48);  // max printer bits = 48 per datasheet
byte OutputBuffer [OutputBufferSize];  // storage of compressed image as bytes


/*
WIRES
Pin = 8;  // This is the Shutter Button
Pin = 7;  // This is the frame ready connection
Pin = 6;  // This is the yellow wire
Pin = 5;  // This is the green wire
*/

void setup()
{
  Wire.begin();   // join i2c bus (address not needed, thank you master)
  
  Serial.begin(9600);  // start serial for output
  Serial.println("ArduinoCam Master 3");
  
  directPrinter.begin(19200);
  
  pinMode(FrameReadyPin, INPUT);
  pinMode(ledRed, OUTPUT);   
  pinMode(ledGreen, OUTPUT);   
  pinMode(buttonShutter, INPUT);    

  delay (4000); // allows the slave to get started up so we stay in snyc  REALLY THE BEGINNING OF THE LOOP!
  
  digitalWrite(ledRed, LOW); 
  digitalWrite(ledGreen, HIGH);  
}

void loop() {
  if (digitalRead(buttonShutter) == HIGH) {       
     delay (100);
     if (digitalRead(buttonShutter) == HIGH) {
       CaptureImage (); 
     }
  }   
} // END OF LOOP

void CaptureImage () {
  
  digitalWrite(ledRed, HIGH); 
  digitalWrite(ledGreen, LOW);  
  
  ///////  START  MAKING AN IMAGE //////////
  
  // TELL THE CAMERA TO ENTER CAPTURE MODE
  Wire.beginTransmission(CAM_ADDRESS);
  Wire.write(0x01);
  Wire.write(0x01);
  Wire.endTransmission();
  
  //delay (500);
  
  //This is the loop that captures the image one line at a time /////
  for (int currentLine = 0; currentLine < ImageHeight; currentLine++) {
    
  ///////  START - get a line from the camera //////////
  Wire.beginTransmission(CAM_ADDRESS);
  Wire.write(0x00);
  Wire.write(currentLine);
  Wire.endTransmission();
  
  do {
    //delay (500);
  } while (digitalRead (FrameReadyPin) == LOW);
  
  if (digitalRead (FrameReadyPin) == HIGH ) {
    //delay (250);
    Wire.requestFrom(CAM_ADDRESS,16);
    for (int i = 0; i < 16; i++) {
      RecieveBuffer[i] = Wire.read();  // put each byte into the recieve buffer
    }
  }
  
  //delay (250); // Give the arduino time to digetst
  
  ///////  END - get a line from the camera //////////
  
/*
  ////////  START - this is a debugging function that displays the recieve buffer - comment out for final //////////
    Serial.println ();
    Serial.println ("Here is everything in RecieveBuffer: ");
    Serial.println ();
    for ( int x = 0; x < RecieveBufferSize; x++) {
      Serial.print (RecieveBuffer [x]);
      Serial.print(",");
    }
    // line break at end
     Serial.println ();
     ////////  END - this is a debugging function that displays the recieve buffer - comment out for final //////////
*/



    ////////  START - compress the recieved line for printing and saving //////////
    byte LocationInRecieveBuffer = 0; // MAX IS 16
    byte LocationIncompressionTemp = 0;  // MAX is 128
    
    byte compressionTemp [128]; // this is a temporary store for the array for the uncompressed image
    
      for (int i = 0 ; i < 16 ; i++ ) {
        for (int x=0; x < 8 ; x++) {
          byte bitshifter = RecieveBuffer [LocationInRecieveBuffer];
          bitshifter = bitshifter << x;
          bitshifter = bitshifter >> 7;
          compressionTemp[LocationIncompressionTemp] = bitshifter;
          LocationIncompressionTemp++;
        }
        LocationInRecieveBuffer++;
      }
     ////////  END - compress the recieved line for printing and saving //////////


    ////////  START - this is a debugging function that displays the compressed buffer - comment out for final //////////
    Serial.println ();
    Serial.println ("Here is everything in compressionTemp: ");
    Serial.println ();
    for ( int x = 0; x < 128; x++) {
      Serial.print (compressionTemp [x]);
      Serial.print(",");
    }
    // line break at end
     Serial.println ();
     ////////  END - this is a debugging function that displays the recieve buffer - comment out for final //////////
  
        
     
     
    ////////  START - compress the recieved line for printing and saving //////////
    LocationIncompressionTemp = 0;
    int LocationInOutputBuffer = 0;

    for (int x=0; x < OutputBufferSize; x++) {
      byte final = 0;
      byte prev;   
      byte next = compressionTemp[LocationIncompressionTemp]; // <---------  INPUT RAW PIXEL
      prev = prev << 1; //  move it over
      prev = prev |= next;  // combine them
      next = compressionTemp[LocationIncompressionTemp]; // <---------  INPUT RAW PIXEL
      prev = prev << 1; //  move it over
      prev = prev |= next;  // combine them
      next = compressionTemp[LocationIncompressionTemp]; // <---------  INPUT RAW PIXEL
      prev = prev << 1; //  move it over
      prev = prev |= next;  // combine them
      LocationIncompressionTemp++; // move to the next pixel on the x axis
      next = compressionTemp[LocationIncompressionTemp]; // <---------  INPUT RAW PIXEL
      prev = prev << 1; //  move it over
      prev = prev |= next;  // combine them
      next = compressionTemp[LocationIncompressionTemp]; // <---------  INPUT RAW PIXEL
      prev = prev << 1; //  move it over
      prev = prev |= next;  // combine them
      next = compressionTemp[LocationIncompressionTemp]; // <---------  INPUT RAW PIXEL
      prev = prev << 1; //  move it over
      prev = prev |= next;  // combine them
      LocationIncompressionTemp++; // move to the next pixel on the x axis
      next = compressionTemp[LocationIncompressionTemp]; // <---------  INPUT RAW PIXEL
      prev = prev << 1; //  move it over
      prev = prev |= next;  // combine them
      next = compressionTemp[LocationIncompressionTemp]; // <---------  INPUT RAW PIXEL
      prev = prev << 1; //  move it over
      prev = prev |= next;  // combine them
      LocationIncompressionTemp++; // move to the next pixel on the x axis
      
      OutputBuffer[LocationInOutputBuffer] = prev;
      LocationInOutputBuffer++;  
    }
    ////////  END - compress the recieved line for printing and saving //////////

  
    ////////  START - this is a debugging function that displays the output buffer - comment out for final //////////
      Serial.println ();
      Serial.println ("Here is everything in OutputBuffer: ");
      Serial.println ();
      for ( int x = 0; x < OutputBufferSize; x++) {
        Serial.print (OutputBuffer [x]);
        Serial.print(",");
      }
      // line break at end
       Serial.println ();
     ////////  END - this is a debugging function that displays the compressed buffer - comment out for final //////////
     
    
     
     
     
     ////////  START - This is the section actually prints the recieved line //////////
     if (debugging) {
        Serial.println("PrintOutputBuffer just started!");
      }
        directPrinter.write(18); //DC2
        directPrinter.write(86); //V
        directPrinter.write(3); //nL = 1*5
        directPrinter.write((byte)0x0); //nH = 0  <==== Fixing Compiler error here
        //Now we need 1 rows, or 3 nL in height
        for(int y = 0 ; y <  3 ; y++) {
          byte currentByte = 0;
          //Now we need 48 bytes total
          for(int x = 0 ; x < 48 ; x++ , currentByte++) {
            directPrinter.write(OutputBuffer[currentByte]);
          }
        }
      if (debugging) {
        Serial.println("PrintOutputBuffer just ended!");
      }
     ////////  END - This is the section actually prints the recieved line //////////

  } ///////  END - This is the loop that will capture an entire image //////////
  
  // Turn Off CAMERA Capture Mode
  Wire.beginTransmission(CAM_ADDRESS);
  Wire.write(0x01);
  Wire.write(0x00);
  Wire.endTransmission();
  
  
  ///////  START -  This feeds the paper at the end of the image capture  //////////
  directPrinter.write(10);
  directPrinter.write(10);
  ///////  END -  This feeds the paper at the end of the image capture  //////////
  
  digitalWrite(ledRed, LOW); 
  digitalWrite(ledGreen, HIGH);  
  
}


//////////////////////////////////////////////////////////////////////////////////////////































