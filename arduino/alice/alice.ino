/*
 * ============================================================
 *  Alice — Speck 64/128 + DH-Lite + LM35 Temperature
 *  Hardware : ATmega328P @ 16 MHz
 *             Button on pin 8 (internal pull-up)
 *             LM35 on A0
 * ============================================================
 *  Button press → reads LM35 temp → encrypts → sends to Bob
 *
 *  TX (PD1) → Bob RX + Attacker RX (tap)
 *  RX (PD0) ← Bob TX
 *  A0       ← LM35 output (10mV per °C)
 * ============================================================
 */

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

void speck_encrypt(uint32_t* x, uint32_t* y) {
  for (uint8_t i = 0; i < SPECK_ROUNDS; i++) {
    *x = (ROR32(*x, 8) + *y) ^ roundKeys[i];
    *y = ROL32(*y, 3) ^ *x;
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

// ── State Machine ───────────────────────────────────────────
enum Phase { PH_KEYGEN, PH_SEND_PK, PH_DERIVE, PH_READY };
Phase phase = PH_KEYGEN;
unsigned long timer = 0;

const int BTN = 8;
bool lastBtn = HIGH;
bool btnReady = false;

void setup() {
  Serial.begin(9600);
  pinMode(BTN, INPUT_PULLUP);
  delay(200);
  lastBtn = HIGH;
  // Use A1 for DH seed (A0 is now LM35)
  myPrivate = (analogRead(A1) % 249) + 2;
}

void loop() {
  switch (phase) {
    case PH_KEYGEN:
      myPublic = dh_lite_modpow(DH_G, myPrivate, DH_P);
      phase = PH_SEND_PK;
      timer = millis();
      break;

    case PH_SEND_PK:
      if (millis() - timer >= 1500) {
        Serial.write(0xD1);
        Serial.write(myPublic);
        timer = millis();
      }
      if (Serial.available() >= 2) {
        if (Serial.peek() == 0xD2) {
          Serial.read();
          peerPublic = Serial.read();
          phase = PH_DERIVE;
        } else {
          Serial.read();
        }
      }
      break;

    case PH_DERIVE:
      sharedSecret = dh_lite_modpow(peerPublic, myPrivate, DH_P);
      buildSpeckKey(sharedSecret);
      delay(500);
      lastBtn = digitalRead(BTN);
      btnReady = true;
      phase = PH_READY;
      break;

    case PH_READY:
      if (btnReady) {
        bool btn = digitalRead(BTN);
        if (lastBtn == HIGH && btn == LOW) {
          delay(50);
          if (digitalRead(BTN) == LOW) {
            // Read LM35 temperature from A0
            int raw = analogRead(A0);
            int tempC = (int)((long)raw * 500 / 1024);

            // Format: "TMP:XXC" (fits in 8-byte block)
            char msg[9];
            snprintf(msg, 9, "TMP:%dC", tempC);
            sendEncrypted(msg);
          }
        }
        lastBtn = btn;
      }
      break;
  }
  delay(5);
}

void sendEncrypted(const char* msg) {
  uint32_t x = 0, y = 0;
  uint8_t* bx = (uint8_t*)&x;
  uint8_t* by = (uint8_t*)&y;
  for (uint8_t i = 0; i < 4 && msg[i]; i++) bx[i] = msg[i];
  for (uint8_t i = 0; i < 4 && msg[i + 4]; i++) by[i] = msg[i + 4];

  speck_encrypt(&x, &y);

  Serial.write(0xAA);
  Serial.write((uint8_t*)&x, 4);
  Serial.write((uint8_t*)&y, 4);
  Serial.write(0x55);
}
