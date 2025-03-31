int delayInt = 1; // Adjust delay for better visibility

void setup() {
  Serial.begin(9600);
  // Configure all pins as OUTPUT
  pinMode(4, OUTPUT); // Enable control for LEDs
  pinMode(7, OUTPUT); // Always HIGH
  pinMode(A0, OUTPUT); // LED control
  pinMode(5, OUTPUT); // Unused in original code, but defined for safety
  pinMode(6, OUTPUT); // LED control
  pinMode(A2, OUTPUT); // Decoder output
  pinMode(A3, OUTPUT); // Decoder output
  pinMode(A4, OUTPUT); // Decoder output
  pinMode(A5, OUTPUT); // Decoder output
}

void loop() {
  digitalWrite(4, LOW); // Enable LEDs
  digitalWrite(7, HIGH); // Always HIGH
  digitalWrite(5, HIGH); // Set pin 5 HIGH (clarify its role if needed)

  // Iterate through all 16 combinations (4-bit binary)
  for (int combination = 0; combination < 16; combination++) {
    // Update all 4 decoder pins (A2 to A5) in the correct bit order
    digitalWrite(A2, (combination >> 3) & 1); // 4th bit
    digitalWrite(A3, (combination >> 2) & 1); // 3rd bit
    digitalWrite(A4, (combination >> 1) & 1); // 2nd bit
    digitalWrite(A5, (combination >> 0) & 1); // 1st bit

    // Turn off direct LEDs during decoder output phase
    digitalWrite(A0, LOW); 
    digitalWrite(6, LOW);

    delay(delayInt); // Wait before switching to the next combination
  }

  digitalWrite(4, HIGH); // Disable decoder temporarily

  // Control the LEDs directly connected to A0 and 6
  for (int i = 0; i < 2; i++) {
    if (i == 0) {
      digitalWrite(A0, HIGH); // Turn on A0
      digitalWrite(6, LOW);   // Ensure 6 is off
    } else {
      digitalWrite(A0, LOW);  // Turn on 6
      digitalWrite(6, HIGH);
    }

    // Disable decoder outputs during direct LED phase
    digitalWrite(A2, LOW); 
    digitalWrite(A3, LOW);
    digitalWrite(A4, LOW);
    digitalWrite(A5, LOW);

    delay(delayInt); // Delay for visibility of the LEDs
  }

  // Turn off direct LEDs
  digitalWrite(A0, LOW);
  digitalWrite(6, LOW);
}