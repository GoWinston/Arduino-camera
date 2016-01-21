const int ImageWidth = 128;
byte RawLine [ImageWidth] = { 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};




// START - TV Capture PRE SETUP START //
#include <TVout.h>
#include <fontALL.h>
#define W 128
#define H 96
TVout tv;
unsigned char x,y;
char s[32];
// END -  TV Capture PRE SETUP END //


// START - i2C Slave  PRE SETUP //
#include <Wire.h>
 
#define  SLAVE_ADDRESS    0x29  //slave address,any number from 0x01 to 0x7F or 1 to 127
#define  MAX_REC_BYTES    2  // sets a global on the max number of bytes of instructions to expect
// END - i2C Slave  PRE SETUP //

byte receivedCommands[MAX_REC_BYTES];
byte instructionsRecieved = 0; // flag for instructions recieved

const int CompressedLineWidth = ImageWidth/8;
byte CompressedLine [CompressedLineWidth];

byte CaptureOn = 0;
byte currentLine = 0;

const int FrameReadyPin =  7;


void setup()
{
  pinMode(FrameReadyPin, OUTPUT); 
  digitalWrite ( FrameReadyPin, LOW);
  
  Wire.begin(SLAVE_ADDRESS); 
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  // TV Capture SETUP START //
  tv.begin(NTSC, W, H);
  initOverlay();
  initInputProcessing();
  tv.select_font(font4x6);
  tv.fill(0);
  // TV Capture SETUP END //
}






void loop()
{
  
  if (instructionsRecieved != 0) {
    processInstructions ();
  }

  if (CaptureOn == 0) {
    tv.capture();
    tv.resume();
    tv.delay_frame(5);
  }
  
}






void processInstructions () {
  
  switch (receivedCommands [0]) {
    
    case 0:  // compress the instructed line
      tv.capture();
      digitalWrite (FrameReadyPin, LOW);
      transmitLine(receivedCommands[1]);
      instructionsRecieved = 0; // reset the flag
      break;

    case 1:  // Capture Control
      CaptureOn = receivedCommands [1];
      digitalWrite (FrameReadyPin, LOW);
      instructionsRecieved = 0; // reset the flag
      break;

    case 2: // 
      instructionsRecieved = 0; // reset the flag
      break;
  }
}


void transmitLine (byte LineRequested) { 
  byte currentPixel = 0;
  for (int x=0; x < ImageWidth/8 ; x++) {  // this loops for the total number of bytes 128/8=16
    byte final = 0;
    for (int y=0; y < 8 ; y++) {        // get 8 pixels and compress them into a byte
      final = final << 1; //  this will be built into the final image, move the bits over one slot
      //byte next = RawLine[currentPixel]; // <---------------------------------------------------------  INPUT RAW PIXEL
      byte next = tv.get_pixel(currentPixel,LineRequested);
      switch (next) {  // this inverts each pixel so the output image is a positive of the input
        case 0:
          next=1;
          break;
        case 1: 
          next=0;
          break;
      }
      currentPixel++;
      final = final | next;  // combine them
    }
    CompressedLine[x]=final; // saves the final compressed 8 pixels into the compressed line buffer
  }
  currentLine ++;
  digitalWrite ( FrameReadyPin, HIGH);
  delay (100); // Allows video signal to stablize after transmission
}




 
void requestEvent()
{
  if (digitalRead (FrameReadyPin) == HIGH) {
    Wire.write(CompressedLine, CompressedLineWidth);  //Set the buffer up to send all 16 bytes of data
    digitalWrite ( FrameReadyPin, LOW);
  }
}
 
void receiveEvent(int bytesReceived)
{
     for (int a = 0; a < bytesReceived; a++)
     {
          if ( a < MAX_REC_BYTES)
          {
               receivedCommands[a] = Wire.read();
               instructionsRecieved++;
          }
          else
          {
               Wire.read();  // if we receive more data then allowed just throw it away
          }
          
     }
}




























// TV Capture Utility Functions START//
void initOverlay() {
  TCCR1A = 0;
  // Enable timer1.  ICES0 is set to 0 for falling edge detection on input capture pin.
  TCCR1B = _BV(CS10);

  // Enable input capture interrupt
  TIMSK1 |= _BV(ICIE1);

  // Enable external interrupt INT0 on pin 2 with falling edge.
  EIMSK = _BV(INT0);
  EICRA = _BV(ISC11);
}
void initInputProcessing() {
  // Analog Comparator setup
  ADCSRA &= ~_BV(ADEN); // disable ADC
  ADCSRB |= _BV(ACME); // enable ADC multiplexer
  ADMUX &= ~_BV(MUX0);  // select A2 for use as AIN1 (negative voltage of comparator)
  ADMUX |= _BV(MUX1);
  ADMUX &= ~_BV(MUX2);
  ACSR &= ~_BV(ACIE);  // disable analog comparator interrupts
  ACSR &= ~_BV(ACIC);  // disable analog comparator input capture
}
ISR(INT0_vect) {
  display.scanLine = 0;
}
// TV Capture Utility Functions END //









