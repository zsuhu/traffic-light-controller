// Initialize pin numbers
const int red = 9;
const int yellow = 8;
const int green = 7;
const int button = 2;
const int crosswalk = 5;

// Initialize input signals
int buttonState = 0;                      // Crosswalk button
const long timerShortInterval = 1500;     // Short time pulse (yellow), 1.5 seconds
const long timerLongInterval = 5000;      // Long time pulse (green), 5 seconds
unsigned long startTime = 0;              // Timer
const unsigned long flashInterval = 150;  // 0.15 second blink
unsigned long startFlashTime = 0;         // Flash timer
bool flashState = false;                  // Flash state of crosswalk light (true/false) -> (HIGH/LOW)

// Declare states
enum class State {
  S0,     // Traffic light green, crosswalk light stop
  S1,     // Traffic light yellow, crosswalk light stop
  S2,     // Traffic light red, crosswalk light walk
  S3      // Traffic light red, crosswalk light flashing
};
State currentState = State::S0; // Starting state

// State outputs
void stateOutput(State s) {
  // Start with all LEDs off
  digitalWrite(green, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(red, LOW);
  digitalWrite(crosswalk, LOW);

  // Turn on LEDs for each state
  switch (state) {
    case State::S0: digitalWrite(green, HIGH); break; // Green, Stop

    case State::S1: digitalWrite(yellow, HIGH); break; // Yellow, Stop

    case State::S2: digitalWrite(red, HIGH); digitalWrite(crosswalk, HIGH); break; // Red, Walk
    
    case State::S3: digitalWrite(red, HIGH); break; // Red, Flashing(handled in state transition)
  }
}


void setup() {
  Serial.begin(9600);

  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(crosswalk, OUTPUT);
  pinMode(button, INPUT);

  startTime = millis();
  stateOutput(currentState);
}

void loop() {
  // Read input signals
  unsigned long currentTime = millis();
  buttonState = digitalRead(button);
  
  // State transitions
  switch (currentState) {
      case State::S0: 
        if ((currentTime - startTime >= timerLongInterval) && (buttonState == HIGH)) { // Long time interval expires and crosswalk button pressed
          currentState = State::S1;
          startTime = currentTime; // Reset timer
          stateOutput(currentState);
        }
        break;

      case State::S1:
        if (currentTime - startTime >= timerShortInterval) { // Short time interval expires
          currentState = State::S2;
          startTime = currentTime; 
          stateOutput(currentState);
        }
        break;

      case State::S2:
        if (currentTime - startTime >= timerLongInterval) { // Long time interval expires or crosswalk button not pressed
          currentState = State::S3;
          startTime = currentTime; 
          stateOutput(currentState);
        }
        break;

      case State::S3:
        unsigned long currentFlashTime = millis();

        if (currentTime - startTime >= timerShortInterval) { // Short time interval expires
          currentState = State::S0;
          startTime = currentTime; 
          stateOutput(currentState);

        } else if (currentFlashTime - startFlashTime >= flashInterval) { // Flash every interval amount
          startFlashTime = currentFlashTime;
          flashState = !flashState;
          digitalWrite(crosswalk, flashState ? HIGH : LOW);
        }
        break;
    }
}