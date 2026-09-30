#include <string.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>  
#include <Wire.h>
#include <EEPROM.h>

#define CPS_SIZE 8

#define clockPin A0  
#define loadPin A1  
#define dataPin A2

const int ledred = 12;  
const int ledgreen = 10;  
const int piezoPin = 11;  
const int piezoPin2 = 13;  
const int button = A3;  
const int size = 8;  
String triggeredSensors;  
String tri;  
const unsigned long t_sirin_delay = 20000; // 10 seconds delay time
const unsigned long t_bp_slow = 500; // Slow beep every 0.2 seconds
const unsigned long t_bp_fast = 200; // Fast beep every 0.2 seconds
const unsigned long t_go = 10000; // Transition to fast beep in the last 10 seconds
const unsigned long t_sirene_restart = 5000;  
int energo = 0;  
int alarmState = 0;  
bool alarmActive = false; // When alarm is armed, it is always TRUE
bool preAlarmActive = false; // When alarm is armed during exit countdown
bool armingWarningActive = false;  
bool fastBeep = false;  
unsigned long preAlarmStartTime = 0;  
unsigned long armingStartTime = 0;  
unsigned long lastBeepTime = 0;  
unsigned long sirenStartTime = 0;  
const unsigned long sirenDuration = 30000;
bool showing_by_cps = false;
bool showing_vltd_cps = false;

byte Burgl_v[size] = {0};  
bool armingB = true;  
byte pli = 0;  
bool Bypass[size];  
int flag = -1;  

const byte ROWS = 4;

const byte COLS = 4;

int buzzer_hz = 500;
int buzzer_del = 100;

char keys[ROWS][COLS] = {

  { '1', '2', '3', 'A' },

  { '4', '5', '6', 'B' },

  { '7', '8', '9', 'C' },

  { '*', '0', '#', 'D' }

};

byte rowPins[ROWS] = { 9, 8, 7, 6 };

byte colPins[COLS] = { 5, 4, 3, 2 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27,16,2); 

bool alarm_armed = false;

bool standby_mode = true;

bool prog_mode = false;

bool validInput = false;

bool prog_user_code = false;

bool prog_prog_code = false;

bool prog_bypass_cps = false;

bool prog_key_timeout = false;

bool prog_burgl_count_down_cps_v = false;

bool prog_fast_arming = false;

bool prog_t_arming_delay = false;

bool prog_t_disarming_delay = false;

bool Bypassed_prog_v[8];

bool Burgl_Count_Down_CPs_v[8];

