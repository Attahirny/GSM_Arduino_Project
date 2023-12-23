// Required Libraries
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// Global Variables
const int rs = 9, en = 8, d4 = 5, d5 = 4, d6 = 3, d7 = 2; // LCD pins
const int buzzer = 10; // Buzzer pin
const int lcdLights = 11; // LED pin for the LCD display
const int selectButton = A2;
bool blinkState = 0;

unsigned long previousMillisLcdLED = 0;
unsigned long previousMillisBuzzer = 0;
unsigned long previousMillisStart = 0;
int sysReady = 0;
int start = 0;

// Initiating instances of libraries
SoftwareSerial gsmSerial(7, 6); // RX, TX pins for the GSM Module
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // LCD setup

bool sendATCommand(const char* command, unsigned long timeout) {
  // Send the command
  gsmSerial.println(command);

  // Record the start time
  unsigned long startTime = millis();

  // Read the response
  while (millis() - startTime < timeout) {
    if (gsmSerial.available()) {
      String response = gsmSerial.readStringUntil('\n');
      if (response.indexOf("OK") != -1) {
        return true; // Command was successful
      }
    }
  }

  return false; // Command failed or timed out
}

// Arduino built-in function that runs once
void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(lcdLights, OUTPUT);
  pinMode(selectButton, INPUT_PULLUP);

  // initialize serial Monitor
  Serial.begin(9600);

  // initialize LCD
  lcd.begin(16, 2); // Initiating the LCD with fullscreen
  digitalWrite(lcdLights, HIGH);
  delay(500);
  
  // Print project messages
  projectMessages();
  
  // initialize GSM
  gsmSerial.begin(9600);
  delay(1000);

  // Add more initialization for GPRS and downloading configuration data here

  while (!sysReady) {
    //  Check if the system is ready
    lcd.clear();
    lcd.setCursor(0, 0);
    if (testSystem()) {
      sysReady = 1;
      lcd.print(" System Ready! ");
      lcd.clear();
      lcd.print("Press start to ");
      lcd.setCursor(0, 1);
      lcd.print("    begin!");

      // Signal finished initiation
      tone(buzzer, 1500);
      delay(250);
      noTone(buzzer);
      delay(250);
      tone(buzzer, 800);
      delay(1000);
      noTone(buzzer);
    } else {
      lcd.print("System not Ready");
      delay(50);
      lcd.setCursor(0, 1);
      lcd.print("      !!!");
      delay(2000);
    }
  }
}

// Arduino built-in function that runs repeatedly
void loop() {
  // Check for incoming SMS
  while (!start) {
    start = !digitalRead(selectButton);

    // Make the cursor blink each second
    if (millis() - previousMillisStart > 1000) {
      if (blinkState) {
        lcd.blink();
        blinkState = false;
      } else {
        lcd.noBlink();
        blinkState = true;
      }
      previousMillisStart = millis(); // reset to current time
    }
  }

  if (gsmSerial.available()) {
    String sms = gsmSerial.readString();
    lcd.clear();
    // Process the incoming SMS and update the database
    processSMS(sms);

    // Give a ringing sound for a new vote
    tone(buzzer, 1500);
    delay(500);
    noTone(buzzer);
  }

  // Add more functionality for continuous feedback and other operations here
}

bool testSystem() {
  // Check GSM for communication
  if (sendATCommand("AT", 10000)) {
    broadcast("Connection Established!");
  } else {
    delay(1000);
    if (sendATCommand("AT", 10000)) {
      broadcast("Connection Established!");
    } else {
      broadcast("Module not Responding!");
    return false;
    }
  }

  // Set SMS mode to text mode
  if (sendATCommand("AT+CMGF=1", 10000)) {
    broadcast("SMS mode set to Text");
  } else {
    delay(1000);
    if (sendATCommand("AT+CMGF=1", 10000)) {
      broadcast("SMS mode set to Text");
    } else {
    broadcast("Failed to set SMS mode.");
    return false;
    }
  }

  // Check if the GSM module is ready to receive SMS
  if (sendATCommand("AT+CNMI?", 10000)) {
    broadcast("Ready to receive SMS.");
  } else {
    delay(1000);
    if (sendATCommand("AT+CNMI?", 10000)) {
      broadcast("Ready to receive SMS.");
    } else {
      //    broadcast("GSM module is not ready to receive SMS.");
    return false;
    }

  }
  return true; // default
}

void processSMS(String sms) {
  // Parse the incoming SMS and update the database accordingly
  // Add your logic here based on the content of the SMS
  broadcast(sms);
}

// Virtual UART communication channel for SIM800L
void updateSerial() {
  while (Serial.available()) {
    gsmSerial.write(Serial.read()); // Forward what Serial received to Software Serial Port
  }
  while (gsmSerial.available()) {
    Serial.write(gsmSerial.read()); // Forward what Software Serial received to Serial Port
  }
}

// Project Messages by the team
void projectMessages() {
  scrollText("   GSM OR SMS BASED INTELLIGENT VOTING SYSTEM", 300);
  lcd.setCursor(7, 1);
  lcd.print("BY");
  delay(1000);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("GROUP 5");
  lcd.setCursor(0, 1);
  lcd.print("----------------");
  delay(1000);
  // add for a web interface if possible
}

// Sends a broadcast message to all display channels
void broadcast(String message) {
  Serial.println(message);
  scrollText(message, 350);
  // add for a web interface
}

// Scrolls text that is more than 16 characters
void scrollText(const String &text, int delayTime) {
  lcd.clear();
  
  // Add an initial delay before starting the loop
  delay(delayTime * 2);

  for (int i = 0; i <= text.length() - 16; ++i) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(text.substring(i, i + 16));
    delay(delayTime);
  }
}
