# System Architecture

## Overview
This document describes the technical details of the Secure UART implementation.

## Communication Flow
1. **Power-on**: Both nodes initialize their Serial pins.
2. **Key Exchange**: Alice sends her public key; Bob responds with his.
3. **Session Establishment**: Shared secret derived using Curve25519.
4. **Data Transmission**: AES-128 used for all subsequent packets.

## Future Improvements
- Implement HMAC for better integrity.
- Add support for multiple baud rates.