bool Bypassed_All_Temp_v[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

bool in_menu = false;

bool arming_prep = false;

bool Bypass_ALL = false;

int t_arming_delay = 30;

int t_disarming_delay = 30;

int keys_pressed_bound = 6;

unsigned long start_time_key;

unsigned long start_time_prog; 

unsigned long start_time_arming;

int countKeys = 0;

int menu_page = 1;

int remaining_seconds;

bool showing_menu = false;

unsigned long key_timeout = 15;

unsigned long prog_timeout = 15;

unsigned long prog_timeout_warning = 10;

bool prog_about_to_timeout = false;

bool arming_about_to_timeout = false;

bool in_menu_function = false;

int dsrm_tries = 3;

char user_code[5];
char prog_code[5];

char keys_pressed[17];

char fast_arming[2];

void setup() {

  keypad.setHoldTime(1000);

  pinMode(piezoPin, OUTPUT);
  pinMode(piezoPin2, OUTPUT);
  pinMode(ledred, OUTPUT);
  pinMode(ledgreen, OUTPUT);

  digitalWrite(piezoPin, HIGH);
  digitalWrite(piezoPin2, HIGH);
  digitalWrite(ledred, LOW);
  digitalWrite(ledgreen, HIGH);

  digitalWrite(clockPin, LOW);  
  pinMode(clockPin, OUTPUT);  
  digitalWrite(clockPin, HIGH);  
  pinMode(loadPin, OUTPUT); // HIGH Parallel Load, or RIGHT SHIFT  
  pinMode(dataPin, INPUT); 

  lcd.init();  
  lcd.backlight();  
  lcd.begin(16, 2);

  Serial.begin(9600);

  start_time_key = 0;
  start_time_prog = millis();

  if(EEPROM.read(301) != 42) {
    lcd.print("Initializing");
    lcd.setCursor(0, 1);
    lcd.print("memory...");
    fill_memory();
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
  }
 
  read_memory();

  print_rdy();
}

void fill_memory() {
  bool temp_bool[8] = {false, false, false, false, false, false, false, false};

  EEPROM.put(0, temp_bool);
  EEPROM.put(8, temp_bool);

  EEPROM.put(16, (char[5]){"0000"});
  EEPROM.put(21, (char[5]){"4444"});
  EEPROM.put(26, (char[2]){"0"});

  EEPROM.put(28, 30);
  EEPROM.put(30, 30);

  EEPROM.put(301, 42);
}

void read_memory() {
  EEPROM.get(0, Bypassed_prog_v);
  EEPROM.get(8, Burgl_Count_Down_CPs_v);

  EEPROM.get(16, user_code);
  EEPROM.get(21, prog_code);
  EEPROM.get(26, fast_arming);

  EEPROM.get(28, t_arming_delay);
  EEPROM.get(30, t_disarming_delay);
}

void BURGL_Load(byte * Burgl_v) {  
  byte parallelData;  
  byte CP1, CP2, CP3, CP4, CP5, CP6, CP7, CP8;  
  digitalWrite(loadPin, LOW); // Enable Parallel Load  
  digitalWrite(clockPin, LOW);  
  digitalWrite(clockPin, HIGH); // Loading Magnetic Contacts  
  digitalWrite(clockPin, LOW);  
  digitalWrite(loadPin, HIGH); // Enable Right Shift  
  for (int i = 0; i < 8; i++) {  
    byte shiftedBit = digitalRead(dataPin);  
    switch (i) {  
      case 0:  
        CP8 = shiftedBit;  
      break;  
      case 1:  
        CP7 = shiftedBit;  
      break;  
      case 2:  
        CP6 = shiftedBit;  
      break;  
      case 3:  
        CP5 = shiftedBit;  
      break;  
      case 4:  
        CP4 = shiftedBit;  
      break;  
      case 5:  
        CP3 = shiftedBit;  
      break;  
      case 6:  
        CP2 = shiftedBit;  
      break;  
      case 7:  
        CP1 = shiftedBit;  
      break;  
    } 
    digitalWrite(clockPin, HIGH);  
    digitalWrite(clockPin, LOW);  
  }

  Burgl_v[0] = CP1;  
  Burgl_v[1] = CP2;  
  Burgl_v[2] = CP3;  
  Burgl_v[3] = CP4;  
  Burgl_v[4] = CP5;  
  Burgl_v[5] = CP6;  
  Burgl_v[6] = CP7;  
  Burgl_v[7] = CP8;  
} 

void startArmingWarning() { // Countdown start for alarm arming  
  armingWarningActive = true;  
  fastBeep = false;  
  Serial.println("Ξεκίνησε προειδοποίηση όπλισης.");  
}

// Helper function
void startPreAlarm() { // Countdown for siren sound when a countdown CP is activated  
  preAlarmActive = true;  
  fastBeep = false;  
  preAlarmStartTime = millis();  
  lastBeepTime = millis();  
  Serial.println("Προειδοποίηση παραβίασης ξεκίνησε.");  
}

// Helper function  
void stopPreAlarm() { // Deactivation of alarm when triggered by a countdown CP  
  preAlarmActive = false;  
  noTone(piezoPin);  
  digitalWrite(piezoPin, HIGH); // Disable buzzer  
  Serial.println("Η προειδοποίηση παραβίασης ακυρώθηκε.");  
}

// Main loop of the system =================================================================================
void loop() {

  BURGL_Load(Burgl_v);               // ==================================== CPs READ ====================================
  if(standby_mode == true && arming_prep == false && prog_mode == false && alarm_armed == false) {
    print_rdy();
  }

  char key = keypad.getKey();

  if (key) {                         // ==================================== KEYPAD READ =================================
    if (!prog_about_to_timeout) {
      if (prog_mode == true)
        start_time_prog = millis();

      if (showing_menu && key != '#' && key != '*' && key != 'B' && key != 'C') {
        lcd.clear();
        lcd.setCursor(0, 0);
        showing_menu = false;
      }

      if(alarm_armed && showing_by_cps == true && key != '#' && key != '*' && key != 'B' && key != 'C') {
        lcd.setCursor(0, 0);
        lcd.print("                ");
        showing_by_cps = false;
      }

      if(alarm_armed && showing_vltd_cps == true && key != '#' && key != '*' && key != 'B' && key != 'C') {
        lcd.setCursor(0, 0);
        lcd.print("                ");
        showing_vltd_cps = false;
      }

      validInput = check_keypad_entry(keys_pressed, key);

      if (in_menu && !in_menu_function && !showing_menu && key == 'C' && countKeys == 0) {
        print_menu(menu_page);
      }
    }
    start_time_key = millis();
  }

  unsigned long current_time = millis();  // ==================================== KEYPAD TIMEOUT ==============================

  if(standby_mode && start_time_key != 0 && countKeys > 0 && current_time - start_time_key >= key_timeout * 1000) {
    standby_key_timeout(keys_pressed);
  }

  if (prog_mode && current_time - start_time_prog >= prog_timeout * 1000) {
    prog_mode_off_timeout(keys_pressed);
    print_rdy();
  }

  if (prog_mode && !prog_about_to_timeout && current_time - start_time_prog >= prog_timeout_warning * 1000) {
    prog_about_to_timeout = true;
    start_warning(keys_pressed);
  }

  if (prog_about_to_timeout) {
    if (key) {
      timeout_canceled_reset(keys_pressed);
      if (in_menu && !in_menu_function) {
        print_menu(menu_page);
      } else if (in_menu_function) {
        enable_fucntion_after_timout_canceled(keys_pressed);
      }
    } else {
      update_lcd_timeout_warning(current_time);
    }
  }           // ==================================== END TIMEOUT KEYPAD ===========================

  if(arming_prep) {
    if(current_time - start_time_arming >= t_arming_delay * 1000)
      t_arming_delay_function();
    else
      update_lcd_arming(current_time);
  }

  if (validInput) {

    if (arming_prep) {
      lcd.setCursor(0, 1);
      lcd.print("Arming in:");
      lcd.setCursor(0, 0);
    }
    if (prog_mode) {

      if (!in_menu_function && keys_pressed[0] == NULL) {
        beep(piezoPin, buzzer_hz, 0, 1);
        if (menu_page == 1)
          menu_page++;
        else
          menu_page = 1;
      }

      in_menu = prog_system(keys_pressed);

      if (in_menu && !in_menu_function) {
        print_menu(menu_page);
      }
    }

    if (strcmp(keys_pressed, prog_code) == 0 && !in_menu && !alarm_armed && !arming_prep) {
      prog_mode_on_off();
    }
    else if (prog_mode == false && ((strncmp(keys_pressed, user_code, 4) == 0 && strlen(keys_pressed) == 4) || (strncmp(keys_pressed, user_code, 4) == 0 && strlen(keys_pressed) == 5 && keys_pressed[4] == 'B') || strcmp(keys_pressed, fast_arming) == 0)) {
      if(strncmp(keys_pressed, user_code, 4) == 0 && keys_pressed[4] == 'B') {
        Bypass_ALL = true;
      }
      if(arming_prep == false && alarm_armed == false) {
        if(arming_check() == true) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Arming poss.");
          delay(2000);
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Arming in:");
          lcd.setCursor(0, 0);
          start_time_arming = millis();
        }
        else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Arming imposs.");
          delay(2000);
          lcd.clear();
          lcd.setCursor(0, 0);
        }
      }
      else if(strcmp(keys_pressed, fast_arming) != 0) {
        arming_disable();
        print_rdy();
      }
    }
    else if(prog_mode == false && alarm_armed == false){
      lcd.setCursor(0, 0);
      lcd.print("Invalid code");
      beep(piezoPin, buzzer_hz, buzzer_del, 3);
      if(arming_prep == true && dsrm_tries > 0) {
        lcd.print(" ( )");
        lcd.setCursor(14, 0);
        lcd.print(--dsrm_tries);
      }
      delay(1000);
      lcd.setCursor(0, 0);
      lcd.print("                ");
      lcd.setCursor(0, 0);
    }
    if(dsrm_tries == 0) {
      t_arming_delay_function();
      alarmActive = true;
      armingB = true;
      dsrm_tries = 3;
    }
    if(standby_mode && arming_prep == false && alarm_armed == false) {
      print_rdy();
    }
    validInput = false;
  }

  // Main block =========================================================================
  if(alarm_armed == true) {
    
    if(armingB == true && pli == 0){ // When alarm is armed, it builds the vector of the ByP CPs and shows them on LCD  
      // Start arming warning  
      for(int i=0;i<size;i++){  
        if(Bypassed_prog_v[i] == 1 || Bypassed_All_Temp_v[i] == 1)  
        {  
          Bypass[i] = 1;  
        }else{  
          Bypass[i] = 0;  
        }  
      }  
      // Display LCD message
      lcd.setCursor(0,0);  
      lcd.print(" ");  
      lcd.setCursor(0,0);  
      lcd.print("User Mode");  
      lcd.setCursor(0,1);  
      lcd.print("System Ready");  
      delay(2000);  
      lcd.clear();  
      startArmingWarning();  
      lcd.setCursor(1, 0);  
      lcd.print("-----armed-----");  
      delay(2000);  
      lcd.setCursor(0, 0);
      lcd.print("                ");
      // End of message
      pli = 1;  
      energo = 1;  
      alarmState = 1;  
    }else if(armingB == false){ // Procedures when alarm is deactivated  
      energo = 0;  
      alarmActive = false;  
      preAlarmActive = false;  
      noTone(piezoPin); // Stop beep  
      digitalWrite(piezoPin, HIGH); // Disable buzzer  
      noTone(piezoPin2); // Stop siren  
      digitalWrite(piezoPin2, HIGH); // Stop siren  
      digitalWrite(ledred, LOW); // Turn off LED  
      digitalWrite(ledgreen, HIGH);  
      // lcd.clear(); // Clear display  
      lcd.setCursor(0, 0);  
      // lcd.print("RDY, VLTD CPs:");  
      lcd.setCursor(0,1);  
      lcd.print(triggeredSensors);  
      Serial.println("Ο συναγερμός απενεργοποιήθηκε.");  
      // delay(2000); // Wait for the message to appear  
      // lcd.clear();  
      pli = 0;  
    } 
    // 
    if(energo == 1 && !preAlarmActive && !alarmActive){  
      digitalWrite(ledgreen, LOW);  
      digitalWrite(ledred,HIGH);  
      lcd.setCursor(0,0);  
      if(showing_by_cps == false && countKeys == 0) {
        lcd.print("ARMD ByP CPs:");  
        // lcd.setCursor(0,1);
        showing_by_cps = true;
      }  
      lcd.setCursor(0,1);
      for(int i=0;i<size;i++){  
        if(Bypass[i] == 1){  
          lcd.print(i+1);  
          lcd.print(" ");  
        }  
      }  
    } 

    BURGL_Load(Burgl_v);


    // Violation check and warning startup when system is armed  
    // Logic for violation and countdown  
    if (energo == 1 && !preAlarmActive && !alarmActive) {  
      triggeredSensors = ""; // String to store activated sensors  
      for (int i = 0; i < size; i++) {  
        if (Bypass[i] == 0 && Burgl_v[i] == 1) {  
          if(flag != -1 && Burgl_Count_Down_CPs_v[i] == 0){  
            noTone(piezoPin);  
            digitalWrite(piezoPin, HIGH); // Disable buzzer  
            preAlarmActive = false;  
            lcd.setCursor(0, 1);
            lcd.print("                "); 
            alarmActive = true;  
            sirenStartTime = millis();  
            Serial.println("Σειρήνα ενεργοποιήθηκε λόγω παραβίασης!");  
          }else if (Burgl_Count_Down_CPs_v[i] == 1) {  
            flag =i;  
            startPreAlarm();  
          }else if(Burgl_Count_Down_CPs_v[i] == 0){  
            preAlarmActive = false; 
            lcd.setCursor(0, 1);
            lcd.print("                ");  
            alarmActive = true;  
            sirenStartTime = millis();  
            Serial.println("Σειρήνα ενεργοποιήθηκε λόγω παραβίασης!");  
          }  
          // Add the sensor number to the string  
          triggeredSensors += String(i + 1) + " ";  
          // Serial.print("triggerd:" + triggeredSensors);
        }  
      }  
      // Display all activated sensors on LCD  
      if (triggeredSensors != "") {  
        // lcd.setCursor(0, 0);  
        if(showing_vltd_cps == false && countKeys == 0) {
          lcd.setCursor(0, 0);
          lcd.print("ARMD VLTD CPs: ");  
          lcd.setCursor(0,1);
          showing_vltd_cps = true;
        }
        //  
        lcd.setCursor(0, 1);
        for(int i=0;i<size;i++){  
          if(Bypass[i] == 1){  
            lcd.print(i+1);  
            lcd.print(" ");  
          }  
        }  
        lcd.setCursor(8, 1);  
        lcd.print(triggeredSensors);  
      }  
    }else {  
      for (int i = 0; i < size; i++) {  
        if (Bypass[i] == 0 && Burgl_v[i] == 1) {  
          if(flag != -1 && Burgl_Count_Down_CPs_v[i] == 0 && i != flag && !alarmActive){  
            noTone(piezoPin);  
            digitalWrite(piezoPin, HIGH); // Disable buzzer  
            preAlarmActive = false;  
            lcd.setCursor(0, 1);
            lcd.print("                "); 
            alarmActive = true;  
            sirenStartTime = millis();  
            Serial.println("Σειρήνα ενεργοποιήθηκε λόγω παραβίασης!");  
          }  
        }  
      }  
      // Auxiliary check  
      for(int i=0;i<size;i++){  
        if(Bypass[i] == 0 && Burgl_v[i] == 1){  
          triggeredSensors = "";  
        }  
      }  
      for(int i=0;i<size;i++){  
        if(Bypass[i] == 0 && Burgl_v[i] == 1){  
          // Add the sensor number to the string  
          triggeredSensors += String(i + 1) + " ";  
        }  
      }  
      // Display all activated sensors on LCD  
      if (triggeredSensors!= "" && energo == 1) {  
        if(showing_vltd_cps == false && countKeys == 0) {
          lcd.setCursor(0, 0);
          lcd.print("ARMD VLTD CPs: ");  
          lcd.setCursor(0,1);
          showing_vltd_cps = true;
        }    
        //  
        lcd.setCursor(0, 1);
        for(int i=0;i<size;i++){  
          if(Bypass[i] == 1){  
            lcd.print(i+1);  
            lcd.print(" ");  
          }  
        }  
        if(!preAlarmActive){  
          lcd.setCursor(8, 1);  
          lcd.print(triggeredSensors);  
        }  
      }  
    } 


    // Violation warning procedure  
    if (preAlarmActive) {  
      unsigned long currentTime = millis();  
      unsigned long elapsedPreAlarmTime = currentTime - preAlarmStartTime;  
      if (elapsedPreAlarmTime >= t_sirin_delay) {  
        // End of delay, activate the siren  
        preAlarmActive = false; 
        alarmActive = true;  
        sirenStartTime = millis();  
        Serial.println("Σειρήνα ενεργοποιήθηκε λόγω παραβίασης!");  
      }  
      else {  
        if (elapsedPreAlarmTime >= (t_sirin_delay - t_go)) {  
          fastBeep = true;  
        }  
        unsigned long beepInterval = fastBeep ? t_bp_fast : t_bp_slow;  
        if (currentTime - lastBeepTime >= beepInterval) {  
          tone(piezoPin, 2000);  
          delay(100);  
          noTone(piezoPin);  
          digitalWrite(piezoPin, HIGH); // Disable buzzer  
          lastBeepTime = currentTime;  
        }  
      }  
      
      unsigned long remainingTime = t_sirin_delay - elapsedPreAlarmTime;  
      // Update the LCD with the countdown  
      if(showing_vltd_cps == false && countKeys == 0) {
        lcd.setCursor(0, 0);
        lcd.print("ARMD VLTD CPs: ");  
        lcd.setCursor(0,1);
        showing_vltd_cps = true;
      } 
      lcd.print(triggeredSensors);  
      lcd.setCursor(8,1); // Move cursor to the second line  
      lcd.print("Time:");  
      lcd.print(remainingTime / 1000); // Display remaining seconds  
      lcd.print("''");  
      
      if(preAlarmActive == false) {
        lcd.setCursor(0, 1);
        lcd.print("                ");
      }
    } 
    // Check remaining conditions 
    // Activate siren if delay time expires  
    if (alarmActive) {  
      if (millis() - sirenStartTime < sirenDuration) {  
        // Toggle siren activation/deactivation  
        if ((millis() / 500) % 2 == 0) {  
          tone(piezoPin2, 800);  
          digitalWrite(ledred,HIGH);  
          //  
          // lcd.setCursor(0, 1);  
          lcd.setCursor(8,1);  
          lcd.print(triggeredSensors);  
          //  
        } else {  
          noTone(piezoPin2);  
          digitalWrite(piezoPin2, HIGH); // Stop siren  
          //  
          lcd.setCursor(8,1);  
          lcd.print("        ");  
          //  
          digitalWrite(ledred,LOW);  
        }  
      } else {  
        // End of siren sound  
        noTone(piezoPin2);  
        digitalWrite(piezoPin2, HIGH); // Stop siren  
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ARMD ByP CPs:");
        showing_by_cps = true;
        showing_vltd_cps = false;
        flag = -1;  
        alarmActive = false;    
        Serial.println("Σειρήνα σταμάτησε.");  
        for(int i=0;i<size;i++){  
          if(Bypass[i] == 0 && Burgl_v[i] == HIGH){  
            lcd.setCursor(0,0);  
            lcd.print("-----restart-----");  
            delay(t_sirene_restart);  
            lcd.clear();
            showing_vltd_cps = false;
            showing_by_cps = false;
            break;  
          }  
        }  
      }  
    }
  }
  // Main block (End) 

  delay(100); // Small delay to avoid problems
}

