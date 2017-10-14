/*
Matt Mammarelli
9/18/17
ECE 09342-2
*/

//MSP430FR5994 Software PWM
//green led toggles when button pressed
//red led affected by PWM
//need p1sel0
//not done


#include <msp430.h>

void ledSetup();

void timerSetup();

int redDuty =100;
int greenDuty = 500;
int blueDuty= 900;


void main(void){

    // Stop watchdog timer
    WDTCTL = WDTPW + WDTHOLD;

    // Disable the GPIO power-on default high-impedance mode
        // to activate previously configured port settings
        //PM5CTL0 = Power mode 5 control register 0
        //LOCKLPM5 = Locks I/O pin, bit is reset by a power cycle
        //~LOCKLPM5=8'b0 and by anding PM5CTL0 it clears the register
        PM5CTL0 &= ~LOCKLPM5;

    ledSetup(); //init leds

    timerSetup(); //init timer

    _BIS_SR(LPM0_bits + GIE);       //Enter low power mode

}




void ledSetup(){

    //selects gpio mode for both leds
        P3SEL0 &= ~(BIT4|BIT5|BIT6);


        //red,green led out
        P3DIR |= (BIT4|BIT5|BIT6);


        //makes sure all colors are off
        P3OUT &= ~(BIT4|BIT5|BIT6);



}

void timerSetup(){

    // SMCLK, Up Mode (Counts to TA0CCR0)
        TB0CTL |= TBSSEL_2 + MC_1;


        //sets cctl1 and 0 to compares ccr0 to ccr4
        TB0CCTL0 = (CCIE);
        TB0CCTL4 = (CCIE);
        //TB0CCTL5 = (CCIE);
        //TB0CCTL6 = (CCIE);

        // PWM period, f=1MHz/1001 = 1khz
        TB0CCR0 = 1000;


        //TB0CCR4 = 100; //pin P3.5 blue
        //TB0CCR5 = 0; //pin P3.6 green
        //TB0CCR6 = 900; //pin P3.7 red


}



#pragma vector=TIMER0_B1_VECTOR
__interrupt void Timer0_B1_ISR (void)
{



    if(TB0CCR4 != 1000 || TB0CCR5 != 1000 || TB0CCR6 != 1000)
    {
        P3OUT &= ~(BIT4); //turns off blue led
        P3OUT &= ~(BIT5); //turns off green led
        P3OUT &= ~(BIT6); //turns off red led
    }
        TB0CCTL4 &= ~BIT0; //clears flag
        TB0CCTL5 &= ~BIT0; //clears flag
        TB0CCTL6 &= ~BIT0; //clears flag
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR (void)
{
    P3OUT |= (BIT4); //turns on blue led
    P3OUT |= (BIT5); //turns on green led
    P3OUT |= (BIT6); //turns on red led

    TB0CCTL0 &= ~BIT0;  //clears flag
}

