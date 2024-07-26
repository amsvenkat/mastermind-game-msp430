/***************************************************************************/
/**
 * @file                    main.c
 * @author                  Amrutha Venkatesan
 * @date                    11th Feb 2022
 * @matriculation number    5169728
 * @e-mail contact          amsvenkat20@gmail.com
 *
 * @brief   Exercise 10
 * This code implements a MasterMind game on two boards.
 * Player 1 and Player 2 can be chosen by using the define functions.
 * Player 1 sets the code and Player 2 has to guess the code.
 * Code is build in terms of the pushbuttons 1,2,3. Correct order has to be identified
 * within a given number of rounds.(Given the current complexity of the game - 3 rounds )
 *
 * Player 1 sends feedback to the Player 2 after comparing the guess and the original code.
 *
 * Feedback is in both
 *  - audio - through PWM Buzzer output and
 *  - visual - through the D1,D2 LEDs - corresponding to the correct sequence
 * Feedback contains only two positions - 1,2 of the code. This increases the complexity of finding the code
 *
 *
 * Player 2 guesses till the end of number of rounds.
 * Game finishes when the number of rounds are completed. New Game is started thereafter.
 *
 * Watchdog timer helps in Timeout if no response or software stuck up issues occur.
 *
 * In addition - Hterm is used to communicate and show the progress of the play
 *
 * Pin connections -
 *
 * Joining two boards
 * GNDs are connected to each other
 * P1.6, P1.6, P1,5, P1.7, P1.0, P1.3 - Data Communication between both the boards
 * Used as interrupts
 *
 * Individual Boards -
 * P3.6 - Connected to Buzzer to give audio feedback
 * P2.2 - Connected to PB5 to start the program when pressed. - Initiliaze the boards
 *
 *
 * Tasks completed: All tasks have been implemented.
 *
 * @note    The project was exported using CCS 12.1.0
 ******************************************************************************/

#include <templateEMP.h>
#define PLAYER2
#ifdef PLAYER2

int order[4];
int flag;
int end_word = 0;
int feedback[2] = { 0, 0 };
int start = 0;
int rc = 0;
int timer_end = 0;

