/*
 * front panel
*/

#include <stdio.h>
#include <wiringPi.h>

#include <CT1642.h>

// Pins according to the schematic
#define CLOCK_PIN 24
#define DATA_PIN  25
#define KEY_PIN   23   // Make sure this is an Interrupt PIN

// Setup an instance to communicate the CT1642 IC (including key scan function).
CT1642 ledDriver(CLOCK_PIN, DATA_PIN, KEY_PIN);

// Variables for the Interrupt
// and Button Debouncing
static volatile boolean btnPressed ;
volatile int key_pressed;
volatile unsigned long last_micros;
long debouncing_time = 200; // in Milliseconds

// Function for the interrupt
void btnPress() {

  // Disable interrupts for now
//  noInterrupts();
  // Debounce button
  if ((long)(micros() - last_micros) >= debouncing_time * 1000) {
    if (!btnPressed) {
      btnPressed = true;
      // Get the key code from the CT1642
      key_pressed = ledDriver.getKeyCode();
    }
    last_micros = micros();
  }
  // Enable interrupts
//  interrupts();
}

int main (void)
{

  wiringPiSetupGpio(); // BCM pin numbers, but allready set in ledDriver constructor

  printf ("CT1642 Front-panel for Raspberry Pi\n") ;

  // IC CT1642 Key Interrupt. Recomended Rising Edge.
  //attachInterrupt(digitalPinToInterrupt(KEY_PIN), btnPress, RISING);
  wiringPiISR(KEY_PIN, INT_EDGE_RISING, &btnPress);

  // Set Persistence of Vision Delay for the Display
  ledDriver.setPovDelay(15);

  btnPressed = false;
  key_pressed = 0;

  for (;;)
  {
	  // Display the number of the button pressed on the last digit
	  ledDriver.showSingle(key_pressed,4);

	  // button pressed
	  if (btnPressed) {
	    // Write result to serial
	    printf("Button press read: %d 0x%02x\n", key_pressed, key_pressed);

	    // Debounce Serial
	    btnPressed = false;
	  }

	  delay(10);
  }
  return 0 ;
}
