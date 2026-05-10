# Secure UART: End-to-End Encrypted Communication
## Digital System Interfacing (DSI) Course Project

### Overview
This project implements a secure, encrypted communication channel between two microcontrollers (ATmega328P) over a standard UART interface. Standard UART communication transmits data in plaintext, which is inherently vulnerable to passive eavesdropping and unauthorized data access. This system addresses these vulnerabilities by layering a lightweight cryptographic protocol stack over the serial link, optimized for resource-constrained 8-bit environments.

### Core Security Features
* **DH-Lite Key Exchange**: A lightweight implementation of the Diffie-Hellman protocol using an 8-bit prime (251) and generator (6). This allows for fast, session-specific shared secret generation without pre-shared keys.
* **Speck 64/128 Encryption**: A modern, lightweight block cipher designed for IoT devices. It utilizes an ARX (Addition-Rotation-XOR) architecture, avoiding memory-intensive lookup tables or S-Boxes, making it ideal for microcontrollers with limited RAM.
* **Cryptographic Framing**: Implements a synchronization protocol using start (0xAA) and end (0x55) delimiters to ensure data integrity and proper block alignment over noisy serial lines.
* **Passive Sniffing Resistance**: Demonstrates resistance to wiretapping via a third "Attacker" node that intercepts the communication but remains unable to decipher the encrypted payload.

### System Architecture

#### Node Roles
* **Alice (Transmitter)**: Monitors an LM35 temperature sensor. Upon a hardware trigger (button press), it performs a DH-Lite handshake, encrypts the sensor data using Speck 64/128, and transmits the ciphered payload.
* **Bob (Receiver)**: Participates in the key exchange, derives the shared secret, and decrypts incoming UART traffic. Results are displayed in real-time on a 16x2 LCD interface.
* **Attacker (Sniffer)**: A passive monitoring node connected to the TX/RX lines. It captures and displays the raw UART traffic on an LCD to prove that the data remains confidential even when intercepted.

#### Communication Workflow
1. **Handshake Phase**: Nodes exchange public keys to establish a shared secret.
2. **Key Expansion**: The 8-bit shared secret is expanded into a 128-bit key schedule required for the Speck cipher.
3. **Encrypted Transmission**: Payload data is padded, encrypted in 64-bit blocks, and transmitted within a secure frame.

### Security Analysis

| Attack Vector | Standard UART | Secure UART (This Project) | Mitigation Technique |
| :--- | :--- | :--- | :--- |
| Passive Sniffing | Vulnerable | Secure | Speck 64/128 Block Cipher |
| Key Compromise | High Risk (Hardcoded) | Mitigated | DH-Lite Dynamic Key Exchange |
| Resource Constraints | Inefficient (e.g., AES) | Optimized | ARX-based Speck Cipher |
| Discrete Log Attack | N/A | Resistant (at 8-bit scale) | Discrete Logarithm Problem |

### Project Team
* **Youssef Basem** (240919)
* **Moaaz Tamer** (245789)
* **Marina Ayman** (246635)
* **Gamila Hasan** (242127)
* **Ramy Emad** (241897)

**Instructor:** Gehad Mohey