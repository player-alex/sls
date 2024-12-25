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

If you want mobile messaging(Alarm), you should use NotificationHubs & Firebase Cloud Messaging.  


# Device Provisioning

Before a device can communicate with IoT Hub, it needs a SAS key.   
Follow next instruction: [Device authentication concepts in IoT Central](https://learn.microsoft.com/en-us/azure/iot-central/core/concepts-device-authentication)

# Audio
The system embeds audio files through the following parts:  
**TTS -> MP3 -> WAV -> Wav2Code**  
</b>  
Audio, not voice, is referenced at: pixabay.com

# Power Management
The system controls the modules with PNP Transistors to save power in active mode.  
And if there no input for 30 seconds, it will enter deep sleep mode.    


# TODO
1. Light Sleep  
There is a slight delay when recovering from Inactive Mode to Active Mode.  
So we should use light sleep.  
The important part is the need for fine-grained control over Executable Tasks.  
If not controlled, a crash will occur.  

2. Executable Task Refactoring  
Need executable task refactoring.  

# Known Issues
1. Azure SDK "Subscribe" functions  
In my experience, i got an MQTT error when calling the subscribe functions.  
The cause has not been idenfitied.

2. JM-101B(Fingerprint Reader) Communication Hangs  
If the connection is suddenly disconnected during transmission, the system will hang.  

3. Inaccurate Motor Control  
If you want more precise control, you should use a timer.

# Notice
This is not the final version.  