void clockReset()
{
    P2OUT |= BIT4;
    P2OUT &= ~BIT4;
}
void shiftClear()
{
    P2OUT &= ~BIT5;
    P2OUT |= BIT5;
}
void shiftModeRight()
{
    P2OUT |= BIT0;
    P2OUT &= ~BIT1;
}
void initializePWMOutput()
{
    P3DIR |= BIT6;
    P3SEL |= BIT6;
    P3REN &= ~BIT6;
}
void pinsAsInput()
{
    // Setting all the pins as input with interrupt
    //P1OUT &= ~(BIT7 | BIT0);
    P1DIR &= ~(BIT7 | BIT0);
    P1REN |= (BIT7 | BIT0);
    P1OUT &= ~(BIT7 | BIT0);
    // Low to High Edge
    P1IES &= ~(BIT7 | BIT0);
    P1IFG &= ~(BIT7 | BIT0);
}
void pinsAsOutput()
{
    // Setting all the pins as input with interrupt
    // BIT 3 for the end signal
    P1OUT &= ~( BIT6 | BIT5 | BIT4);
    P1DIR |= (BIT6 | BIT5 | BIT4);
    // P1REN &= ~(BIT6 | BIT5 | BIT4);

}
void settingShiftRegisters()
{
    // Setting directions
    P2DIR |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6);
    P2DIR &= ~BIT7;
    // XTAL pins made to I/O
    P2SEL &= ~(BIT6 | BIT7);
    // Resetting Shift registers
    P2OUT &= ~BIT5;
    P2OUT |= BIT5;
    P2OUT &= ~BIT4;
}
void setVisual()
{

    int i;
    shiftClear();
    shiftModeRight();
    // LED on as per feedback
    for (i = 1; i >= 0; i--)
    {
        if (feedback[i] == 1)
            P2OUT |= BIT6;
        else
            P2OUT &= ~BIT6;
        clockReset();
    }

}
void playAudio(int c)
{

    initializePWMOutput();
    TA0CCTL2 = OUTMOD_3;
    TA0CTL = TASSEL_2 + MC_1;
    int data_win[2] = { 440, 523 };

    int data_lose[2] = { 440, 440 };

    int i;
    if (c == 1)
    {
        for (i = 0; i < 2; i++)
        {
            TA0CCR0 = data_win[i];
            TA0CCR2 = data_win[i] / 2;
            _delay_cycles(400000);
        }
    }
    else
    {
        for (i = 0; i < 2; i++)
        {
            TA0CCR0 = data_lose[i];
            TA0CCR2 = data_lose[i] / 2;
            _delay_cycles(400000);
        }
    }

    TA0CTL = MC_0;

}
int pushButtonsClicked()
{
    // Shift register 1
    P2OUT |= (BIT2 | BIT3);
    // Shift register 2
    P2OUT &= ~(BIT0 | BIT1);
    clockReset();
    if (P2IN & BIT7)
    {
        flag = 4;
        return 4;
    }
    else
    {
        int i;
        P2OUT |= BIT2;
        P2OUT &= ~BIT3;
        for (i = 3; i >= 1; i--)
        {
            clockReset();
            if (P2IN & BIT7)
            {
                flag = i;
                return i;
            }
        }
    }
    return 99;
}
void setPin()
{
    switch (flag)
    {
    // when the case is 1 - PB1 then the pin 4 is activated
    case 1:
        P1OUT |= BIT4;
        break;
    case 2:
        P1OUT |= BIT5;
        break;
    case 3:
        P1OUT |= BIT6;
        break;
    case 4:
        P1OUT |= BIT7;
        break;
    }
}
void getGuess()
{
    int i = 0;

    while (i < 3)
    {
        serialPrintln(" Press button ");
        serialPrintInt(i + 1);
        while (pushButtonsClicked() == 99)
            ;
        order[i] = flag;
        setPin();
        _delay_cycles(250000);
        i++;
    }

    P1OUT &= ~(BIT6 | BIT5 | BIT4);
}
void initFeedback()
{
    int i;
    for (i = 0; i < 2; i++)
        feedback[i] = 0;
    //serialPrint(" FeedBack initialization ");
    rc = 0;
    P1IE |= (BIT7 | BIT0);

}
void enableStartButton()
{
    //Pushbutton 5
    P3DIR &= ~BIT4;
    P3REN |= BIT4;
    P3OUT |= BIT4;

}
void startTimer(int duration)
{
    // ACLK from VLO - 12KHz
    BCSCTL3 |= LFXT1S_2;
    // Clock at 1500Hz
    BCSCTL1 |= DIVA_3;
    TA1CTL |= TACLR;
    TA1CTL = MC_1 + TASSEL_1;
    TA1CCTL0 |= OUTMOD_1 + CCIE;
    TA1CCTL0 &= ~CCIFG;
    TA1CCR0 = 1500 * duration;
    timer_end = 0;
}
void stopTimer()
{
    TA1CTL = MC_0;  // Stop timer
}
void otherPorts()
{
    P1OUT &= ~(BIT3);
    P1DIR &= ~BIT3;
}
int checkWinLose()
{
    int i;
    for (i = 0; i < 2; i++)
    {
        if (feedback[i] != 1)
            return 0;

    }
    return 1;
}

