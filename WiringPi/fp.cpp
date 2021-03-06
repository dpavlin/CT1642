/*
 * front panel
*/

#include <stdio.h>
#include <wiringPi.h>

#include <CT1642.h>

// Pins according to the schematic
#define KEY_PIN   17   // Make sure this is an Interrupt PIN
#define CLOCK_PIN 27
#define DATA_PIN  22

// Setup an instance to communicate the CT1642 IC (including key scan function).
//CT1642 ledDriver(CLOCK_PIN, DATA_PIN, KEY_PIN);
// FIXME if we init class here, wiringpi is not yet initilized
CT1642 *ledDriver;

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
      key_pressed = ledDriver->getKeyCode();
    }
    last_micros = micros();
  }
  // Enable interrupts
//  interrupts();
}

#include <iostream>
#include <string>

enum DisplayMode { Number, Time, Chars };
DisplayMode display_mode = Number;

int display_number = 0;
int display_time[] = { 23, 33 };
char display_chars[4] = { 'b', 'e', 'e', 'f' };

void test_led_segments(int digit_delay) {
  for( int digit = 1; digit <= 4; digit++ ) {
	int send = 1;
	printf("test digit %2d ", digit);
	for( int bit = 0; bit <=7; bit++ ) {
		  ledDriver->write( send , digit);
		  send = send << 1;
		  printf("%d 0x%02x ", digit, bit, send);
		  delay(digit_delay);
	}
	printf("\n");
  }
}

#include <signal.h>
#include <libconfig.h++>

using namespace std;
using namespace libconfig;

void sig_handler(int signum) {
	std::cout << "signum=" << signum << std::endl;
	if ( signum == SIGUSR1 ) {
		ledDriver->clear(); // blank display so we don't leave random digit light up

		Config cfg;

		try {
			cfg.readFile("lcd.cfg");
		} catch(const FileIOException &fioex) {
			std::cerr << "I/O error while reading file." << std::endl;
		} catch(const ParseException &pex) {
			std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
				<< " - " << pex.getError() << std::endl;
		}

		try {
			int number = cfg.lookup("number");
			cout << "number=" << number << endl << endl;
			display_number = number;
			display_mode = Number;
		} catch(const SettingNotFoundException &nfex) {
			//cerr << "No 'number' setting in configuration file." << endl;
		}

		try {
			int hour = cfg.lookup("time_hour");
			int min  = cfg.lookup("time_min");
			cout << "time=" << hour << ":" << min << endl << endl;
			display_time[0] = hour;
			display_time[1] = min;
			display_mode = Time;
		} catch(const SettingNotFoundException &nfex) {
			//cerr << "No 'time_hour' or 'time_min' setting in configuration file." << endl;
		}

		try {
			string chars = cfg.lookup("chars");
			cout << "chars=" << chars << endl << endl;
			for(int i=0; i<=3; i++)
				display_chars[i] = chars[i];
			display_mode = Chars;
		} catch(const SettingNotFoundException &nfex) {
			//cerr << "No 'chars' setting in configuration file." << endl;
		}

		try {
			int delay = cfg.lookup("delay");
			cout << "delay=" << delay << endl << endl;
			ledDriver->setPovDelay( delay );
		} catch(const SettingNotFoundException &nfex) {
			//cerr << "No 'delay' setting in configuration file." << endl;
		}

		try {
			int test = cfg.lookup("test");
			cout << "test=" << test << endl << endl;
			test_led_segments( test );
		} catch(const SettingNotFoundException &nfex) {
		}
	}
}

#include <sys/types.h>
#include <unistd.h>

int main (void)
{

	// register signals for configuration
	signal(SIGUSR1, sig_handler);

  wiringPiSetupGpio(); // BCM pin numbers, but allready set in ledDriver constructor

  printf ("CT1642 Front-panel for Raspberry Pi to reload config use: kill -SIGUSR1 %d\n", getpid()) ;

  // IC CT1642 Key Interrupt. Recomended Rising Edge.
  //attachInterrupt(digitalPinToInterrupt(KEY_PIN), btnPress, RISING);
  wiringPiISR(KEY_PIN, INT_EDGE_RISING, &btnPress);

  ledDriver = new CT1642(CLOCK_PIN, DATA_PIN, KEY_PIN); // init, after wiringPiSetup

  // Set Persistence of Vision Delay for the Display
  ledDriver->setPovDelay(5); // default = 2

  test_led_segments(50);

  btnPressed = false;
  key_pressed = 0;

  for (;;) {

	/* This is thight inner loop to refresh display, loop() in Arduino
	 * We need to call ledDriver once to refresh display
	 * */

	// Display the number of the button pressed on the last digit
	//ledDriver->showSingle(key_pressed,4);

	if ( display_mode == Number ) {
		ledDriver->showNumber(display_number);
	} else if ( display_mode == Time ) {
		ledDriver->showTime(display_time[0],display_time[1]);
	} else if ( display_mode == Chars ) {
		ledDriver->showChars(display_chars[0],display_chars[1],display_chars[2],display_chars[3]);
	}

	// button pressed
	if (btnPressed) {
	    printf("Button press read: %d 0x%02x\n", key_pressed, key_pressed);

	    // Debounce
	    btnPressed = false;
	}

  }
  return 0 ;
}
