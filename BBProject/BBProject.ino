//BB Code

// LED display libraries
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_SSD1306.h>

// Default number of bbs to be loaded
#define DEF_BBS 100

// Default number of bbs to load per second
#define DEF_BBSPERSEC 25

// Pin definitions
#define START_BUTTON 11
#define LEFT_BUTTON 10
#define RIGHT_BUTTON 12
#define MOTOR 9

// LED display setup
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Struct to hold state of a button
typedef struct {
  int pin;
  byte pressed;
} Button;

// Array of the states of each button
Button buttons[] = {{START_BUTTON, 0}, {LEFT_BUTTON, 0}, {RIGHT_BUTTON, 0}};

// Whether or not to start execution
bool start = false;

// The rate at which to load bbs (bbs/second)
// Initialized to default value.
long bbsPerSecond = DEF_BBSPERSEC;

// Number of BBs to be loaded
// Set to defined default
int bbs = DEF_BBS;

void setup() {
  // Initialize the display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  
  // Pin setup
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

/*  *   *   *   *  LED Display Methods  *   *   *   *  */
// Updates the display with current status
void updateDisplay() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("BB Destroyer");

  String statusString;

  if (start) {
    statusString = "Firing!";
  } else {
    statusString = "Ready to fire";
  }

  display.println(statusString);
  display.setTextSize(2);
  display.print("    ");
  display.println(bbs); 

  display.display();
}

// Fires the specified number of bbs
void fire() {
  // Same current time in millis for tracking time passed
  unsigned long currentTime = millis();
  unsigned long previousTime = currentTime;

  // Calculate time taken for one motor rotation (in millis).
  const long interval = 1000 / bbsPerSecond;

  // Hold on to number of bbs initially loaded
  int startBbs = bbs;

  // Start the motor rotation.
  // Will become analogWrite once we have the speed parameter
  digitalWrite(MOTOR, HIGH);
  
  // Allow motor to spin the number of times needed
  for (int i = 0; i < startBbs; i++) {
    if (!start) { break; }

    // Wait for motor to complete one full rotation
    while (currentTime - previousTime < interval) {
      // Allow user to stop firing by pressing the start button again.
      // The current rotation is allowed to finish first.
      if (buttonTapped(START_BUTTON)) { start = false; }

      // Check the time
      currentTime = millis();
    }

    // Update the number of bbs loaded and update display
    bbs--;
    updateDisplay();

    // Save the last checked time after each rotation
    previousTime = currentTime;
  }

  // Halt execution once we're finished
  digitalWrite(MOTOR, LOW);
  start = false;
  bbs = startBbs;
}



void loop() {
  if (start){
    // Start firing the gun
    fire();
  } else {
    // Allow manipulation of start parameters:
    //   - Number to be loaded
    bbs += buttonTapped(RIGHT_BUTTON);
    bbs -= buttonTapped(LEFT_BUTTON);

    // Specify allowed range for number of bbs
    if (bbs < 1) { bbs = 1; }
  }

  // Toggle start status when start button is pressed
  if (buttonTapped(START_BUTTON)) {
    start = !start;
  }

  updateDisplay();
}


