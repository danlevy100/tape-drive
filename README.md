# WIS lab smart tape drive
## Introduction
The smart tape drive is used for laser-plasma ion acceleration from thin foils. It was conceived during my PhD.
The tape refreshes the foil after each shot, thus allowing for operation at the repetition rate of the laser.

The "smart" part involves two closed feedback control loops:
1. Tape tension, for maintaining precise value such that the tape does not break or bend.
2. Tape position with respect to the laser focus, so that conditions are constant when the tape is rolling.

To the best of my knowledge, these two features are unique and differentiate this tape from other similar tapes.

## Implementation
### Hardware
The tape tension is maintained by two stepper motors and a spring. To increase the tension, the motors turn in such a way that the spring compresses and pushes on the sliding block. Once the desired tension (or block position) is reached, the motors stop and resist any further motion. The block position is monitored by a linear optical encoder.

- Motors: Arun microelectronics (AML) D42.1 UHV Stepper Motor, 180mNm, Radiation hardened.
- Linear optical encoder: Celera Motion Mercury 1500V.
- The tape itself: $5\,\rm\mu m$ stainless steel, 1.27 inch wide, 50 m long. Sold by Maton Metals (Israel).
- 



### Software



## Credits
