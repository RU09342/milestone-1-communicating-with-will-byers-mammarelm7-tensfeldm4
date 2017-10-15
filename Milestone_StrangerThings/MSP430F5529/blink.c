/*
Matt Mammarelli
9/18/17
ECE 09342-2
*/

//MSP430F5529 Milestone
//Takes a string of bits over UART, parses it for the current and next rgb values, then transmits the next board rgb
//When incoming is 56 or greater bytes the txd will only output 48 bytes, need to reset after first transmit


#include <msp430f5529.h>

int byteCount=0; //the current byte that is iterating through the packet
int numBytes=0; //number of bytes in packet
int red,green,blue=0; //holds UART values for current rgb


void main(void)
{

  //stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  //uart **************************************************************************************************
  // P3.3, P3.4 transmit/receive
  P3SEL = BIT3+BIT4;
  // Put state machine in reset
  UCA0CTL1 |= UCSWRST;
  // SMCLK
  UCA0CTL1 |= UCSSEL_2;
  // 1MHz 9600 baud
  UCA0BR0 = 6;
  // 1MHz 9600
  UCA0BR1 = 0;
  //sets m control register
  UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16;
  //sets control register
  UCA0CTL1 &= ~UCSWRST;
  //enable interrupt
  UCA0IE |= UCRXIE;
  //*******************************************************************************************************


  //rgb pwm *****************************************************************************************

  // P1.2 , P1.3, P1.4 output
  P1DIR |= BIT2+BIT3+BIT4;

  // P1.2 and P1.3, P1.4 options select GPIO
  P1SEL |= BIT2+BIT3+BIT4;

  // PWM Period about 1khz
  TA0CCR0 = 1024;

  // CCR1 reset/set
  TA0CCTL1 = OUTMOD_7;

  // CCR2 reset/set
  TA0CCTL2 = OUTMOD_7;

  // CCR3 reset/set
  TA0CCTL3 = OUTMOD_7;

  // SMCLK, up mode, clear TAR
  TA0CTL = TASSEL_2 + MC_1 + TACLR;

  //***************************************************************************************************

  // Low power mode
  __bis_SR_register(LPM0_bits + GIE);

  // For debugger
  __no_operation();
}




//uart interrupt vector
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;   // Vector 0 - no interrupt
  case 2:{
      while (!(UCA0IFG&UCTXIFG));  // USCI_A0 TX buffer check
          if(byteCount==0){
                          numBytes = UCA0RXBUF;
                          byteCount++;


                      }
                      //current rgb
                      else if ((byteCount>0 & byteCount <4)){
                          switch(byteCount){
                          case 1:{
                              red = UCA0RXBUF;
                              // CCR1 PWM duty cycle red
                              TA0CCR1 = red * 4;
                              break;
                          }
                          case 2:{
                              green = UCA0RXBUF;
                              // CCR2 PWM duty cycle green
                              TA0CCR2 = green * 4;
                              break;
                          }
                          case 3:{
                              blue = UCA0RXBUF;
                              // CCR3 PWM duty cycle blue
                              TA0CCR3 = blue * 4;
                              //beginning of new transmit message
                              UCA0TXBUF = numBytes-3;
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
                              byteCount=0;
                          }

                      }

          break;

  }

  case 4:break;    // Vector 4 - TXIFG
  default: break;
  }
}





