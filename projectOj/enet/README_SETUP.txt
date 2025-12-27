ENet Library Setup
==================

This directory contains the ENet networking library.

Library Files:
- enet64.lib (64-bit static library) - Used for linking
- enet.lib (32-bit static library) - Not used (project is 64-bit)

Header Files:
- include/enet/enet.h - Main ENet header
- include/enet/*.h - Additional ENet headers

Project Configuration:
- Include directory: enet/include
- Library directory: enet
- Library name: enet64

The project is configured to automatically link enet64.lib.
No DLL files are needed as ENet is statically linked.