void sirene() {
  tone(piezoPin, buzzer_hz);
}

void print_rdy() {
  lcd.setCursor(0, 1);
  lcd.print("RDY,Cps:");
  for(int i = 0; i < 8; i++) {
    lcd.print(Burgl_v[i]);
  }
  
  lcd.setCursor(0, 0);
}

bool arming_check() {
  if(Bypass_ALL == false) {
    // BURGL_Load();
    for(int i = 0; i < 8; i++) {
      if(Burgl_v[i] == 1 && Bypassed_prog_v[i] == false) {
        arming_prep = false;
        return false;
      }
    }
    arming_prep = true;
    return true;
  }
  BURGL_Load(Burgl_v);
  for(int i = 0; i < 8; i++) {
    if(Burgl_v[i] == 1)
      Bypassed_All_Temp_v[i] = 1;
    else
      Bypassed_All_Temp_v[i] = 0;
  }
  arming_prep = true;
  return true;
}

void standby_key_timeout(char keys_pressed[]) {
  countKeys = 0;
  keys_pressed_bound = 6;
  start_time_key = 0;
  validInput = false;
  keys_pressed[0] = '\0';
  beep(piezoPin, buzzer_hz, buzzer_del, 3);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timeout due to");
  lcd.setCursor(0, 1);
  lcd.print("inactivity.");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
}

