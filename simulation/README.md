# Simulation & Implementation Guide

This directory contains guidance for setting up the Secure UART communication in **Proteus** or other simulation environments.

## ⚡ How the Simulation Works

The simulation models a point-to-point UART link between three distinct nodes to demonstrate both successful lightweight encryption and the failure of classical sniffing.

### 1. The Hardware Setup
- **Node A (Alice)**: Acts as the secure transmitter. Equipped with a push button (Pin 8) and an LM35 temperature sensor (A0).
- **Node B (Bob)**: Acts as the secure receiver. Equipped with a 16x2 LCD for visualization.
- **Node C (Attacker)**: A "Dual-Line Sniffer" Arduino. Equipped with a 16x2 LCD.
- **Wiring**: 
  - Alice `TX` (PD1) is wired to Bob `RX` (PD0) **AND** Attacker `RX` (PD0).
  - Bob `TX` (PD1) is wired to Alice `RX` (PD0) **AND** Attacker `Pin 6` (SoftwareSerial RX).

### 2. The Communication Flow
1. **Public Key Exchange (DH-Lite)**: 
   - Alice and Bob generate 8-bit public/private keys using an ultra-lightweight custom DH algorithm (prime 251, base 6).
   - These 1-byte public keys are transmitted over the UART line. The Attacker intercepts both, but cannot easily derive the private keys (Discrete Logarithm Problem).
2. **Key Derivation**: 
   - Both nodes compute the **Shared Secret**. This secret never travels over the wire. It is expanded into a 128-bit key schedule.
3. **Encrypted Tunneling (Speck 64/128)**:
   - When Alice's button is pressed, she reads the LM35 and encrypts the data block using the **Speck** lightweight block cipher (27 rounds).
   - The encrypted 8-byte block is framed by markers (`0xAA` start, `0x55` end) and sent.

## 🛠️ Software Logic (The Code)

### Secure Handshake
The State Machine handles synchronization. It ensures both nodes exchange their `0xD1` and `0xD2` key packets before any payload data is processed.

### Authenticated Encryption
We use the NSA's **Speck** cipher because it provides strong confidentiality while being entirely optimized for microcontrollers (it uses only Add, Rotate, and XOR operations—no lookup tables).

## 🕵️ Observing the "Attack"
To see the system in action within Proteus:
1. Start the simulation. Wait 2 seconds for the nodes to boot.
2. Watch the **Attacker's LCD**: You will see it successfully intercept `Alice PK` and then `Bob PK`.
3. Press **Alice's Button**.
4. Watch **Bob's LCD**: It will successfully decrypt the message and show `Msg #1 OK / TMP:XXC`.
5. Watch the **Attacker's LCD**: It will capture the payload, but it will display as random 16-character hexadecimal garbage. 
   - **Result**: The Sniffer taps both wires successfully but fails to read the information.