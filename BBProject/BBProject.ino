//BB Code

// Default number of bbs to be loaded
#define DEF_BBS 100

// Rate of bbs loaded per second
#define DEF_BBSPERSEC 11

// Pin definitions
#define LEFT_BUTTON 11
#define START_BUTTON 8
#define RIGHT_BUTTON 12
#define MOTOR 10

// LED display libraries
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_SSD1306.h>

#include "TimerOne.h"

// LED display setup
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
  // Initialize the display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  
  // Pin setup
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(MOTOR, OUTPUT);

  // 1 second period, 75% duty cycle
  Timer1.initialize(10000);
  Timer1.pwm(9, 128);
}


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
  display.println("BB Feeder");

  String statusString;

  if (start) {
    statusString = "Feeding";
  } else {
    statusString = "Ready to feed";
  }

  display.println(statusString);
  display.setTextSize(2);
  display.print("    ");
  display.println(bbs); 

  display.display();
}


/*  *   *   *   *  Motor actions  *   *   *   *  */
// Fires the specified number of bbs
void fire() {
  Timer1.attachInterrupt(toggleMotor);
  
  // Same current time in millis for tracking time passed
  unsigned long currentTime = millis();
  unsigned long previousTime = currentTime;

  // Calculate time taken for one motor rotation (in millis).
  const long interval = 1000 / bbsPerSecond;

  // Hold on to number of bbs initially loaded
  int startBbs = bbs;

  // Start the motor rotation.
  //digitalWrite(MOTOR, HIGH);
//  TCCR1A = 0b01010011;
//  TCCR1B = 0b00011101;
//  OCR1A = 125000;
//  OCR1B = 125000;
//  DDRB |= 0b00000010;

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
  //digitalWrite(MOTOR, LOW);
  //  TCCR1A = 0b00000000;
  //  TCCR1B = 0b00000000;
  //  OCR1A = 0;
  //  OCR1B = 0;
  start = false;
  bbs = startBbs;
  Timer1.detachInterrupt();
  digitalWrite(MOTOR, LOW);
}

void toggleMotor() {
  digitalWrite(MOTOR, digitalRead(MOTOR) ^ 1);
}

void loop() {
  if (start){
    // Start firing the gun
    fire();
  } else {
    // Allow manipulation of start parameters:
    //   - Number to be loaded
    bbs += 5 * buttonTapped(RIGHT_BUTTON);
    bbs -= 5 * buttonTapped(LEFT_BUTTON);

    // Specify allowed range for number of bbs
    if (bbs < 1) { bbs = 0; }
  }

  // Toggle start status when start button is pressed
  if (buttonTapped(START_BUTTON)) {
    start = !start;
  }

  // Update the display with current status
  updateDisplay();
}