void prog_mode_on_off() {
  if (prog_mode == false) {
    standby_mode = false;
    prog_mode = true;
    keys_pressed_bound = 5;
    in_menu = true;
    menu_page = 1;
    start_time_prog = millis();
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Prog mode ON.");
    delay(1000);
    lcd.clear();
    print_menu(menu_page);
  } 
  else if (strcmp(keys_pressed, prog_code) == 0) {
    standby_mode = true;
    prog_mode = false;
    keys_pressed_bound = 6;
    start_time_prog = 0;
    in_menu = false;
    in_menu_function = false;
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Prog mode OFF.");
    delay(1000);
    lcd.clear();
  }
  else {
    beep(piezoPin, buzzer_hz, buzzer_del, 3);
  }
}

// Restarts the function selected by the user after timeout is canceled
void enable_fucntion_after_timout_canceled(char keys_pressed[]) {
  if (prog_bypass_cps) {
    keys_pressed[0] = '1';
    keys_pressed[1] = '\0';
    prog_bypass_cps = false;
  } else if (prog_burgl_count_down_cps_v) {
    keys_pressed[0] = '2';
    keys_pressed[1] = '\0';
    prog_burgl_count_down_cps_v = false;
  } else if (prog_user_code) {
    ;
    keys_pressed[0] = '3';
    keys_pressed[1] = '\0';
    prog_user_code = false;
  } else if (prog_prog_code) {
    keys_pressed[0] = '4';
    keys_pressed[1] = '\0';
    prog_prog_code = false;
  } else if (prog_fast_arming) {
    keys_pressed[0] = '5';
    keys_pressed[1] = '\0';
    prog_fast_arming = false;
  } else if (prog_t_arming_delay) {
    keys_pressed[0] = '6';
    keys_pressed[1] = '\0';
    prog_t_arming_delay = false;
  } else if (prog_t_disarming_delay) {
    keys_pressed[0] = '7';
    keys_pressed[1] = '\0';
    prog_t_disarming_delay = false;
  }

  validInput = true;
}

