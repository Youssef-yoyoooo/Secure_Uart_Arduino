/*
 * ============================================================
 *  Bob — Speck 64/128 + DH-Lite + LCD Visualization
 *  Hardware : ATmega328P @ 16 MHz + 16x2 LCD (4-bit)
 * ============================================================
 *  LCD: RS=12, E=11, D4=5, D5=4, D6=3, D7=2
 *  RX (PD0) ← Alice TX,  TX (PD1) → Alice RX
 * ============================================================
 */

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// ── DH-Lite ─────────────────────────────────────────────────
const uint8_t DH_P = 251;
const uint8_t DH_G = 6;

uint8_t myPrivate, myPublic, peerPublic, sharedSecret;

uint8_t dh_lite_modpow(uint8_t base, uint8_t exp, uint8_t mod) {
  uint16_t result = 1;
  uint16_t b = base % mod;
  while (exp > 0) {
    if (exp & 1) result = (result * b) % mod;
    exp >>= 1;
    b = (b * b) % mod;
  }
  return (uint8_t)result;
}

// ── Speck 64/128 ────────────────────────────────────────────
#define SPECK_ROUNDS 27
uint32_t roundKeys[SPECK_ROUNDS];

#define ROR32(x, r) (((x) >> (r)) | ((x) << (32 - (r))))
#define ROL32(x, r) (((x) << (r)) | ((x) >> (32 - (r))))

void speck_key_schedule(const uint32_t K[4]) {
  uint32_t l[3] = { K[1], K[2], K[3] };
  roundKeys[0] = K[0];
  for (uint8_t i = 0; i < SPECK_ROUNDS - 1; i++) {
    uint8_t li = i % 3;
    l[li] = (ROR32(l[li], 8) + roundKeys[i]) ^ i;
    roundKeys[i + 1] = ROL32(roundKeys[i], 3) ^ l[li];
  }
}

void speck_decrypt(uint32_t* x, uint32_t* y) {
  for (int8_t i = SPECK_ROUNDS - 1; i >= 0; i--) {
    *y = ROR32(*y ^ *x, 3);
    *x = ROL32((*x ^ roundKeys[i]) - *y, 8);
  }
}

void buildSpeckKey(uint8_t secret) {
  uint32_t K[4];
  K[0] = (uint32_t)secret * 0x01010101UL;
  K[1] = (uint32_t)(secret ^ 0x5A) * 0x00FF00FFUL;
  K[2] = (uint32_t)(secret + 42) * 0x0F0F0F0FUL ^ 0xDEADBEEF;
  K[3] = (uint32_t)(secret * 7) * 0x33333333UL ^ 0xCAFEBABE;
  speck_key_schedule(K);
}

void lcdShow(const char* l1, const char* l2) {
  lcd.clear(); lcd.print(l1); lcd.setCursor(0, 1); lcd.print(l2);
}

// ── State Machine ───────────────────────────────────────────
enum Phase {
  PH_BOOT, PH_KEYGEN, PH_WAIT_ALICE,
  PH_SEND_PK, PH_DERIVE, PH_SUCCESS, PH_IDLE
};
Phase phase = PH_BOOT;
unsigned long timer = 0;
uint8_t msgNum = 0;

uint8_t frameBuf[8];
uint8_t frameIdx = 0;
bool inFrame = false;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("  SECURE UART");
  lcd.setCursor(0, 1);
  lcd.print(" Speck + DH-Lite");
  myPrivate = (analogRead(A0) % 249) + 2;
  timer = millis();
}

void loop() {
  switch (phase) {

    case PH_BOOT:
      if (millis() - timer >= 1000) phase = PH_KEYGEN;
      break;

    case PH_KEYGEN:
      lcdShow("DH-Lite KeyGen", "g^b mod 251...");
      myPublic = dh_lite_modpow(DH_G, myPrivate, DH_P);
      lcd.clear();
      lcd.print("My PK: ");
      lcd.print(myPublic);
      lcd.setCursor(0, 1);
      lcd.print("Waiting Alice..");
      phase = PH_WAIT_ALICE;
      break;

    case PH_WAIT_ALICE:
      if (Serial.available() >= 2) {
        if (Serial.peek() == 0xD1) {
          Serial.read();                     // consume 0xD1
          peerPublic = Serial.read();        // 1 byte!

          lcd.clear();
          lcd.print("Step 1: Recv'd");
          lcd.setCursor(0, 1);
          lcd.print("Alice PK: ");
          lcd.print(peerPublic);
          timer = millis();
          phase = PH_SEND_PK;
        } else {
          Serial.read();
        }
      }
      break;

    case PH_SEND_PK:
      if (millis() - timer >= 1500) {
        lcdShow("Step 2: Sending", "My Public Key..");
        Serial.write(0xD2);
        Serial.write(myPublic);              // 1 byte!
        timer = millis();
        phase = PH_DERIVE;
      }
      break;

    case PH_DERIVE:
      if (millis() - timer >= 1500) {
        lcdShow("Step 3: DH-Lite", "B^b mod 251...");
        sharedSecret = dh_lite_modpow(peerPublic, myPrivate, DH_P);
        buildSpeckKey(sharedSecret);

        lcd.clear();
        lcd.print("Secret: ");
        lcd.print(sharedSecret);
        lcd.setCursor(0, 1);
        lcd.print("Key Exchange OK!");
        timer = millis();
        phase = PH_SUCCESS;
      }
      break;

    case PH_SUCCESS:
      if (millis() - timer >= 2000) {
        lcdShow("SECURE LINK", "Waiting Data...");
        phase = PH_IDLE;
      }
      break;

    case PH_IDLE:
      while (Serial.available()) {
        uint8_t b = Serial.read();

        if (!inFrame && b == 0xAA) {
          inFrame = true;
          frameIdx = 0;
          continue;
        }

        if (inFrame) {
          if (frameIdx < 8) {
            frameBuf[frameIdx++] = b;
          } else {
            // 9th byte should be 0x55 end marker
            if (b == 0x55) {
              decryptAndDisplay();
            }
            inFrame = false;
            frameIdx = 0;
          }
        }
      }
      break;
  }
  delay(5);
}

void decryptAndDisplay() {
  uint32_t x, y;
  memcpy(&x, frameBuf, 4);
  memcpy(&y, frameBuf + 4, 4);
  speck_decrypt(&x, &y);

  char plain[9];
  memcpy(plain, &x, 4);
  memcpy(plain + 4, &y, 4);
  plain[8] = '\0';

  msgNum++;
  lcd.clear();
  lcd.print("Msg #");
  lcd.print(msgNum);
  lcd.print(" OK");
  lcd.setCursor(0, 1);
  lcd.print(plain);
}
