# Secure UART: End-to-End Encrypted Communication
## Digital Systems Integration (DSI) Project

### 📌 Project Overview
This project implements a secure communication channel between two microcontrollers (Arduinos) over a standard UART interface. By default, UART transmits data in plaintext, making it vulnerable to eavesdropping (sniffing) and data injection.

This system secures the link using ultra-lightweight, IoT-optimized cryptography:
* **DH-Lite Key Exchange**: A custom 8-bit Diffie-Hellman implementation for microsecond-fast shared secret generation.
* **Speck 64/128 Encryption**: An NSA-designed lightweight block cipher optimized for resource-constrained microcontrollers (no lookup tables, minimal RAM).
* **Dual-Line Sniffing Demonstration**: Proves that even with full visibility of the handshake and data, an attacker cannot break the encryption.

### 🛠️ System Architecture

#### The Hardware Setup
* **Node A (Alice)**: The Transmitter. Reads an LM35 temperature sensor on button press, encrypts the data, and sends it.
* **Node B (Bob)**: The Receiver. Decrypts the data and visualizes the key exchange and decrypted messages on a 16x2 LCD.
* **Node C (The Attacker)**: A dual-line "sniffer" node tapping both Alice's and Bob's TX lines. Demonstrates that intercepted traffic is unreadable.

#### The Security Protocol
1. **Handshake Phase (DH-Lite)**: Alice and Bob exchange 1-byte public keys using modular exponentiation over the 8-bit prime 251.
2. **Key Derivation**: Both nodes compute a shared secret. This secret is expanded into a 128-bit key schedule.
3. **Encrypted Tunnel (Speck)**: Data (e.g., `"TMP:25C"`) is padded to 8-byte blocks, encrypted using Speck 64/128 (27 rounds), and sent over UART framed by `0xAA` and `0x55` markers.

### 🔒 Security Features vs. Attack Vectors

| Attack Vector | Standard UART | This Project | Mitigation Technique |
| :--- | :--- | :--- | :--- |
| Passive Sniffing | ❌ Vulnerable | ✅ Secure | Speck 64/128 Encryption |
| Key Theft | ❌ Hardcoded | ✅ Secure | DH-Lite Key Exchange |
| Resource Exhaustion | ❌ CPU Heavy | ✅ Optimized | Speck Cipher (Add/Rot/Xor only) |
| Complete Wiretap | ❌ Vulnerable | ✅ Secure | Discrete Logarithm Problem |