void main(void)
{
    initMSP();
    BCSCTL3 |= LFXT1S_2;
    BCSCTL1 |= DIVA_3;
    WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

    settingShiftRegisters();
    enableStartButton();
    otherPorts();

    serialPrintln(" Press PB5 to Start Game ");
    int check;
    pinsAsOutput();
    pinsAsInput();

    while (1)
    {
        if (!(P3IN & BIT4))
        {
            _delay_cycles(200000);
            start = 1;
            WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;
        }

        if (start == 1)
        {
            serialPrintln(" New Game Started ");
            pinsAsOutput();
            pinsAsInput();
            int rounds = 1;
            WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

            while (rounds <= 3)
            {
                rc = 0;
                serialPrint(" Round ");
                serialPrintInt(rounds);
                P1REN |= BIT3;
                P1OUT &= ~BIT3;
                P1IES &= ~BIT3;
                P1IFG |= BIT3;
                P1IE |= BIT3;

                while (rc == 0) // (!(P1IN & BIT3))
                {
                    //serialPrint("wait");
                };

                if (rc != 0) //((P1IN & BIT3))
                {
                    _delay_cycles(250000);
                    serialPrintln(" Guess ");
                    getGuess();
                }

                WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

//                while (rc==1) //((P1IN & BIT3))
//                    ;
                initFeedback();

                while (rc < 2) //!(P1IN & BIT3))
                {
                    //serialPrint("wait");
                };
                // rc =2
                startTimer(1);
                while (timer_end == 0)
                    ;

                serialPrintln(" Feedback Done ");
                WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

                int i;
                for (i = 0; i < 2; i++)
                    serialPrintInt(feedback[i]);

                setVisual();

                WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

                check = checkWinLose();
                if (check == 1)
                {
                    serialPrintln("You Win !!! Congratulations! Start new Game? ");
                    playAudio(1);
                    break;
                }
                else if (rounds != 3)
                {
                    serialPrintln(" Not correct! Try Again ");
                    playAudio(0);
                }
                else
                {
                    serialPrintln(" You Lose! Start new Game? ");
                    playAudio(0);
                }
                WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;
                rounds++;
                startTimer(3);
                while (timer_end == 0)
                    ;
            }
        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    //rc++;
   //serialPrint("interrupt");
    _delay_cycles(20000);
    if (P1IFG & BIT0)
    {
        feedback[0] = 1;
        P1IFG &= ~BIT0;
    }
    // 2nd position
    if (P1IFG & BIT7)
    {
        feedback[1] = 1;
        P1IFG &= ~BIT7;
    }
    if (P1IFG & BIT3)
    {
        rc++;
        P1IFG &= ~BIT3;
        if (rc > 2)
            rc = 0;
    }

}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer(void)
{

    stopTimer();
    timer_end = 1;
    TA1CCTL0 &= ~(CCIFG | CCIE);
}
#endif

//#define PLAYER1
#ifndef PLAYER2
int order[2];
int result[2];
int code[3] = { 1, 2, 3 };
unsigned int pos = 0;
int flag = 0;
int start = 0;
int duration;
int timer_end = 0;
int rc;

void initializePWMOutput()
{
    P3DIR |= BIT6;
    P3SEL |= BIT6;
    P3REN &= ~BIT6;
}
void clockReset()
{
    P2OUT |= BIT4;
    P2OUT &= ~BIT4;
}
void shiftClear()
{
    P2OUT &= ~BIT5;
    P2OUT |= BIT5;
}
void shiftModeRight()
{
    P2OUT |= BIT0;
    P2OUT &= ~BIT1;
}
void settingShiftRegisters()
{
    // Setting directions
    P2DIR |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6);
    P2DIR &= ~BIT7;
    // XTAL pins made to I/O
    P2SEL &= ~(BIT6 | BIT7);

    // Resetting Shift registers
    P2OUT &= ~BIT5;
    P2OUT |= BIT5;

    P2OUT &= ~BIT4;

    P1DIR |= (BIT7 | BIT6 | BIT5);
    P1OUT &= ~(BIT7 | BIT6 | BIT5);
}
void generateSequence()
{
    int i;
    for (i = 0; i < 3; i++)
    {
        int j = rand() % 3;
        int temp = code[i];
        code[i] = code[j];
        code[j] = temp;
    }
    serialPrintln(" Code ");
    for (i = 0; i < 3; i++)
    {
        serialPrintInt(code[i]);
    }
}
void pinsAsInput()
{
    // Setting all the pins as input with interrupt
    P1OUT &= ~(BIT6 | BIT5 | BIT4);
    P1DIR &= ~(BIT6 | BIT5 | BIT4);
    P1REN |= (BIT6 | BIT5 | BIT4);
    P1OUT &= ~( BIT6 | BIT5 | BIT4);
    // Low to High Edge
    P1IES &= ~( BIT6 | BIT5 | BIT4);
    P1IFG &= ~(BIT6 | BIT5 | BIT4);
}
void pinsAsOutput()
{
    // Setting all the pins as input with interrupt
    // BIT 3 for the end signal
    P1OUT &= ~(BIT7 | BIT0);
    P1DIR |= (BIT7 | BIT0);
    //P1REN &= ~(BIT7 BIT0);
}
void getResponse()
{
    pinsAsInput();
    pos = 0;
    serialPrintln(" Waiting for response ");
    P1IE |= (BIT6 | BIT5 | BIT4);
}
void compareCodes()
{
    int i;
    for (i = 0; i < 4; i++)
    {
        serialPrintInt(order[i]);
        serialPrintInt(code[i]);
        if (order[i] == code[i])
        {
            result[i] = 1;
        }
        else
        {
            result[i] = 0;
        }
    }
}
void setPin()
{
    switch (flag)
    {
    // when the case is 1 -result in 1st pos is correct
    case 1:
        P1OUT |= BIT0;
        break;
    case 2:
        P1OUT |= BIT7;
        break;
    }

}
void sendResult()
{

    int i;
    for (i = 0; i < 2; i++)
    {
        if (result[i] == 1)
        {
            flag = i + 1;
            setPin();
            //_delay_cycles(250000);
        }
    }
    //flag = 5;
    //setPin();
    //pinsAsInput();
}
void setVisual()
{

    int i;
    shiftClear();
    shiftModeRight();
    // LED on as per feedback
    for (i = 1; i >= 0; i--)
    {
        if (result[i] == 1)
            P2OUT |= BIT6;
        else
            P2OUT &= ~BIT6;
        clockReset();
    }

}
void enableStartButton()
{
    //Pushbutton 5
    P3DIR &= ~BIT4;
    P3REN |= BIT4;
    P3OUT |= BIT4;

}
void startTimer(int duration)
{
    // Set mode for 0.5sec - ACLK, divider -8, mode - up
    // ACLK from VLO - 12KHz
    BCSCTL3 |= LFXT1S_2;
    // Clock at 1500Hz
    BCSCTL1 |= DIVA_3;

    TA1CTL |= TACLR;

    TA1CTL = MC_1 + TASSEL_1;

    TA1CCTL0 |= OUTMOD_1 + CCIE;
    TA1CCTL0 &= ~CCIFG;
    TA1CCR0 = 1500 * duration;
    timer_end = 0;
}
void stopTimer()
{
    TA1CTL = MC_0;  // Stop timer
}
void playAudio(int c)
{

    initializePWMOutput();
    TA0CCTL2 = OUTMOD_3;
    TA0CTL = TASSEL_2 + MC_1;
    int data_win[2] = { 440, 523 };

    int data_lose[2] = { 440, 440 };

    int i;
    if (c == 1)
    {
        for (i = 0; i < 2; i++)
        {
            TA0CCR0 = data_win[i];
            TA0CCR2 = data_win[i] / 2;
            _delay_cycles(400000);
        }
    }
    else
    {
        for (i = 0; i < 2; i++)
        {
            TA0CCR0 = data_lose[i];
            TA0CCR2 = data_lose[i] / 2;
            _delay_cycles(400000);
        }
    }

    TA0CTL = MC_0;

}
int checkWinLose()
{
    int i;
    for (i = 0; i < 2; i++)
    {
        if (result[i] != 1)
            return 0;

    }
    return 1;
}
void otherPorts()
{
    P1OUT &= ~(BIT3);
    P1DIR |= BIT3;
}
void main(void)
{
    initMSP();
    BCSCTL3 |= LFXT1S_2;
    BCSCTL1 |= DIVA_3;
    WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;
    settingShiftRegisters();
    otherPorts();
    enableStartButton();

    int check;
    serialPrintln(" Press PB5 to Start Game ");
    pinsAsOutput();
    pinsAsInput();
    while (1)
    {

        if (!(P3IN & BIT4))
        {
            _delay_cycles(20000);
            start = 1;
        }

        if (start == 1)
        {

            serialPrintln(" New Game Started ");
            pinsAsOutput();
            pinsAsInput();
            otherPorts();
            int rounds = 1;
            generateSequence();
            WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

            while (rounds <= 3)
            {
                serialPrint(" Round ");
                serialPrintInt(rounds);

                getResponse();
                // rc =1
                P1OUT |= BIT3;
                P1OUT &= ~BIT3;

                while (pos != 3)
                {
                }

                WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

                startTimer(2);
                while (timer_end == 0)
                    ;
                compareCodes();
                //serialPrint(" Result ");

                int i;
                for (i = 0; i < 2; i++)
                    //serialPrintInt(result[i]);

                startTimer(1);
                while (timer_end == 0)
                    ;

                sendResult();
                // rc =2
                P1OUT |= BIT3;
                P1OUT &= ~BIT3;

                startTimer(1);
                while (timer_end == 0)
                    ;

                //serialPrint("Result sent");
                setVisual();

                WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

                check = checkWinLose();
                if (check == 1)
                {
                    serialPrintln(" You Win !!! Congratulations! Start new Game?  ");
                    playAudio(1);
                    break;
                }
                else if (rounds != 3)
                {
                    serialPrintln(" Not correct! Try Again! ");
                    playAudio(0);
                }
                else
                {
                    serialPrintln(" You Lose! Start new Game? ");
                    playAudio(0);
                }
                WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;

                rounds++;
                startTimer(8);
                while (timer_end == 0)
                    ;

            }
            unsigned int i;
            for (i = 0; i < 3; i++)
                code[i] = 3 - i;
        }

    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    //serialPrint("Interrupt");
    _delay_cycles(20000);
    if (P1IFG & BIT4)
    {
        order[pos] = 1;
        pos++;
        P1IFG &= ~BIT4;

    }

    if (P1IFG & BIT5)
    {
        order[pos] = 2;
        pos++;
        P1IFG &= ~BIT5;
    }

    if (P1IFG & BIT6)
    {
        order[pos] = 3;
        pos++;
        P1IFG &= ~BIT6;
    }

    //serialPrintInt(pos);

    WDTCTL = WDTPW + WDTSSEL + WDTCNTCL;
}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer(void)
{

    stopTimer();
    timer_end = 1;
    TA1CCTL0 &= ~(CCIFG | CCIE);
}
#endif

