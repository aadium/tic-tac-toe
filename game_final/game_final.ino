#include <util/delay.h>

// Timer Interrupt Setup for Timer1
const int moveInterrupt = 3; // Pin used for the move interrupt
const int newGameInterrupt = 2; // Pin used for the new game/reset interrupt
const int rowPins[3] = {10, 9, 8}; // Pins for the 3 rows
const int colPins[3] = {13, 12, 11}; // Pins for the 3 columns

// Variables to track time
volatile unsigned long previousMoveTime = 0; // Tracks the time of the last move
volatile unsigned long currentMoveTime = 0; // Tracks the current move time
volatile unsigned long totalGameTime = 0; // Tracks the total game time

// 3x3 matrix to store button states
int buttonStates[3][3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};

bool gameWon = false; // Flag to indicate if the game is over
volatile bool moveInterruptTriggered = false; // Move interrupt flag
volatile bool newGameInterruptTriggered = false; // New game interrupt flag

// Custom delay function
void delay_ms(unsigned int ms) {
  while (ms--) {
    _delay_ms(1);
  }
}

// Timer function to increment the total game time
void incrementGameTime() {
  totalGameTime += 100;
}

// Function to detect a winner (1 for user, 2 for computer)
int checkWinner(int (&grid)[3][3]) {
  for (int i = 0; i < 3; i++) {
    // Check rows
    if (grid[i][0] != 0 && grid[i][0] == grid[i][1] && grid[i][1] == grid[i][2]) {
      return grid[i][0];
    }
    // Check columns
    if (grid[0][i] != 0 && grid[0][i] == grid[1][i] && grid[1][i] == grid[2][i]) {
      return grid[0][i];
    }
  }

  // Check diagonals
  if (grid[0][0] != 0 && grid[0][0] == grid[1][1] && grid[1][1] == grid[2][2]) {
    return grid[0][0];
  }
  if (grid[0][2] != 0 && grid[0][2] == grid[1][1] && grid[1][1] == grid[2][0]) {
    return grid[0][2];
  }

  // Check for draw
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (grid[i][j] == 0) {
        return 0; // Grid is not full, game is still in progress
      }
    }
  }

  return -1; // Grid is full and no winner, it's a draw
}

// Function to predict the computer's best move
void predictBestMove(int (&grid)[3][3], int &bestRow, int &bestCol) {
    int positions[9][2]; // Array that stores all possible positions
    int index = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            positions[index][0] = i;
            positions[index][1] = j;
            index++;
        }
    }

    int seed = analogRead(A1) % 10; // The seed is the analog value from pin A1
    Serial.print("Random seed: ");
    Serial.println(seed);
    for (int i = 8; i > 0; i--) {
        int j = seed % (i + 1); // Calculate the random index using the seed
        // Swap positions[i] and positions[j]
        int temp0 = positions[i][0];
        int temp1 = positions[i][1];
        positions[i][0] = positions[j][0];
        positions[i][1] = positions[j][1];
        positions[j][0] = temp0;
        positions[j][1] = temp1;
    }

    // Find the first empty cell in the shuffled positions
    for (int k = 0; k < 9; k++) {
        int i = positions[k][0];
        int j = positions[k][1];
        if (grid[i][j] == 0) {
            bestRow = i;
            bestCol = j;
            return;
        }
    }

    // If no empty cell is found, it is a draw
    bestRow = -1;
    bestCol = -1;
}

// Function to print the winning player
void handleVictory(int winner, unsigned long totalGameTime, bool &gameWon){
  gameWon = true;
  if(winner == 1) {
    Serial.println("User Wins!");
    digitalWrite(5, HIGH);
  } else if(winner == 2) {
    Serial.println("Computer Wins!");
    digitalWrite(7, HIGH);
  } else if(winner == -1) {
    Serial.println("No one wins");
    digitalWrite(5, HIGH);
    digitalWrite(7, HIGH);
  }
  Serial.print("Total game time: ");
  Serial.print(totalGameTime);
  Serial.println(" ms");
}

