# Smart Door Lock System
This project is Smart(IoT) Door Lock System based on ESP32-S3 and Microsoft Azure.  
  
# Requirements
1. ESP32-S3 N16R8
2. PNP Transistor(Current Resistance >= 350mA) * 3
3. DC Geared Motor
4. MAX98357A Module
5. L298N Motor Driver
6. Microsoft Azure Account

# Azure Services
1. IoT Central
2. Functions

# TODO
1. Light Sleep
There is a slight delay when recovering from Inactive Mode to Active Mode.  
So we should use light sleep.  
The important part is the need for fine-grained control over Executable Tasks.  
If not controlled, a crash will occur.

# Known Issues
1. Azure SDK "Subscribe" functions
In my experience, i got an MQTT error when calling the subscribe functions.  
The cause has not been idenfitied.

2. JM-101B(Fingerprint Reader) Communication Hangs
If the connection is suddenly disconnected during transmission, the system will hang.  