void update_lcd_arming(unsigned long current_time) {
  unsigned long elapsed_time = current_time - start_time_arming;
  int remaining_seconds = t_arming_delay - (elapsed_time - t_arming_delay) / 1000;

  if (remaining_seconds >= 0) {
    static int last_displayed_seconds = -1;

    if (remaining_seconds != last_displayed_seconds) {
      last_displayed_seconds = remaining_seconds;
      if(remaining_seconds >= 10)
        lcd.setCursor(11, 1);
      else {
        lcd.setCursor(11, 1);
        lcd.print("0");
        lcd.setCursor(12, 1);
      }
      lcd.print(remaining_seconds);
    }
  }
}

// Updates the seconds displayed on the LCD during the timeout warning
void update_lcd_timeout_warning(unsigned long current_time) {
  unsigned long elapsed_time = current_time - start_time_prog;
  int remaining_seconds = 5 - (elapsed_time - prog_timeout_warning * 1000) / 1000;

  if (remaining_seconds >= 0) {
    static int last_displayed_seconds = -1;

    if (remaining_seconds != last_displayed_seconds) {
      last_displayed_seconds = remaining_seconds;
      lcd.setCursor(12, 0);
      lcd.print(remaining_seconds);
    }
  }
}

// Resets variables after the timeout is canceled
void timeout_canceled_reset(char keys_pressed[]) {
  lcd.clear();
  lcd.setCursor(0, 0);
  prog_about_to_timeout = false;
  keys_pressed_bound = 5;
  validInput = false;
  start_time_prog = millis();
  keys_pressed[0] = '\0';
}

