const int interruptPin = 3; // Pin used for the interrupt (must be 2 or 3 on the Uno)
const int rowPins[3] = {10, 9, 8}; // Pins for the 3 rows
const int colPins[3] = {13, 12, 11}; // Pins for the 3 columns
volatile bool interruptTriggered = false; // Interrupt flag

int buttonStates[3][3] = { // 3x3 matrix to store button states, initialized to 0
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};

void handleInterrupt() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // Debounce method - Assuming a typical 50 ms debounce delay
  if (interruptTime - lastInterruptTime > 50) {
    interruptTriggered = true;
    lastInterruptTime = interruptTime;
  }
}

void setup() {
  pinMode(interruptPin, INPUT_PULLUP); // Set interruptPin as INPUT_PULLUP

  // Configure all row and column pins as INPUT_PULLUP
  for (int i = 0; i < 3; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
    pinMode(colPins[i], INPUT_PULLUP);
  }

  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING); // Trigger on button press (falling edge)
  Serial.begin(9600); // Initialize serial communication
}

void loop() {
  if (interruptTriggered) {
    interruptTriggered = false; // Reset the interrupt flag

    int pressedRow = -1;
    int pressedCol = -1;

    // First loop: Check all rows
    for (int row = 0; row < 3; row++) {
      if (digitalRead(rowPins[row]) == LOW) { // Detect a LOW signal
        pressedRow = row; // Record the active row
        break; // Exit as soon as a row is detected
      }
    }

    // Second loop: Check all columns
    for (int col = 0; col < 3; col++) {
      if (digitalRead(colPins[col]) == LOW) { // Detect a LOW signal
        pressedCol = col; // Record the active column
        break; // Exit as soon as a column is detected
      }
    }

    // Record the button press if valid and if it's not already recorded
    if (pressedRow != -1 && pressedCol != -1) {
      if (buttonStates[pressedRow][pressedCol] == 0) {
        buttonStates[pressedRow][pressedCol] = 1; // Mark the button as pressed
        Serial.print("Button pressed at Row: ");
        Serial.print(pressedRow);
        Serial.print(", Column: ");
        Serial.println(pressedCol);

        // Print the updated array as a grid
        printButtonStates();
      } else {
        Serial.print("Button at Row: ");
        Serial.print(pressedRow);
        Serial.print(", Column: ");
        Serial.println(pressedCol);
        Serial.println("already pressed. Ignoring...");
      }
    }

    delay(200); // Debounce wait time
  }

  delay(100); // Add a small delay before checking for the next interrupt
}

// Function to print the buttonStates array as a grid
void printButtonStates() {
  Serial.println("Button States:");
  for (int i = 0; i < 3; i++) { // Loop through each row
    for (int j = 0; j < 3; j++) { // Loop through each column
      Serial.print(buttonStates[i][j]);
      Serial.print(" "); // Add space between columns
    }
    Serial.println(); // Newline after each row
  }
  Serial.println(); // Extra newline for better readability
}
