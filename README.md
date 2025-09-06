### RIOT-RC
This library contains hardware and software files for Robo RIOT's Rc controllers. These sets are meant to control 1lb, 3lb, and possibly 5lb robotics in a compact package, allowing any individual to easily create their own open-source battle bot electronics set.  


## Quickstart -- Preassembled Remote and Receiver

The RIOT RC has the following basic specs

# Receiver
| Parameter | Value |
|----------|----------|
| Supply Voltage  | 5V - 36V  |
| Idle Amperage Time  | .05A  |
| Default ESC Hookups | 2 |
| Default Servo Hookups | 2 |
| DC Drives | 2 |

# Remote 
| Parameter | Value |
|----------|----------|
| Operating Time | ~3 hours  |
| Radio Range  | ~40-60m  |
| Analog Joysticks | 2 |
| Buttons | 4 |

# Hookup

The pinout of the Receiver is the following:

<img width="1001" height="634" alt="Screenshot 2025-09-06 at 12 33 19 AM" src="https://github.com/user-attachments/assets/480ee33d-c25c-42c9-8541-a58169aa7bda" />


Attach your receiver as according to the diagram:

<img width="1019" height="520" alt="Screenshot 2025-09-06 at 1 02 53 AM" src="https://github.com/user-attachments/assets/bb22b6ed-3aa3-4c2c-9c89-1583e719abd8" />

# Basic Operation

The staring operation of the remote goes as such.

1. Start your remote
2. Select your desired reciever by username
3. Control your reciever

The controls for selecting your receiver go as such:

<img width="689" height="417" alt="Screenshot 2025-09-06 at 1 23 56 AM" src="https://github.com/user-attachments/assets/55943886-9c74-4ac4-98c8-31570566be78" />

The OLED screen will display all **Powered** receivers automatically with their assigned name. Select one to automatically connect it. 

The controls for the remote after the reciver go as such: 

<img width="948" height="436" alt="Screenshot 2025-09-06 at 1 29 56 AM" src="https://github.com/user-attachments/assets/114bae18-ae3b-420b-93ce-6f1c298fa1e2" />

Keep in mind that there is a **Software** based arming mechanism for the BLDC motor. The receiver will not activate it unless the arming command is first sent. It may take about 5 seconds to arm your receiver. **Please be cautious when actiavate the BLDC, as it's speed rating may be enough to harm.**

The brushed motors are programmed to act as a drive, moving according to a two wheeled battlebot. Although the supply voltage is attached to the motor drive, the original code is deisgned around a 6V N20, and thus creates a PWM signal to prevent users from frying the motor. To change this. Please refer to the **Change code** section for more information.

## Simple edits - Uploading Sketches

Many values as of right now are hard-coded into the remote or receiver to change, and most of those changes can be altered by re-flashing them. The software was created in PlatoformIO, and integrated IDE into VSCode for microcontrollers. Many resources are avaliable for you to download and operate the software.

Once PlatformIO has been downloaded, clone the repository and naviage to /RIOT-RC/PlatformIO Imports/ and select the platoformIO directory. 

**Caution**: The receiver is attached to the battery voltage. **FOR THE RECEIVER DO NOT ATTACH A USB WHILE A LIPO BATTERY IS ATTACHED. YOU WILL FRY YOUR USB PORT AND YOUR COMPUTER**

Most easily accessible commands will be placed at the top of main.cpp for ease of access. Alterations from the basic control scheme will be implemented later. 

<img width="1089" height="575" alt="Screenshot 2025-09-06 at 1 55 30 AM" src="https://github.com/user-attachments/assets/51247b05-15dc-4710-8b34-08ba830c4c66" />

# Battery Charging

The Remote has a one cell battery charger. Simply attach the charger to your USB to recharge it.


