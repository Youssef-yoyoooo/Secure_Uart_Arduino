# Simulation and Implementation Guide
## Digital System Interfacing (DSI) Project

### Overview
This directory contains technical guidance for simulating and implementing the Secure UART communication system. The simulation models a point-to-point UART link between three distinct nodes to demonstrate cryptographic security and the limitations of passive sniffing attacks.

### Hardware Configuration

#### Node Specification
* **Node A (Alice)**: Secure transmitter. Interface includes a hardware trigger (Pin 8) and an LM35 temperature sensor (A0).
* **Node B (Bob)**: Secure receiver. Interface includes a 16x2 LCD for real-time data visualization.
* **Node C (Attacker)**: Passive dual-line sniffer. Interface includes a 16x2 LCD to monitor intercepted traffic.

#### Wiring Logic
* **Alice TX (PD1)**: Connected to Bob RX (PD0) and Attacker RX (PD0).
* **Bob TX (PD1)**: Connected to Alice RX (PD0) and Attacker Pin 6 (SoftwareSerial RX).

### System Logic and Workflow

#### Secure Handshake (DH-Lite)
1. **Public Key Exchange**: Alice and Bob generate 8-bit public/private key pairs using a custom Diffie-Hellman implementation (prime 251, base 6).
2. **Transmission**: 1-byte public keys are transmitted over the UART link.
3. **Security**: The Attacker can intercept these keys but cannot efficiently derive the private exponents due to the Discrete Logarithm Problem.

#### Key Derivation and Expansion
Both nodes compute a shared secret independently. This 8-bit secret is expanded into a 128-bit key schedule, which is required for the symmetric encryption phase.

#### Symmetric Encryption (Speck 64/128)
* **Trigger**: Upon button press, Alice samples the LM35 sensor.
* **Encryption**: The data is encrypted using the Speck 64/128 block cipher (27 rounds).
* **Framing**: The encrypted 8-byte block is encapsulated within a frame delimited by start (0xAA) and end (0x55) markers to ensure synchronization.

### Operational Verification
To verify the system performance in a simulation environment (e.g., Proteus):
1. **Initialization**: Start the simulation and allow 2 seconds for node synchronization and handshake completion.
2. **Key Capture**: Observe the Attacker's LCD to confirm interception of the public keys (Alice PK and Bob PK).
3. **Data Transmission**: Trigger a transmission via Alice's button.
4. **Decryption Verification**: Confirm that Bob's LCD displays the correctly decrypted message and sensor data.
5. **Security Validation**: Observe that the Attacker's LCD captures the payload but displays it as hexadecimal ciphertext, confirming that the data remains confidential.
