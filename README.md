# NX13A
### Summary
6 axis biped robot with M5Atom S3

### Programming M5Atom S3
Programming [NX13A_walking.ino](M5AtomS3/NX13A_walking/NX13A_walking.ino) M5Atom S3 with Arduino IDE 

#### Arduino Librairies
Arduino Librairies:
* [M5AtomS3](https://github.com/m5stack/M5AtomS3) for M5AtomS3.h
* [DabbleESP32](https://github.com/STEMpedia/DabbleESP32)



### Connection

| Servo  | GPIO  | Usage |
| ---- | ------| --------- |
| Srv0 | GPIO5 | Right Arm |
| Srv1 | GPIO6 | Right Leg |
| Srv2 | GPIO7 | Right Foot |
| Srv3 | GPIO8 | Left Foot |
| Srv4 | GPIO38 | Left Leg |
| Srv5 | GPIO29 | Left Arm |

#### Schematic

![Connection](Connection.jpg)



### Remote control
The remote control uses the iPhone application called Dabble. Dabble is a versatile project interaction & Bluetooth controller app for Arduino, ESP32.

* [Dabble - Bluetooth Controller for iOS](https://apps.apple.com/us/app/dabble-bluetooth-controller/id1472734455) 

![Remote control](Dabble_control.jpg)


![Robot](NX13A_01.jpg)