// Handle main interrupt and game logic
void handleMoveInterrupt() {
  if (gameWon) return;
  digitalWrite(4, LOW);
  int pressedRow = -1;
  int pressedCol = -1;

  // Check all rows
  for (int row = 0; row < 3; row++) {
    if (digitalRead(rowPins[row]) == LOW) {
      pressedRow = row;
      break;
    }
  }

  // Check all columns
  for (int col = 0; col < 3; col++) {
    if (digitalRead(colPins[col]) == LOW) {
      pressedCol = col;
      break;
    }
  }

  // If a valid button press has been detected (row and column identified)
  if (pressedRow != -1 && pressedCol != -1) {
    // Check if the cell has not already been pressed
    if (buttonStates[pressedRow][pressedCol] == 0) {
        // Mark the user's move in the grid
        buttonStates[pressedRow][pressedCol] = 1;
        // Print the user's move to the Serial Monitor
        Serial.print("User's move at Row: ");
        Serial.print(pressedRow);
        Serial.print(", Column: ");
        Serial.println(pressedCol);

        // Record the time of the current move
        currentMoveTime = totalGameTime;
        // Calculate and display the time taken for this move, if it's not the first move
        if (previousMoveTime != 0) { // Skip the calculation for the very first move
            unsigned long timeTakenToMakeAMove = currentMoveTime - previousMoveTime;
            Serial.print("Time taken to make this move: ");
            Serial.print(timeTakenToMakeAMove);
            Serial.println(" ms");
        }
        // Update the previous move time for future calculations
        previousMoveTime = currentMoveTime;

        // Display the updated grid state in the Serial Monitor
        printButtonStates();

        // Check if the user's move resulted in a win
        int winner = checkWinner(buttonStates);
        if (winner != 0) {
            // If there's a winner, handle victory and display the winner and game stats
            handleVictory(winner, totalGameTime, gameWon);
            return; // End the function to stop further processing
        }

        // Let the computer make its move
        int bestRow = -1, bestCol = -1;
        predictBestMove(buttonStates, bestRow, bestCol); // Predict the best move for the computer
        if (bestRow != -1 && bestCol != -1) {
            // Mark the computer's move in the grid
            buttonStates[bestRow][bestCol] = 2;
            // Print the computer's move to the Serial Monitor
            Serial.print("Computer's move at Row: ");
            Serial.print(bestRow);
            Serial.print(", Column: ");
            Serial.println(bestCol);

            printButtonStates();

            // Check if the computer's move resulted in a win
            winner = checkWinner(buttonStates);
            if (winner != 0) {
                // If there's a winner, handle victory and display the winner and game stats
                handleVictory(winner, totalGameTime, gameWon);
                return; // End the function to stop further processing
            }
        }
    } else {
        // If the selected cell has already been pressed, notify the user
        Serial.print("Button at Row: ");
        Serial.print(pressedRow);
        Serial.print(", Column: ");
        Serial.println(pressedCol);
        Serial.println("already pressed. Ignoring...");
    }
  }
}

// Handle new game interrupt
void handleNewGameInterrupt() {
  gameWon = false;
  totalGameTime = 0;
  previousMoveTime = 0;

  // Set all pins to low, and pin 4 to high
  digitalWrite(4, HIGH);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(6, LOW);
  digitalWrite(A0, LOW);
  Serial.println("New game started!");

  // Clear the buttonStates grid
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      buttonStates[i][j] = 0;
    }
  }
  printButtonStates();
}

// Function to print the buttonStates array as a grid
void printButtonStates() {
  Serial.println("Current Grid:");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (buttonStates[i][j] == 0) {
        Serial.print(". ");
      } else if (buttonStates[i][j] == 1) {
        Serial.print("X ");
      } else {
        Serial.print("O ");
      }
    }
    Serial.println();
  }
  Serial.println();
}

void setup() {
  pinMode(moveInterrupt, INPUT_PULLUP);
  pinMode(newGameInterrupt, INPUT_PULLUP);

  // Initialize output pins
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);

  // Set all pins to low, and pin 4 to high
  digitalWrite(4, HIGH);
  digitalWrite(6, LOW);
  digitalWrite(A0, LOW);
  digitalWrite(A5, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A3, LOW);
  digitalWrite(A2, LOW);

  // Initialize input pins
  for (int i = 0; i < 3; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
    pinMode(colPins[i], INPUT_PULLUP);
  }

  // Attach pins 2 and 3 to their interrupt functions
  attachInterrupt(digitalPinToInterrupt(moveInterrupt), handleMoveInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(newGameInterrupt), handleNewGameInterrupt, FALLING);

  Serial.begin(9600);

  // Timer1 setup for incrementing totalGameTime every 100 milliseconds
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 15624; // Compare match register (16MHz/256/10Hz) (Clock Frequency / Prescaler / Desired Frequency)
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS12); // 256 prescaler
  TIMSK1 |= (1 << OCIE1A); // Enable timer compare interrupt

  interrupts();
}

ISR(TIMER1_COMPA_vect) {
  incrementGameTime();
}

