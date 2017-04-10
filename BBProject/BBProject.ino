//BB Code

// Default number of bbs to be loaded
#define DEF_BBS 100

// Pin definitions
#define START_BUTTON 11
#define LEFT_BUTTON 10
#define RIGHT_BUTTON 12
#define MOTOR 9

// Struct to hold state of a button
typedef struct {
  int pin;
  byte pressed;
} Button;

// Array of the states of each button
Button buttons[] = {{START_BUTTON, 0}, {LEFT_BUTTON, 0}, {RIGHT_BUTTON, 0}};

// Whether or not to start execution
bool start = false;

// Number of BBs to be loaded
// Set to defined default
int bbs = DEF_BBS;

void setup() {
  Serial.begin(9600);
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(MOTOR, OUTPUT);
}

/*  *   *   *   * Button Actions  *   *   *   *  */
// Gets the last saved state of a button
int getButtonState(int pin) {
  // Search for the button
  for (int i = 0; i < sizeof(buttons); i++) {
    Button currentButton = buttons[i];

    if (currentButton.pin == pin) {
      // Found the button, return its state
      return currentButton.pressed;
    }
  }
}

// Checks if the button attached to a given pin is compressed
// The button's state is saved for later
int buttonPressed(int pin) {
  int pressed = !digitalRead(pin);

  // Search for the button
  for (int i = 0; i < sizeof(buttons); i++) {
    Button currentButton = buttons[i];

    if (currentButton.pin == pin) {
      // Found the button, set its new state and return
      buttons[i].pressed = pressed;
      return pressed;
    }
  }
}

// Checks if the button attached to a given pin was tapped (pressed then released)
int buttonTapped(int pin) {
  int wasSwPressed = getButtonState(pin);
  int isSwPressed = buttonPressed(pin);
  int isSwJustReleased = (!isSwPressed && wasSwPressed);

  return isSwJustReleased;
}

// Loads the specified number of bbs
// TODO: Take into account the motor speed that was set for timing
void fire(int bbs) {
  // For tracking time passed
  unsigned long currentTime = millis();
  unsigned long previousTime = currentTime;

  // TODO: Calculate actual time taken for rotation. I need motor specs for this.
  const long interval = 200;

  // Start the motor rotation.
  // Will become analogWrite once we have the speed parameter
  digitalWrite(MOTOR, HIGH);
  
  // Allow motor to spin the number of times needed.
  for (int i = 0; i < bbs; i++) {
    if (!start) { break; }

    // Wait for motor to complete one full rotation
    while (currentTime - previousTime < interval) {
      // Allow user to stop firing by pressing the start button again.
      // The last rotation is allowed to finish first.
      if (buttonTapped(START_BUTTON)) { start = false; }

      // Check the time
      currentTime = millis();
    }

    // Save the last checked time after each rotation
    previousTime = currentTime;
  }

  // Halt execution once we're finished
  digitalWrite(MOTOR, LOW);
  start = false;
}



void loop() {
  if (start){
    // Start firing the gun
    fire(bbs);
  } else {
    // Allow manipulation of start parameters:
    //   - Number to be loaded
    bbs += buttonTapped(RIGHT_BUTTON);
    bbs -= buttonTapped(LEFT_BUTTON);
  }

  // Toggle start status when start button is pressed
  if (buttonTapped(START_BUTTON)) {
    start = !start;
  }

  Serial.println(bbs);
}


