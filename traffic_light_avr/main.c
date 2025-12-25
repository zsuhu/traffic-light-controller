#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

/* Timer setup
Calculations for timer interrupt every 1 ms:
System clock = 16 MHz
Prescalar = 64
Timer 0 speed: 16 MHz / 64 = 250,000 Hz
Number of ticks every 1 ms: 50,000 / 1000 = 250 counts
*/
volatile uint32_t millisCount = 0;	// Counter that keeps track of current time in milliseconds

void timer0Init(void) {
	// Reset control registers to disable all functions
	TCCR0A = 0;
	TCCR0B = 0;

	// Initialize register settings for timer 0
	TCCR0A = (1 << WGM01); // Enable CTC mode (timer counter cleared when it reaches target value)
	TCCR0B = (1 << CS01) | (1 << CS00); // Set prescaler to 64

	// Enable timer interrupts
	TIMSK0 = (1 << OCIE0A); // Enable compare match mode on register A

	// Set target value to compare to and trigger interrupt
	OCR0A = 249; // Every 1 ms (0 -> 250 ticks)
}

ISR(TIMER0_COMPA_vect) { // Run during each timer intrrupt
	millisCount++;
}

uint32_t millis(void) { // Return the current time count in ms
	// Safely get millis_count value, ensures variable isnt changed mid read / mid write
	uint32_t m;
	cli(); 					// Disable interrupts
	m = millisCount;
	sei(); 					// Re enable interrupts
	return m;
}

// Initialize input signals
_Bool buttonState = 0;                      // Crosswalk button
const long timerShortInterval = 1500;     // Short time pulse (yellow), 1.5 seconds
const long timerLongInterval = 5000;      // Long time pulse (green), 5 seconds
unsigned long startTime = 0;              // Timer
const unsigned long flashInterval = 150;  // 0.15 second blink
unsigned long startFlashTime = 0;         // Flash timer

// Declare states
enum State {
	S0,     // Traffic light green, crosswalk light stop
	S1,     // Traffic light yellow, crosswalk light stop
	S2,     // Traffic light red, crosswalk light walk
	S3      // Traffic light red, crosswalk light flashing
};
enum State currentState = S0; // Starting state

// State outputs
	void stateOutput(enum State state) {
  		// Start with all LEDs off
  		PORTH &= ~(1 << PORTH4);
  		PORTH &= ~(1 << PORTH5);
  		PORTH &= ~(1 << PORTH6);
  		PORTE &= ~(1 << PORTE3);

  		// Turn on LEDs for each state
  		switch (state) {
    		case S0: PORTH |= (1 << PORTH4); break; // Green, Stop

    		case S1: PORTH |= (1 << PORTH5); break; // Yellow, Stop

    		case S2: PORTH |= (1 << PORTH6); PORTE |= (1 << PORTE3); break; // Red, Walk
    
    		case S3: PORTH |= (1 << PORTH6); break; // Red, Flashing(handled in state transition)
  		}
	}


int main(void) {
	// Setup
	// Set Data Direction Registers as input/output
	DDRH |= (1 << DDH6);	// Red
	DDRH |= (1 << DDH5);	// Yellow
	DDRH |= (1 << DDH4);	// Green
	DDRE &= ~(1 << DDE4);	// Button
	DDRE |= (1 << DDE3);	// Crosswalk

	// Start timer, set default state
	timer0Init();
	sei();
	startTime = millis();
  	stateOutput(currentState);

  	// Loop
  	while(1) {
  		// Read input signals
  		unsigned long currentTime = millis();
  		buttonState = (PINE & (1 << PINE4));

	  	// State transitions
	  	switch (currentState) {
	      	case S0: 
	        	if ((currentTime - startTime >= timerLongInterval) && (buttonState)) { // Long time interval expires and crosswalk button pressed
		          	currentState = S1;
		          	startTime = currentTime; // Reset timer
		          	stateOutput(currentState);
	        	}
	        	break;

	      	case S1:
		        if (currentTime - startTime >= timerShortInterval) { // Short time interval expires
		        	currentState = S2;
		          	startTime = currentTime; 
		          	stateOutput(currentState);
		        }
		        break;

	      	case S2:
		        if (currentTime - startTime >= timerLongInterval) { // Long time interval expires
			        currentState = S3;
			        startTime = currentTime; 
			        stateOutput(currentState);
		        }
		        break;

	      	case S3: {
	        	unsigned long currentFlashTime = millis();

		        if (currentTime - startTime >= timerShortInterval) { // Short time interval expires
		        	currentState = S0;
		          	startTime = currentTime; 
		          	stateOutput(currentState);

		        } else if (currentFlashTime - startFlashTime >= flashInterval) { // Flash every interval amount
		          	startFlashTime = currentFlashTime;
		          	PORTE ^= (1 << PORTE3);
		        }
		        break;
		    }
		}
	}

}