void loop() {

  // Handle move interrupt
  if (moveInterruptTriggered) {
    moveInterruptTriggered = false;
    handleMoveInterrupt();
  }

  // Handle new game interrupt
  if (newGameInterruptTriggered) {
    newGameInterruptTriggered = false;
    handleNewGameInterrupt();
  }

  // Set pin 4 to high to turn off all LEDs
  digitalWrite(4, HIGH);

  // Row 0 Col 0
  if(buttonStates[0][0] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, LOW);
    digitalWrite(A4, HIGH);
    digitalWrite(A3, LOW);
    digitalWrite(A2, LOW);
  } else if (buttonStates[0][0] == 2){
    digitalWrite(4, LOW);
    digitalWrite(A5, HIGH);
    digitalWrite(A4, HIGH);
    digitalWrite(A3, LOW);
    digitalWrite(A2, LOW);
  }  
  delay_ms(1);

  // Row 0 Col 1
  if(buttonStates[0][1] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, LOW);
    digitalWrite(A4, LOW);
    digitalWrite(A3, LOW);
    digitalWrite(A2, HIGH);
  } else if (buttonStates[0][1] == 2){
    digitalWrite(4, LOW);
    digitalWrite(A5, HIGH);
    digitalWrite(A4, LOW);
    digitalWrite(A3, LOW);
    digitalWrite(A2, HIGH);
  }  
  delay_ms(1);

  // Row 0 Col 2
  if(buttonStates[0][2] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, LOW);
    digitalWrite(A4, HIGH);
    digitalWrite(A3, HIGH);
    digitalWrite(A2, HIGH);
  } else if (buttonStates[0][2] == 2){
    digitalWrite(4, LOW);
    digitalWrite(A5, HIGH);
    digitalWrite(A4, HIGH);
    digitalWrite(A3, HIGH);
    digitalWrite(A2, HIGH);
  }  
  delay_ms(1);

  // Row 1 Col 0
  if(buttonStates[1][0] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, HIGH);
    digitalWrite(A4, LOW);
    digitalWrite(A3, LOW);
    digitalWrite(A2, LOW);
  } else if (buttonStates[1][0] == 2){
    digitalWrite(4, LOW);
    digitalWrite(A5, LOW);
    digitalWrite(A4, LOW);
    digitalWrite(A3, HIGH);
    digitalWrite(A2, LOW);
  }  
  delay_ms(1);

  // Row 1 Col 1
  if(buttonStates[1][1] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, HIGH);
    digitalWrite(A4, HIGH);
    digitalWrite(A3, HIGH);
    digitalWrite(A2, LOW);
  } else if (buttonStates[1][1] == 2){
    digitalWrite(4, LOW);
    digitalWrite(A5, LOW);
    digitalWrite(A4, HIGH);
    digitalWrite(A3, LOW);
    digitalWrite(A2, HIGH);
  }  
  delay_ms(1);

  // Row 1 Col 2
  if(buttonStates[1][2] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, HIGH);
    digitalWrite(A4, LOW);
    digitalWrite(A3, HIGH);
    digitalWrite(A2, HIGH);
  } else if (buttonStates[1][2] == 2){
    digitalWrite(A0, HIGH);
    digitalWrite(6, LOW);
  }  
  delay_ms(1);

  // Row 2 Col 0
  if(buttonStates[2][0] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, LOW);
    digitalWrite(A4, LOW);
    digitalWrite(A3, LOW);
    digitalWrite(A2, LOW);
  } else if (buttonStates[2][0] == 2){
    digitalWrite(4, LOW);
    digitalWrite(A5, HIGH);
    digitalWrite(A4, LOW);
    digitalWrite(A3, HIGH);
    digitalWrite(A2, LOW);
  }  
  delay_ms(1);

  // Row 2 Col 1
  if(buttonStates[2][1] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, LOW);
    digitalWrite(A4, HIGH);
    digitalWrite(A3, HIGH);
    digitalWrite(A2, LOW);
  } else if (buttonStates[2][1] == 2){
    digitalWrite(4, LOW);
    digitalWrite(A5, HIGH);
    digitalWrite(A4, HIGH);
    digitalWrite(A3, LOW);
    digitalWrite(A2, HIGH);
  }  
  delay_ms(1);

  // Row 2 Col 2
  if(buttonStates[2][2] == 1){
    digitalWrite(4, LOW);
    digitalWrite(A5, LOW);
    digitalWrite(A4, LOW);
    digitalWrite(A3, HIGH);
    digitalWrite(A2, HIGH);
  } else if (buttonStates[2][2] == 2){
    digitalWrite(A0, LOW);
    digitalWrite(6, HIGH);
  }  
  delay_ms(1);
}
