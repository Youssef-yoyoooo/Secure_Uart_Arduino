# System Architecture

## Overview
Secure UART communication between two ATmega328P nodes using:
- **Diffie-Hellman Key Exchange** (modular exponentiation, p=65521, g=3)
- **Speck 64/128 Cipher** (NSA lightweight block cipher for IoT)

## Protocol — What Goes On The Wire

### Phase 1: Key Exchange
```
Alice → [0xD1][PK_HIGH][PK_LOW]     Alice's public key (g^a mod p)
Bob   → [0xD2][PK_HIGH][PK_LOW]     Bob's public key   (g^b mod p)
```
Both compute: `shared = peer^own mod p = g^(ab) mod p`

The attacker CAN see both public keys. But computing the private exponent
from a public key requires solving the **Discrete Logarithm Problem** — which
is computationally infeasible even for small primes at simulation scale.

### Phase 2: Encrypted Data
```
Alice → [0xAA][8 bytes ciphertext][0x55]
```
Ciphertext is a Speck 64/128 encrypted block using the shared secret as key.

## Speck 64/128
- **Block size**: 64 bits (two 32-bit words)
- **Key size**: 128 bits (four 32-bit words)
- **Rounds**: 27
- **Operations**: rotate right 8, rotate left 3, addition, XOR
- **No lookup tables** — ideal for microcontrollers with limited memory

## Wiring
```
Alice TX (PD1) ──┬──► Bob RX (PD0)
                 └──► Attacker RX (PD0)
Bob TX (PD1)   ──────► Alice RX (PD0)
Button ──────────────► Alice pin 7 (PD7) → GND

Bob LCD:      RS=12, E=11, D4=5, D5=4, D6=3, D7=2
Attacker LCD: RS=12, E=11, D4=5, D5=4, D6=3, D7=2
```
