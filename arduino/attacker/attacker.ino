/*
 * ============================================================
 *  Attacker — Dual-Line Sniffer + LCD
 *  Hardware : ATmega328P + 16x2 LCD
 * ============================================================
 *  Taps BOTH directions of the UART link:
 *    RX (PD0)    ← Alice TX    (sees Alice's DH + encrypted data)
 *    Pin 6       ← Bob TX      (sees Bob's DH reply)
 *
 *  LCD: RS=12, E=11, D4=5, D5=4, D6=3, D7=2
 * ============================================================
 */

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// SoftwareSerial: RX on pin 6 (taps Bob's TX), TX unused (pin 7)
SoftwareSerial bobLine(6, 7);

uint8_t pktNum = 0;
uint8_t col = 0;
bool inFrame = false;
bool waitAlicePK = false;
bool waitBobPK = false;

void setup() {
  Serial.begin(9600);      // Alice's TX
  bobLine.begin(9600);     // Bob's TX
  lcd.begin(16, 2);
  lcd.print("DUAL SNIFFER");
  lcd.setCursor(0, 1);
  lcd.print("2-Line Tap...");
}

void loop() {
  // ── Listen to Alice's TX line (hardware Serial) ──────────
  while (Serial.available()) {
    uint8_t b = Serial.read();

    // Alice DH packet: [0xD1][pubKey]
    if (b == 0xD1 && !inFrame) {
      waitAlicePK = true;
      continue;
    }
    if (waitAlicePK) {
      lcd.clear();
      lcd.print("Alice PK: ");
      lcd.print(b);
      lcd.setCursor(0, 1);
      lcd.print("(Intercepted!)");
      waitAlicePK = false;
      continue;
    }

    // Encrypted frame: [0xAA][8 bytes][0x55]
    if (b == 0xAA && !inFrame) {
      inFrame = true;
      pktNum++;
      lcd.clear();
      lcd.print("Pkt#");
      lcd.print(pktNum);
      lcd.print(" SNIFFED:");
      lcd.setCursor(0, 1);
      col = 0;
      continue;
    }
    if (b == 0x55 && inFrame) {
      inFrame = false;
      continue;
    }
    if (inFrame && col < 16) {
      if (b < 0x10) lcd.print('0');
      lcd.print(b, HEX);
      col += 2;
    }
  }

  // ── Listen to Bob's TX line (SoftwareSerial) ─────────────
  while (bobLine.available()) {
    uint8_t b = bobLine.read();

    // Bob DH packet: [0xD2][pubKey]
    if (b == 0xD2) {
      waitBobPK = true;
      continue;
    }
    if (waitBobPK) {
      lcd.clear();
      lcd.print("Bob PK: ");
      lcd.print(b);
      lcd.setCursor(0, 1);
      lcd.print("(Intercepted!)");
      waitBobPK = false;
      continue;
    }
  }
}
