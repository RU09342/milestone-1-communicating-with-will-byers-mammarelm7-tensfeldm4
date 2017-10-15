# Milestone 1: Communicating with Will Byers MSP430F5529
Thie objective is to control the color of an RGB LED through the use of UART and three individual PWM signals.
Multiple MSP430 boards can be connected together in any order using UART and the code should still work.

## Why MSP430F5529?
All of the board were considered but this one came out on top because of the easy and reliable implementation for hardware PWM along with reliable UART communication using the usb serial cable.

## Problems with other boards
### MSP430FR2311 
Only has Timer B, does not have reliable PWM, only works with UART USB backchannel
### MSP430FR5994
Doesn't reliably work with hardware PWM. Doesn't reliably work with UART serial cable
### MSP430FR6989 
Doesn't work at all with either UART USB backchannel or serial cable
### MSP430G2553 
Doesn't have enough CCR registers for 3 PWM

## Circuit
The RGB LED was common anode so a circuit needed to be created to support this. 

![alt text](MSP430F5529/images/commonAnode.png "Common Anode")


## PWM 
Hardware PWM was utilized for the MSP430F5529 for simplicity since this board supports up to seven PWM outputs. 
We need three PWM signals, one for each color of the led. The common anode means that the longest leg of the led
must be hooked up to VCC which I chose to be 5V. Each leg then is connected to a 1K resistor to limit the current running through the nMOS transistors.
The nMOS transistors are used as low side switches. This means that the gate is connected to a GPIO pin on the microcontroller which controls whether the led is on or not.
1K pulldown resistors are needed between the gate and ground for each of these transistors so that they aren't floating.
The headers on the bottom of the board were used to breakout the pins and power to a breadboard. 

### Red control: P1.2
### Green control: P1.3
### Blue control: P1.4

![alt text](MSP430F5529/images/circuitSchem.png "Circuit Schematic")

![alt text](MSP430F5529/images/circuitLedOFF.JPG "Circuit OFF")

## UART
The UART structure was standardized so that each node in the chain will parse through 8 bytes worth of data to control the PWM on that board as well as what data to send to the next.
Using a serial to usb cable, the green was connected to P3.4 and white connected to P3.3. A packet with RGB values will be sent to the next board in the chain while the values that were used for the current board were removed.

### Byte 0: Total bytes in package 0x08
### Byte 1: Red duty cycle current board
### Byte 2:	Green duty cycle current board
### Byte 3: Blue duty cycle current board
### Byte 4: Red duty cycle next board
### Byte 5: Green duty cycle next board
### Byte 6: Blue duty cycle next board
### Byte 7: End of message 0X0D


## UART Testing
The UART was tested using Realterm where an 8 byte string was sent to the board to turn the led a purple color. 
The Realterm displayed the data that will be transmitted to the next board which are rgb values encapsulated in a new 5 byte package. 

### Sent: 0x08 0x7D 0x00 0xFE 0x41 0xFA 0x00 0x0D

![alt text](MSP430F5529/images/realterm.png "Realterm")

![alt text](MSP430F5529/images/circuitLedON.JPG "Circuit ON")


## Code
The code was broken up into a UART block and a PWM block.

```c
/*
Matt Mammarelli
9/18/17
ECE 09342-2
*/

//MSP430F5529 Milestone
//When incoming is 56 or greater bytes the txd will only output 48 bytes

#include <msp430f5529.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int byteCount=0;
int numBytes=0;
int red,green,blue=0;


void main(void)
{

  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  //uart
  P3SEL = BIT3+BIT4;                        // P3.4,5 = USCI_A0 TXD/RXD
  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 6;                              // 1MHz 9600 (see User's Guide)
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Modln UCBRSx=0, UCBRFx=0,
                                            // over sampling
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt


  //rgb pwm
  P1DIR |= BIT2+BIT3+BIT4;                       // P1.2 , P1.3, P1.4 output
  P1SEL |= BIT2+BIT3+BIT4;                       // P1.2 and P1.3, P1.4 options select
  TA0CCR0 = 1024;                          // PWM Period ~~ 1khz
  TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set

  TA0CCTL2 = OUTMOD_7;                      // CCR2 reset/set

  TA0CCTL3 = OUTMOD_7;

  TA0CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, up mode, clear TAR

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
  __no_operation();                         // For debugger
}




// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:{
      while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
          if(byteCount==0){
                          numBytes = UCA0RXBUF;
                          byteCount++;
                          //UCA0TXBUF = 0xFF;

                      }
                      //current rgb
                      else if ((byteCount>0 & byteCount <4)){
                          switch(byteCount){
                          case 1:{
                              red = UCA0RXBUF;
                              TA0CCR1 = red * 4;                            // CCR1 PWM duty cycle red
                              break;
                          }
                          case 2:{
                              green = UCA0RXBUF;
                              TA0CCR2 = green * 4;                            // CCR2 PWM duty cycle green
                              break;
                          }
                          case 3:{
                              blue = UCA0RXBUF;
                              TA0CCR3 = blue * 4;                            //blue
                              UCA0TXBUF = numBytes-3; //beginning of new message
                              break;
                          }
                          default:break;


                          }

                          byteCount++;

                      }
                      //sending rgb and rest of message
                      else if (byteCount>3 & byteCount <= numBytes-1){
                          if (byteCount!=numBytes-1){
                              UCA0TXBUF = UCA0RXBUF;
                              byteCount++;
                          }
                          else{
                              UCA0TXBUF = 0x0D; //end of new message
                              __delay_cycles(100000);
                              byteCount=0;
                          }

                      }


          //UCA0TXBUF = message[1];
          //UCA0TXBUF = UCA0RXBUF+1;                  // TX -> RXed character
          break;
      // Vector 2 - RXIFG
  }

  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}


```




  


