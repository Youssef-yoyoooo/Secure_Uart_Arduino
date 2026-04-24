# Simulation & Implementation Guide

This directory contains guidance for setting up the Secure UART communication in **Proteus** or other simulation environments.

## ⚡ How the Simulation Works

The simulation models a point-to-point UART link between three distinct nodes to demonstrate both successful encryption and the failure of classical sniffing.

### 1. The Hardware Setup
- **Node A (Alice)**: Acts as the secure transmitter.
- **Node B (Bob)**: Acts as the secure receiver.
- **Node C (Attacker)**: A "Man-in-the-Middle" or "Sniffer" Arduino or Virtual Terminal.
- **Wiring**: Alice's `TX` pin is wired to Bob's `RX` pin. The Attacker's `RX` pin is also tapped into this same line.

### 2. The Communication Flow
1. **Public Key Exchange**: 
   - Alice and Bob generate high-entropy 32-byte public/private keys using **Curve25519**.
   - These public keys are transmitted over the UART line. The Attacker can see these, but cannot derive the private keys.
2. **Key Derivation**: 
   - Both nodes compute the **Shared Secret**. This secret never travels over the wire.
3. **Encrypted Tunneling**:
   - Alice encrypts data using **AES-128 GCM**.
   - A unique 12-byte **IV (Initialization Vector)** and a 16-byte **Auth Tag** are sent with each packet.

## 🛠️ Software Logic (The Code)

### Secure Handshake
The `performHandshake()` function handles the synchronization. It ensures both nodes have shared their keys before any payload data is processed.

### Authenticated Encryption
We use **GCM (Galois/Counter Mode)** because it provides:
- **Confidentiality**: The `plaintext` is converted to `ciphertext`.
- **Integrity**: The `Tag` ensures that if a single bit of the ciphertext is changed during transmission (e.g., by an injector), the receiver (Bob) will reject it.

## 🕵️ Observing the "Attack"
To see the system in action within Proteus:
1. Open three **Virtual Terminals**.
2. **Terminal A (Alice)**: Shows the plaintext data before it is sent.
3. **Terminal B (Bob)**: Shows the successfully decrypted message.
4. **Terminal C (Sniffer)**: Shows what is "actually on the wire". You will see:
   - Initial public keys (binary data).
   - Subsequent messages appearing as random "garbage" characters (the ciphertext). 
   - **Result**: The Sniffer fails to read the information.
