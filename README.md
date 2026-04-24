# Secure UART: End-to-End Encrypted Communication
## Digital Systems Integration (DSI) Project

### 📌 Project Overview
This project implements a secure communication channel between two microcontrollers (Arduinos) over a standard UART interface. By default, UART transmits data in plaintext, making it vulnerable to eavesdropping (sniffing) and data injection.

This system secures the link using:
* **Diffie-Hellman (Curve25519) Key Exchange**: To safely establish a shared secret without hardcoding keys.
* **AES-128 Symmetric Encryption**: To ensure data confidentiality.
* **Frame Integrity**: To prevent Man-in-the-Middle (MitM) message tampering.

### 🛠️ System Architecture

#### The Hardware Setup
* **Node A (Alice)**: The Transmitter. Generates sensor data or commands.
* **Node B (Bob)**: The Receiver. Decrypts and executes commands (e.g., controlling a motor or LED).
* **Node C (The Attacker)**: A "sniffer" node connected to the TX line of Node A to demonstrate the failure of standard communication vs. the success of this project.

#### The Security Protocol
1. **Handshake Phase**: Alice and Bob exchange public keys using the Curve25519 algorithm.
2. **Key Derivation**: Both nodes compute a 32-byte shared secret. The first 16 bytes are used as the AES-128 Key.
3. **Encrypted Tunnel**: All subsequent data is padded to 16-byte blocks, encrypted, and sent as a binary packet.

### 🔒 Security Features vs. Attack Vectors

| Attack Vector | Standard UART | This Project | Mitigation Technique |
| :--- | :--- | :--- | :--- |
| Passive Sniffing | ❌ Vulnerable | ✅ Secure | AES-128 Encryption |
| Key Theft | ❌ Hardcoded | ✅ Secure | Diffie-Hellman Exchange |
| Data Tampering | ❌ Vulnerable | ✅ Secure | Block Padding & Integrity Checks |
| Replay Attack | ❌ Vulnerable | ✅ Partial | Nonce/Counter (Optional Feature) |