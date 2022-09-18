# WIS lab smart tape drive
## Introduction
The smart tape drive is used for laser-plasma ion acceleration from thin foils. It was conceived during my PhD.
The tape refreshes the foil after each shot, thus allowing for operation at the repetition rate of the laser.

![image](https://user-images.githubusercontent.com/77229620/190892208-cdf41c67-2c24-4098-807b-cfe0f13edb1f.png)


The "smart" part involves two closed feedback control loops:
1. Tape tension, for maintaining precise value such that the tape does not break or bend.
2. Tape positioning with respect to the laser focus, so that conditions are constant when the tape is rolling.

To the best of my knowledge, these two features are unique and differentiate this tape from other similar tapes.

![image](https://user-images.githubusercontent.com/77229620/190892389-40d65123-9292-4f73-841d-5dab0cf151e5.png)

## Implementation
### Tape tension
#### Hardware
The tape tension is maintained by two stepper motors and a spring. To increase the tension, the motors turn in such a way that the spring compresses and pushes on the sliding block. Once the desired tension (or block position) is reached, the motors stop and resist any further motion. The block position is monitored by a linear optical encoder.

- Motors: Arun microelectronics (AML) D42.1 UHV Stepper Motor, 180mNm, Radiation hardened.
- Linear optical encoder: Celera Motion Mercury 1500V.
- The tape itself: 5 $\rm\mu m$ stainless steel, 12.7 mm wide, 50 m long. Sold by Maton Metals (Israel). Testing was done with a standard VHS tape.

#### Software
- The motors are controlled by an Arduino connected to a stepper motor driver.
- A Python GUI communicates with the Arduino over the serial port.

### Tape positioning
- A UHV piezo stage (Newport AG-LS25V6) is mounted such that it moves the tape into and out of focus.
- A laser distance measuring device (Acuity AR700-6) sits outside the chamber and measures the distance to the tape.
- The tape is moved accordingly.

## Acknowledgements
LOA: Thomas Lavergne, Alessandro Flacco.
WIS: Noam Inbari, Alex Roich.
