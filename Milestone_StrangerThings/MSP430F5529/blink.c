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





