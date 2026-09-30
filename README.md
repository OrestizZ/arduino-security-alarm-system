# Arduino Security Alarm System

A microcontroller-based security alarm system powered by **Arduino Uno**, designed to manage magnetic contact sensors, motion detectors, and user interactions through a keypad and an LCD screen.

## Core Features:
* **Arming & Disarming Modes:** Supports standard arming, fast arming (quick arm), and system bypass configurations using custom user codes.
* **Zone & Sensor Management:** Interfaces with magnetic contacts (CPs) to monitor entry points, featuring customizable bypass options and countdown zones for delayed triggers.
* **Interactive Keypad & LCD Display:** Features a 4x4 keypad for entering codes, navigating system settings, and viewing real-time status updates on a 16x2 I2C LCD screen.
* **Programming & Maintenance Menu:** Allows administrators to change user and programming codes, adjust arming/disarming delays, reset system memory via EEPROM, and configure quick-arm settings.
* **Alert & Siren Mechanism:** Triggers visual indicators (LEDs) and audio alerts (piezo buzzer and siren output) with pre-alarm countdowns and automatic timeouts upon security breaches.

## Technologies Used:
* **Hardware:** Arduino Uno, 4x4 Keypad, 16x2 I2C LCD, Piezo Buzzer, Status LEDs, Shift Registers / Digital I/O for sensor reading.
* **Libraries:** `Keypad`, `LiquidCrystal_I2C`, `EEPROM`, `Wire`, `string.h`.

## Authors

This project was created by:

* **Orestis Zappas** - @OrestizZ (https://github.com/OrestizZ)
* **Georgios Dilioridis** - @GeorgiosDilio (https://github.com/GeorgiosDilio)

as a university assignment for the **Department of Informatics and Telecommunications (University of Peloponnese)**.