// Starts the warning for the timeout
void start_warning(char keys_pressed[]) {
  countKeys = 0;
  keys_pressed_bound = 0;
  validInput = false;
  keys_pressed[0] = '\0';

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timeout in: 5");
  lcd.setCursor(0, 1);
  lcd.print("Type to cancel.");
}

// Exits programming mode after timeout finishes
void prog_mode_off_timeout(char keys_pressed[]) {
  countKeys = 0;
  standby_mode = true;
  start_time_key = 0;
  prog_mode = false;
  keys_pressed_bound = 6;
  start_time_prog = 0;
  in_menu = false;
  validInput = false;
  prog_about_to_timeout = false;
  in_menu_function = false;
  prog_bypass_cps = false;
  prog_burgl_count_down_cps_v = false;
  prog_user_code = false;
  prog_prog_code = false;
  prog_fast_arming = false;
  prog_t_arming_delay = false;
  prog_t_disarming_delay = false;
  keys_pressed[0] = '\0';
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timeout due to");
  lcd.setCursor(0, 1);
  lcd.print("inactivity.");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Prog mode OFF.");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
}

// Checks which function to call based on user input in prog mode =============== PROGRAMMING START =====
bool prog_system(char keys_pressed[]) {

  if (prog_bypass_cps == true) {
    bypass_cps(keys_pressed);
    in_menu_function = false;
    prog_bypass_cps = false;
    keys_pressed_bound = 5;
  } else if (prog_burgl_count_down_cps_v == true) {
    burgl_count_down_cps_v(keys_pressed);
    in_menu_function = false;
    prog_burgl_count_down_cps_v = false;
    keys_pressed_bound = 5;
  } else if (prog_user_code == true) {
    change_user_code(keys_pressed);
    in_menu_function = false;
    prog_user_code = false;
  } else if (prog_prog_code == true) {
    change_prog_code(keys_pressed);
    in_menu_function = false;
    prog_prog_code = false;
  } else if (prog_fast_arming == true) {
    change_fast_arming(keys_pressed);
    in_menu_function = false;
    prog_fast_arming = false;
    keys_pressed_bound = 5;
  } else if (prog_t_arming_delay == true) {
    change_t_arming(keys_pressed);
    in_menu_function = false;
    prog_t_arming_delay = false;
    keys_pressed_bound = 5;
  } else if (prog_t_disarming_delay == true) {
    change_t_disarming(keys_pressed);
    in_menu_function = false;
    prog_t_disarming_delay = false;
    keys_pressed_bound = 5;
  } else if (strcmp(keys_pressed, "1") == 0) {
    in_menu_function = true;
    prog_bypass_cps = true;
    keys_pressed_bound = 9;
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("By_CPs: ");
    for (int i = 0; i < CPS_SIZE; i++)
      lcd.print(Bypassed_prog_v[i]);
    lcd.setCursor(0, 0);
  } else if (strcmp(keys_pressed, "2") == 0) {
    in_menu_function = true;
    prog_burgl_count_down_cps_v = true;
    keys_pressed_bound = 9;
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("CD_CPs: ");
    for (int i = 0; i < CPS_SIZE; i++)
      lcd.print(Burgl_Count_Down_CPs_v[i]);
    lcd.setCursor(0, 0);
  } else if (strcmp(keys_pressed, "3") == 0) {
    in_menu_function = true;
    prog_user_code = true;
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("User Code: ");
    lcd.print(user_code);
    lcd.setCursor(0, 0);
  } else if (strcmp(keys_pressed, "4") == 0) {
    in_menu_function = true;
    prog_prog_code = true;
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Prog Code: ");
    lcd.print(prog_code);
    lcd.setCursor(0, 0);
  } else if (strcmp(keys_pressed, "5") == 0) {
    in_menu_function = true;
    prog_fast_arming = true;
    keys_pressed_bound = 2;
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Quick Arm: ");
    lcd.print(fast_arming);
    lcd.setCursor(0, 0);
  } else if (strcmp(keys_pressed, "6") == 0) {
    in_menu_function = true;
    prog_t_arming_delay = true;
    keys_pressed_bound = 4;
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Arm Delay: ");
    lcd.print(t_arming_delay);
    lcd.setCursor(0, 0);
  } else if (strcmp(keys_pressed, "7") == 0) {
    in_menu_function = true;
    prog_t_disarming_delay = true;
    keys_pressed_bound = 4;
    beep(piezoPin, buzzer_hz, 0, 1);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Dsrm Delay: ");
    lcd.print(t_disarming_delay);
    lcd.setCursor(0, 0);
  } else if (strcmp(keys_pressed, "8") == 0) {
    fill_memory();
    read_memory();
    tone(piezoPin, buzzer_hz);
    lcd.clear();
    lcd.print("Memory reset.");
    delay(2000);
    noTone(piezoPin);
    digitalWrite(piezoPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
  } else if (strcmp(keys_pressed, prog_code) == 0) {
    return false;
  } else if (strlen(keys_pressed) == 4){
    beep(piezoPin, buzzer_hz, buzzer_del, 3);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong prog code.");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
  }else if (keys_pressed[0] != NULL) {
    beep(piezoPin, buzzer_hz, buzzer_del, 3);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid option.");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
  }

  return true;
}    // ======================================= PROGRAMMING END ===========================================

// Function to program bypass CPs
void bypass_cps(char keys_pressed[]) {

  for (int i = 0; i < strlen(keys_pressed); i++) {

    if ((keys_pressed[i] - '0') >= 1 && (keys_pressed[i] - '0') <= CPS_SIZE) {
      if (Bypassed_prog_v[(keys_pressed[i] - '0') - 1] == 0)
        Bypassed_prog_v[(keys_pressed[i] - '0') - 1] = 1;
      else
        Bypassed_prog_v[(keys_pressed[i] - '0') - 1] = 0;
    }
  }

  EEPROM.put(0, Bypassed_prog_v);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("By_CPs: ");
  for (int i = 0; i < CPS_SIZE; i++) {
    lcd.print(Bypassed_prog_v[i]);
  }

  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
}

// Function to program burglary countdown CPs
void burgl_count_down_cps_v(char keys_pressed[]) {

  for (int i = 0; i < strlen(keys_pressed); i++) {

    if ((keys_pressed[i] - '0') >= 1 && (keys_pressed[i] - '0') <= CPS_SIZE) {
      if (Burgl_Count_Down_CPs_v[(keys_pressed[i] - '0') - 1] == 0)
        Burgl_Count_Down_CPs_v[(keys_pressed[i] - '0') - 1] = 1;
      else
        Burgl_Count_Down_CPs_v[(keys_pressed[i] - '0') - 1] = 0;
    }
  }

  EEPROM.put(8, Burgl_Count_Down_CPs_v);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("CD_CPs: ");

  for (int i = 0; i < CPS_SIZE; i++) {
    lcd.print(Burgl_Count_Down_CPs_v[i]);
  }

  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
}

// Function to program user code
void change_user_code(char keys_pressed[]) {

  if (strcmp(keys_pressed, prog_code) == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("User can't be");
    lcd.setCursor(0, 1);
    lcd.print("same with prog.");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
  } else if (strlen(keys_pressed) == 4) {
    strcpy(user_code, keys_pressed);
    EEPROM.put(16, user_code);
    lcd.setCursor(0, 1);
    lcd.print("User Code: ");
    lcd.print(user_code);
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid input.");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
  }
}

// Function to program programming code
void change_prog_code(char keys_pressed[]) {

  if (strcmp(keys_pressed, user_code) == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("User can't be");
    lcd.setCursor(0, 1);
    lcd.print("same with prog.");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
  } else if (strlen(keys_pressed) == 4) {
    strcpy(prog_code, keys_pressed);
    EEPROM.put(21, prog_code);
    lcd.setCursor(0, 1);
    lcd.print("Prog Code: ");
    lcd.print(prog_code);
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid Input.");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
  }
}

// Function to program fast arming
void change_fast_arming(char keys_pressed[]) {

  if (strlen(keys_pressed) == 1) {
    strcpy(fast_arming, keys_pressed);
    EEPROM.put(26, fast_arming);
    lcd.setCursor(0, 1);
    lcd.print("Quick Arm: ");
    lcd.print(fast_arming);
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid Input.");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
  }
}

// Function to program arming delay
void change_t_arming(char keys_pressed[]) {

  if (strlen(keys_pressed) > 0) {
    int num = 1;
    t_arming_delay = 0;

    for (int i = strlen(keys_pressed) - 1; i >= 0; i--) {
      t_arming_delay += (keys_pressed[i] - '0') * num;
      num *= 10;
    }

    EEPROM.put(28, t_arming_delay);

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Arm Delay: ");
    lcd.print(t_arming_delay);
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid Input.");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
  }
}

// Function to program disarming delay
void change_t_disarming(char keys_pressed[]) {

  if (strlen(keys_pressed) > 0) {
    int num = 1;
    t_disarming_delay = 0;

    for (int i = strlen(keys_pressed) - 1; i >= 0; i--) {
      t_disarming_delay += (keys_pressed[i] - '0') * num;
      num *= 10;
    }

    EEPROM.put(30, t_disarming_delay);

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Dsrm Delay: ");
    lcd.print(t_disarming_delay);
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid Input.");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
  }
}

// Prints the menu with the ability to change between pages (1 or 2)
void print_menu(int menu_page) {

  lcd.clear();
  lcd.setCursor(0, 0);

  if (menu_page == 1) {
    lcd.print("1=By_CPs 2=CNT_D");
    lcd.setCursor(0, 1);
    lcd.print("3=USR  4=PRG D->");
  } else if (menu_page == 2) {
    lcd.print("5=Q_A      6=ARM");
    lcd.setCursor(0, 1);
    lcd.print("7=DSRM 8=RST D<-");
  }

  showing_menu = true;
}

// Checks the user input
bool check_keypad_entry(char keys_pressed[], char key) {

  if (key == 'D') {
    lcd.clear();
    lcd.setCursor(0, 0);
    if (countKeys != 0) {
      keys_pressed[countKeys] = '\0';
      countKeys = 0;
      return true;
    } else {
      keys_pressed[countKeys] = '\0';
      return true;
    }
  } 
  else if (key == 'C') {             // Handle backspace
    if (countKeys > 0) {               // Only backspace if there's something to delete
      beep(piezoPin, buzzer_hz, 0, 1);
      countKeys--;                     // Decrement the character count
      keys_pressed[countKeys] = '\0';  // Remove the last character from the array

      // Update the LCD
      lcd.setCursor(countKeys, 0);  // Move the cursor to the last character's position
      lcd.print(" ");               // Overwrite the character with a space
      lcd.setCursor(countKeys, 0);  // Reset the cursor to the cleared position */
    }
    else{
      beep(piezoPin, buzzer_hz, buzzer_del, 3);
    }
  } 
  else if (countKeys < keys_pressed_bound - 1) {
    if (prog_mode && key != '#' && key != '*') {
      beep(piezoPin, buzzer_hz, 0, 1);
      keys_pressed[countKeys] = key;
      lcd.setCursor(countKeys, 0);
      lcd.print(key);
      countKeys++;
    } 
    else if (!prog_mode && key != '*' && key != '#' && key != 'A') {
      beep(piezoPin, buzzer_hz, 0, 1);
      keys_pressed[countKeys] = key;
      lcd.setCursor(countKeys, 0);
      lcd.print('*');  
      countKeys++;
    }
    else {
      beep(piezoPin, buzzer_hz, buzzer_del, 3);
    }
  }
  else if (countKeys == keys_pressed_bound - 1) {
    beep(piezoPin, buzzer_hz, buzzer_del, 3);
  }

  return false;
}

// Disables arming
void arming_disable() {
  energo = 0;  
  alarmActive = false;  
  preAlarmActive = false;
  pli = 0;
  dsrm_tries = 3;
  arming_prep = false;
  alarm_armed = false;
  standby_mode = true;
  Bypass_ALL = false;
  keys_pressed_bound = 6;
  noTone(piezoPin);
  digitalWrite(piezoPin, HIGH);
  for (int i = 0; i < 8; i++) {
    Bypassed_All_Temp_v[i] = 0;
  }

  digitalWrite(ledred, LOW);
  digitalWrite(ledgreen, HIGH);

  armingB = false;  
  lcd.clear();  
  noTone(piezoPin2); // Stop siren  
  digitalWrite(piezoPin2, HIGH); // Disable buzzer  
  Serial.println("απενεργοποιηση σειρηνα");  

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Arming is");
  lcd.setCursor(0, 1);
  lcd.print("disabled.");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
}

// Countdown to arm the system
void t_arming_delay_function() {
  arming_prep = false;
  alarm_armed = true;
  armingB = true; 
  keys_pressed_bound = 5;
  digitalWrite(ledred, HIGH);
  digitalWrite(ledgreen, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System armed.");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
}

void beep(int buzzer, int hz, int del, int times) {
  for(int i = 0; i < times; i++) {
    tone(buzzer, hz);
    delay(20);
    noTone(buzzer);
    digitalWrite(buzzer, HIGH);
    delay(del);
  }
}