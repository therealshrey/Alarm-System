

/*******************************
 * Name:Shrey Aryal 
 * Student ID#:1001232716
 * Lab Day:
 * CSE 3442/5442 - Embedded Systems 1
 * Lab 7 (ABET): Building a PIC18F4520 Standalone Alarm System with EUSART Communication 
 ********************************/

 /**
 NOTE: 	
	*Your comments need to be extremely detailed and as professional as possible
	*Your code structure must be well-organized and efficient
	*Use meaningful naming conventions for variables, functions, etc.
	*Your code must be cleaned upon submission (no commented out sections of old instructions not used in the final system)
	*Your comments and structure need to be detailed enough so that someone with a basic 
            embedded programming background could take this file and know exactly what is going on
 **/
 

#define _XTAL_FREQ 20000000   //Frequency of the external Oscillator (20 MHz)
#include <xc.h>
#include <pic18f4520.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



// PIC18F4520 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1H
#pragma config OSC = HS    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON      // MCLR Pin Enable bit (RE3 input pin enabled; MCLR enabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

//other includes

//Configuration bits settings code goes here

/*******************************
 * Function prototypes
 ********************************/


void my_print(char *message);
void halfSecDelay();
void initialScreen(void);
char reception();
char * getPassword();
void setUpPassword();
void existingUserLogin();
void mainScreen();
void verifyPassword();
void option_passcode();
void option_PIR();
void option_TempSensor();
void option_Keyboard();
void option_Keypad();
void option_KeyboardKeypad();
void clearScreen();
int changePassword();
void FindCurrentTemp();
void choiceKeypad();
void updateThreshold();
void initialize_serial(void);
void initialize_timer(void);
void initialize_PIR(void);
void initialize_tempSensor(void);
void initial(void);
void interrupt low_priority My_ISR_Low_TempSensor(void);
void My_ISR_High_MotionSensor(void);
int PIR_reset();
void write_EEPROM(int,char);
char read_EEPROM(int);
void checkStatus();
int temp_reset();


//Global Variables
volatile unsigned char pass[4];
static char tempPass[4];
int static flag = 0;
int static inReset = 0;
int static correctPassword = 1;
unsigned int addr;
unsigned int PIRStatusAddr = 0xE0; //Address on EEPROM to save PIR Sensor Status
unsigned int tempStatusAddr = 0xE1;//Address on EEPROM to save Temperature Sensor Status
unsigned int inputTypeAddr = 0xE3;//Address on EEPROM to save input type
unsigned int thresholdAddr = 0xD0;//Address on EEPROM to save threshold temperature
char keyVal;
char choice;
double temperature;
unsigned int temp1;
unsigned static int tempThreshold;
unsigned int check;
volatile unsigned char thresholdArray[3];
char tempArray[6];
static int tempIntSet = 0;
unsigned char *statusOfPir = "INACTIVE";
unsigned char *statusOfTemp = "INACTIVE";
unsigned char *inputMethod = "KEYBOARD";


void main(void)
{
    tempIntSet = 0;
    TRISBbits.RB4 = 0;      //Making PORTB pin 4 an output
    PORTBbits.RB4 = 0;      //Setting the value of pin4 PORTB to 0
    initialize_PIR();       //Calling function for initialing settings for PIR
                            //motion sensor
    ADCON1 = 0b00001110;    // ANO is the only analog input setup
    TRISBbits.RB5 = 0;      // PORTB pin 4 as output
    initialize_serial();    //Calling function for initializing Serial Communication
    PIR1bits.ADIF = 0;      //Clear ADIF flag bit
    IPR1bits.ADIP = 0;      //ADC is low Priority
    PIE1bits.ADIE = 0;      //Set ADIE enable bit
    initialize_timer();     //Calling function to setting up timer
    initialScreen();        //Initial Screen Dispaly
    halfSecDelay();         //Half second delay
    halfSecDelay();         //Half second delay
    
    if(read_EEPROM(0x00) ==(char)255) //checking EEPROM to see if password has
                                       //been set or not. Calling function 
                                       //setUpPassword() if password has not been
                                       //set or calling existingUserLogin() function
                                       //if it has already been set. Saved Password is
                                       //written in address 0x00,0x01,0x02 and 0x03   
    {
         setUpPassword();    

    }
    else
    {
         existingUserLogin(); 
    }
    
    while(1);
  

} //end of void main()

/******************************************************************************
 *  void my_print()
 *
 *  Parameter: Pointer to a char array
 *
 *  Returns: No returns 
 *
 *  This is a custom print function for serial communication. It takes a pointer to a
 * char array and then transmits the message to the and finally prints the message.
 *  This function uses TXREG register where one 8-bit is transferred at time which is 
 * used to print the message to the screen using serial communication
 ******************************************************************************/
void my_print(char *message)
{
    RCSTAbits.CREN = 0; //Disabling Reception while transmission
    while(*message)     //While more character in the message array
    {
        while(TXSTAbits.TRMT == 0); //wait until transmission is complete
        TXREG = *message;           //Send one 8-bit byte
        message++;                  //Increment the index for sending another
                                    //8-bit
    }
    RCSTAbits.CREN = 1; //Enabling Reception once transmission completes
}

/******************************************************************************
 *  void reception()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 *  This function receives 8-bit at a time using serial communication. The received
 * 8-bit char value is saved in RCREG register and can be accessed from there and saved 
 * anywhere as as required.
 ******************************************************************************/
char reception()
{
    while(PIR1bits.RCIF==0);    //waiting for incoming data
    char val = RCREG;          //Reading RCREG which also clears RCIF
    return val;                //returns the received value in RCREG
}

/******************************************************************************
 *  void setUpPassword()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function provided the first time user of the Alarm System with a welcome 
 * screen and provides the functionality of setting up the password.
 ******************************************************************************/
void setUpPassword()
{
    my_print("\n\rWelcome User. This is the first time you have logged in");
    my_print("\r\nLets set up your Password");
    my_print("\r\nEnter 4 digit password: ");
    halfSecDelay();

    int i;
    addr = 0x00; //password saved in address 0x00, 0x01, 0x02 and 0x03 on EEPROM
        for(i=0; i<4; i++)
        {
            pass[i] = reception();           // calls reception function to get the character input and store it to option
            my_print("*");
            write_EEPROM(addr, pass[i]);    //Writting the received data to the EEPROM
            addr++;
        }
        while(RCREG!=13);               // checks if the user hit 'Enter'
    
    my_print("\n\rPassword Registered\n\r");    

    halfSecDelay();         
    initialScreen();    //Calling the function for displaying initial screen
    halfSecDelay();
    
    initialScreen();
    existingUserLogin(); //Once the password has been registered, Calling the 
                         //function to ask the user to login
}


/******************************************************************************
 *  void existingUserLogin()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function provides the registered user of the Alarm System with a screen
 * to login to the system
 ******************************************************************************/
void existingUserLogin()
{
    int i;
    my_print("\r\nEnter the password: ");   //Getting password from the user
                                            //one number at a time
    for(i = 0; i < 4; i ++)
    {
    tempPass[i] = reception();  //calling the reception function to receive user
                                //input
    my_print("*");              //Printing "*" in the screen for each charactered 
                                //typed by the user
    }
    
    while(RCREG!=13); //waiting for user to hit ENTER key
    verifyPassword();   //Calls the function that verifies if the password 
                        //provided by the user is correct.

}
/******************************************************************************
 *  void verifyPassword()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function verifies if the user provided password is valid or not. It compares
 * the password provided by the user with the password that has been saved on the
 * EEPROM during the registration process
 ******************************************************************************/
void verifyPassword()
{
    correctPassword = 1;    //variable to keep track if the password is correct
    int i;
    for(i = 0, addr = 0x00; i < 4;addr++,i++) //Iterating through the EEPROM address where
                                                //password is saved
    {
        pass[i] = read_EEPROM(addr);    //Reading the registered password from EEPROM and saving it
                                        //to the pass array   
        
        //if password provided by the user does not match the password saved on EEPROM
        //set correctPassword variable to 0. If it matches do noting i.e correctPassword 
        //value will remain 1
        if(pass[i] == tempPass[i])      
        {
            
        }
        else
        {
            correctPassword = 0;
            
        }
    }
    
    if(correctPassword == 1)    //Correct password is received from the user
                                //Ask the user to set up the threshold temperature
                                //for temperature sensor
    {
        int i;
        for(int i = 0; i < 2; i++)
        {
            //Receiving the threshold temperature from the EEPROM and saving it in 
            //thresholdArray
            thresholdArray[i] = read_EEPROM(thresholdAddr);
            thresholdAddr++;
        }
       tempThreshold = atoi(thresholdArray); //getting a integer value from an integer array
 
        checkStatus(); //Calls the checkStatus() function that checks the status of various components that 
                        //has been set on EEPROM
        
        PORTBbits.RB3 = 1; //Turning on the green LED to let the user know the Alarm system is up and running
        mainScreen();   //Calling the mainScreen() function
        
    }
    else if(correctPassword == 0) //If the User entered password is not correct
    {
        my_print("\n\rIncorrect Password\n\r");
        halfSecDelay();  
        halfSecDelay(); 
        existingUserLogin();    //Calling the exisitngUserLogin() function again 
    }

}

/******************************************************************************
 *  void initialize_timer()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * Function that initializes TIMER0 that is used for updating the temperature value
 * read from the Temperature sensor every 1 second
 ******************************************************************************/
void initialize_timer(void)
{
    INTCONbits.PEIE = 1; //Set PEIE enable bit
    INTCONbits.GIE = 1; //Set GIE enable bit
    INTCONbits.TMR0IF=0;//Clearing the TIMER0 Interrupt flag initiallt
    INTCONbits.TMR0IE=1;//Enabling TIMER0
    INTCON2bits.TMR0IP =0; //TMRO priority low
    T0CON = 0b00000110; //1:128 prescaler
    
    /* Preload Value Calculation
         * d = 1sec (time period)
         * Fosc = 20MHz
         * Fin = Fosc/4 = 5 MHz
         * 16-bit Timer: 0 - 65,536
         * X = d*Fin = 1s*5MHz = 5000000 i.e. 5000000 cycles/ticks occur in a 1s time span
         * Using prescaler to bring down X to fit into the 16-bit Timer register(0 - 65,536)
         * Using Prescaler of 1:128
         * Preload = 65,536 - 5000000/128
         *         = 65,536 - 39063
         *         = 26473
         * dec = 0x6769
         */

    
    TMR0H = 0x67;
    TMR0L = 0x69;
    
    PORTBbits.RB5 =0;   //Turning off the yellow LED(Temperature Sensor LED) initially.
}

/******************************************************************************
 *  void initialize_serial()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This fucntion initializes the serial communication between USART and PC.
 * TXSTA register is used for tranmission and RCSTA is used for reception
 ******************************************************************************/
void initialize_serial()
{
 /*
 Serial Communication Configuration
 */
SPBRG = 15;  //low Speed 19,200 BAUD for Fosc = 20 MHZ
TRISCbits.RC7 = 1; //RX is input
TRISCbits.RC6 = 0; //TX is output
TXSTAbits.SYNC = 0; //Async Mode
TXSTAbits.BRGH = 0; //Low Speed Baud Rate
RCSTAbits.RX9 = 0; //8-bit Reception
TXSTAbits.TX9 = 0; //8-bit Transmission
RCSTAbits.SPEN = 1; //Serial port Enabled (RX and TX active)
TXSTAbits.TXEN = 1; //Enabling Transmission
RCSTAbits.CREN = 1; //Enabling Reception
}

/******************************************************************************
 *  void initialize_PIR()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function initializes PIR motion sensor. RCON and INTCON register 
 * settings are set for using the motion sensor
 ******************************************************************************/
void initialize_PIR()
{

 //Interrupt Configuration for motion sensor
RCONbits.IPEN = 1; // enabling the high and low priority interrupt
INTCONbits.INT0IE = 0; //disable the INT0 interrupt
INTCON2bits.INTEDG0 = 0; //INT0 on falling edge
INTCONbits.INT0IF = 0;
TRISBbits.RB2 = 0; //PORTB pin 2 as an output to light up Red led
TRISBbits.RB0 = 1; //PORTB pin 0 as in input
PORTBbits.RB2 = 0;
//For green LED to show alarm system is running
TRISBbits.RB3 = 0; //set up as output
}

/******************************************************************************
 *  void initialize_tempSensor()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function initializes the temperature sensor. PIR and PIE register settings
 * are set for the temperature sensor
 ******************************************************************************/
void initialize_tempSensor()
{
    PIR1bits.ADIF = 0; //Clear ADIF flag bit
    PIE1bits.ADIE = 1; //Set ADIE enable bit
}

/******************************************************************************
 *  void clearScreen()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function clears the screen of the terminal where text are displayed
 ******************************************************************************/
void clearScreen()
{
  my_print("\033[2J"); //ascii code for clearing the screen of the terminal
    

}

/******************************************************************************
 *  void initialScreen()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function displays the Alarm System Header to the User
 ******************************************************************************/
void initialScreen()
{
    clearScreen(); 
    my_print("\n\r************************************************************");
    my_print("\n\r*                     Alarm System                         *");
    my_print("\n\r*                   Embedded Systems 1                     *");
    my_print("\n\r*                          Lab7                            *");
    my_print("\n\r*                       Shrey Aryal                        *");
    my_print("\n\r************************************************************");
            
}

/******************************************************************************
 *  void mainScreen()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function displays the Component Status header and the Alarm System 
 * functionality to the user.
 ******************************************************************************/
void mainScreen()
{
    do{
    initialScreen();
    tempIntSet = 0;
    //The following lines of code display the component status header
    my_print("\n\r");
    my_print("\n\rCURRENT STATUS OF COMPONENTS");
    my_print("\n\r************************************************************");
    my_print("\n\rTemperature Alarm State:         ");
    my_print(statusOfTemp);
    my_print("\n\rCurrent Temperature Reading:     ");
    char tempTemperatureArray[sizeof(temperature)];
    int i;
    for(i = 0; i < 6; i++)  //getting the current temperature from the temperature sensor
                            //and storing it in an array
    {
        if (i<5)
        {
            tempArray[i] = tempTemperatureArray[i];
        }
        else
        {
            tempArray[i] = '\0';        //adding \0 to the end of the array 
        }
    }
    
    sprintf(tempArray, "%2.2f", temperature); //Converting the temperature that is double 
                                                //to store it in char array
    my_print(tempArray);    
    while(TXSTAbits.TRMT == 0);
    TXREG = 167;                    //degree symbol
    while(TXSTAbits.TRMT == 0);
    TXREG = 'F';                     //degree symbol
    my_print("\n\rTemperature Alarm Threshold:     ");
    char tempThresholdArray[sizeof(tempThreshold)];
    sprintf(tempThresholdArray, "%d", tempThreshold);
    my_print(tempThresholdArray);
    while(TXSTAbits.TRMT == 0);
    TXREG = 167;
    while(TXSTAbits.TRMT == 0);
    TXREG = 'F';
    my_print("\n\rPIR Sensor Alarm State:          ");
    my_print(statusOfPir); 
    my_print("\n\rCurrent Input Method:            ");
    my_print(inputMethod);
    my_print("\n\r");
    my_print("\n\r************************************************************\n\r");
    

    //Following lines of code displays the alarm system options to the user
    my_print("\n\r*********            Main Menu                **************");
    my_print("\n\rSelect One of the Following:");
    my_print("\n\r      1.Passcode Options");
    my_print("\n\r      2.PIR Sensor Alarm Options");
    my_print("\n\r      3.Temperature Sensor Alarm Options");
    my_print("\n\r      4.Use Keyboard(Terminal) as the only input");
    my_print("\n\r      5.Use Keypad as the only input ('A' = Enter Key)");
    my_print("\n\n\n\r  0.Refresh Main Menu");
    
    my_print("\n\rInput: ");
    
   
    halfSecDelay();
    
        // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        choiceKeypad();                   // calling  choiceKeypad() function to get the key pressed
        while(TXSTAbits.TRMT == 0);     // waiting until possible previous transmission data is done
        TXREG = keyVal;            // sending one 8-bit byte
        choice = keyVal;           //saving the value to a char variable choice
        halfSecDelay();                 //Calling half second delay
        while(keyVal != 'A'){      //wait until user enters 'A'. 'A' is enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        choice = reception();           // calls reception function to get the character input and store it to choice
        while(TXSTAbits.TRMT == 0);     // wait until possible previous transmission data is done 
        TXREG = choice;                 // send one 8-bitbyte
        while(RCREG!=13);               // wait until user hits ENTER
    }

    flag = 0;
    
    //Switch statement to route to a correct option as per the user input
    switch(choice)
    {
        case '1':
            option_passcode();
            mainScreen();
            break;
        case '2':
            option_PIR();
            mainScreen();
            break;
        case '3':
            option_TempSensor();
            mainScreen();
            break;
         case '4':
            option_Keyboard();
            mainScreen();
            break;
        case '5':
            option_Keypad();
            mainScreen();
            break;
        case '0':
            mainScreen();
            break;
        default:
        {
            my_print("\n\rInvalid Selection.\n\r");
            halfSecDelay();
            halfSecDelay();
            flag = 1;
        }
    }
    
} while(flag == 1);
}

 /******************************************************************************
 *  void option_passcode()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function displays the user with the passcode options screen. User is able 
  * to change the passcode if they wish or can go back to the main screen
 ******************************************************************************/
void option_passcode()
{
    do
    {  
        //Display user the passcode option menu
    initialScreen();
    my_print("\n\r*********         Passcode Menu               **************");
    my_print("\n\rSelect One of the Following:");
    my_print("\n\r                 1.Change passcode");
    my_print("\n\r                 0.Return to Main Menu");
    
    my_print("\n\rInput: ");
    halfSecDelay();
    
         // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        choiceKeypad();                   // calling  choiceKeypad() function to get the key pressed
        while(TXSTAbits.TRMT == 0);     // waiting until possible previous transmission data is done
        TXREG = keyVal;            // sending one 8-bit byte
        choice = keyVal;           //saving the value to a char variable choice
        halfSecDelay();                 //Calling half second delay
        while(keyVal != 'A'){      //wait until user enters 'A'. 'A' is enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        choice = reception();           // calls reception function to get the character input and store it to choice
        while(TXSTAbits.TRMT == 0);     // wait until possible previous transmission data is done 
        TXREG = choice;                 // send one 8-bitbyte
        while(RCREG!=13);               // wait until user hits ENTER
    }
    
    flag = 0;
    if (choice == '0')
    {
    }
    else if(choice == '1')  //If user want to change the password
    {
        my_print("\n\rEnter Current Passcode:");    //Getting current passcode from the user
        halfSecDelay();
        
         int i;
    // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        for(i=0; i<4; i++)
        {
            choiceKeypad();                   // calls the keypadOnly() function to get the key pressed
            tempPass[i] = keyVal;           // sets the char variable option with the key pressed
            halfSecDelay();                        // calls a delay function for 2 sec delay
            my_print("*");
            
        }
        while(keyVal != 'A'){      // checks if the 'A' is pressed in keypad, here pressing 'A' functions as enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        for(i=0; i<4; i++)
        {
            tempPass[i] = reception();           // calls reception function to get the character input and store it to option
            my_print("*");
            
        }
        while(RCREG!=13);               // checks if the user hit 'Enter'
    }
    
        int my_correctPassword = changePassword(); //calling the changePassword() function to verify if it is possible or not.
                                                    //This function checks if the current passcode provided by the user is valid
                
        if(my_correctPassword == 1) //if the user provided passcode is valid, get new passcode from the user
        {
           my_print("\r\nEnter new 4 digit password: ");
           halfSecDelay();
    
             int i;
    addr = 0x00; //password saved in address 0x00, 0x01, 0x02 and 0x03 on EEPROM
    // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        for(i=0; i<4; i++)
        {
            choiceKeypad();                   // calls the keypadOnly() function to get the key pressed
            pass[i] = keyVal;          
            halfSecDelay();                    
            my_print("*");
            write_EEPROM(addr, pass[i]);
            addr++;
        }
        while(keyVal != 'A'){      // checks if the 'A' is pressed in keypad, here pressing 'A' functions as enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        for(i=0; i<4; i++)
        {
            pass[i] = reception();           // calls reception function to get the character input and store it to option
            my_print("*");
            write_EEPROM(addr, pass[i]);
            addr++;
        }
        while(RCREG!=13);               // checks if the user hit 'Enter'
    }
                my_print("\r\nYour Password has been changed!\n\r"); //Notifying user that the password has been changed
                halfSecDelay();
                halfSecDelay();

            }
        
        else    //If incorrect password is provided by the user, notifying the user that password change is not possible
        {
            my_print("\n\rIncorrect Password. Cannot change password.\n\r");
            halfSecDelay();
            halfSecDelay();   
        }

    }
    else //If invalid input is received
    {
        my_print("Invalid Selection\n\r");
        halfSecDelay();
        halfSecDelay();
        flag = 1;
    }
    
    }while(flag == 1);
    
}

 /******************************************************************************
 *  void option_PIR()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function displays the user with the PIR Motion Sensor options screen. 
  * User is able to enable or disable the PIR sensor alarm.
 ******************************************************************************/
void option_PIR()
{
    do
    {
  initialScreen();
  my_print("\n\r*********       PIR Sensor Alarm Menu           **************");
  my_print("\n\rSelect One of the Following:");
  my_print("\n\r                 1.Enable PIR Sensor Alarm");
  my_print("\n\r                 2.Disable PIR Sensor Alarm");
  my_print("\n\r                 0.Return to Main Menu");
  
  my_print("\n\rInput: ");
  halfSecDelay();
        // checks if the input from the keypad only mode is on
   if(PORTBbits.RB4 == 1)
    {
        choiceKeypad();                   // calling  choiceKeypad() function to get the key pressed
        while(TXSTAbits.TRMT == 0);     // waiting until possible previous transmission data is done
        TXREG = keyVal;            // sending one 8-bit byte
        choice = keyVal;           //saving the value to a char variable choice
        halfSecDelay();                 //Calling half second delay
        while(keyVal != 'A'){      //wait until user enters 'A'. 'A' is enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        choice = reception();           // calls reception function to get the character input and store it to choice
        while(TXSTAbits.TRMT == 0);     // wait until possible previous transmission data is done 
        TXREG = choice;                 // send one 8-bitbyte
        while(RCREG!=13);               // wait until user hits ENTER
    }

    flag = 0;
    
    if (choice == '0') //if user selects option 0 go to the main screen
    {
        mainScreen(); 
    }
    else if(choice == '1') //Option 1 enables the PIR Motion Sensor
    {
        initialScreen();
        initialize_PIR(); //Initializing the PIR sensor
        INTCONbits.INT0IE = 1; //Enable the INT0 interrupt
        write_EEPROM(PIRStatusAddr,'E');//Writing the status of 
                                        //the PIR Motion sensor 'E' to the EEPROM at address PIRStatusAddr
        my_print("\n\r PIR Sensor Alarm Enabled.\n\r");
        statusOfPir = "ACTIVE"; //changing the status display of the 
                                //PIR Motion sensor in the component status bar to ACTIVE
        halfSecDelay();
        halfSecDelay();
        halfSecDelay();
        halfSecDelay();
    

    }
    else if(choice == '2')//Option 1 disables the PIR Motion Sensor
    {
        statusOfPir = "INACTIVE";//changing the status display of the PIR Motion sensor in the 
                                //component status bar to INACTIVE
        initialScreen();
        INTCONbits.INT0IE = 0; //Disable the INT0 interrupt
        write_EEPROM(PIRStatusAddr,'D');//Writing the status of 
                                        //the PIR Motion sensor 'D'to the EEPROM at address PIRStatusAddr
        my_print("\n\r PIR Sensor Alarm Disabled.\n\r");
        halfSecDelay();
        halfSecDelay();
        halfSecDelay();
        halfSecDelay();
     
    }
    
    else //If invalid option is selected by the user
    {
        my_print("Invalid Selection.\n\r");
        halfSecDelay();
        halfSecDelay();
        halfSecDelay();
        halfSecDelay();
        flag = 1;
    }
    
    }while(flag == 1);
    
}

 /******************************************************************************
 *  void option_TempSensor()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 * This function displays the user with the Temperature Sensor options screen. 
 * User is able to enable or disable the Temperature sensor alarm. Also user 
 * can access the functionality of changing the Temperature threshold for the 
 * sensor
 ******************************************************************************/
void option_TempSensor()
{
    do{
        
  initialScreen();
  //Displaying the Temperature Sensor Alarm Options
  my_print("\n\r*********       Temperature Sensor Alarm Menu         ********");
  my_print("\n\rSelect One of the Following:");
  my_print("\n\r                 1.Enable Temperature Sensor Alarm");
  my_print("\n\r                 2.Disable Temperature Sensor Alarm");
  my_print("\n\r                 3.Change Temperature Sensor Alarm Threshold");
  my_print("\n\r                 0.Return to Main Menu");
  
  my_print("\n\rInput: ");
  halfSecDelay();
  
       // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        choiceKeypad();                   // calling  choiceKeypad() function to get the key pressed
        while(TXSTAbits.TRMT == 0);     // waiting until possible previous transmission data is done
        TXREG = keyVal;            // sending one 8-bit byte
        choice = keyVal;           //saving the value to a char variable choice
        halfSecDelay();                 //Calling half second delay
        while(keyVal != 'A'){      //wait until user enters 'A'. 'A' is enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        choice = reception();           // calls reception function to get the character input and store it to choice
        while(TXSTAbits.TRMT == 0);     // wait until possible previous transmission data is done 
        TXREG = choice;                 // send one 8-bitbyte
        while(RCREG!=13);               // wait until user hits ENTER
    }
    
    flag = 0;
    if (choice == '0')
    {
        
    }
    else if(choice == '1')//Enabling the temperature sensor alarm
    {
        
        initialize_tempSensor(); //Initializing the initialize_tempSensor() function 
                                    //initialize the temperature sensor
        T0CONbits.TMR0ON = 1;   //Timer0 on
        initialScreen();
        write_EEPROM(tempStatusAddr,'E');//Writing the status of temperature sensor 'E' 
                                        //to the EEPROM at address tempStatusAddr
        my_print("\n\r Temperature Sensor Alarm Enabled.\n\r");
        statusOfTemp = "ACTIVE";//changing the status display of the 
                                //Temperature sensor in the component status bar to ACTIVE
        halfSecDelay();
        halfSecDelay();
    }
    else if(choice == '2')//Disabling the temperature sensor alarm
    {
        statusOfTemp = "INACTIVE";//changing the status display of the 
                                //Temperature sensor in the component status bar to ACTIVE
        T0CONbits.TMR0ON = 0;   //Turning timer0 off
        PIE1bits.ADIE = 0; //Set ADIE enable bit
        PORTBbits.RB5 =0;   //Turning off the Yellow LED to let the user know that the temperature sensor
                            //is not active
        write_EEPROM(tempStatusAddr,'D');//Writing the status of temperature sensor 'D' 
                                        //to the EEPROM at address tempStatusAddr
        initialScreen();
        my_print("\n\r Temperature Sensor Alarm Disabled.\n\r");  
        halfSecDelay();
        halfSecDelay();      
    }
    
    else if (choice == '3')
    {
        updateThreshold();
    }
    else
    {
        my_print("\n\rInvalid Selection.\n\r");
        halfSecDelay();
        halfSecDelay();
        flag = 1;
    }
        
    }while(flag == 1);

}
 /******************************************************************************
 *  void option_Keyboard()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 *This function turns off the blue LED to let the user know that the selected
 * method of input is keyboard
 ******************************************************************************/
void option_Keyboard()
{
   //Make Keyboard the input 
    inputMethod = "KEYBOARD";
    write_EEPROM(inputTypeAddr, 'K');//Writing the status of input type 'K' 
                                     //to the EEPROM at address inputTypeAddr
    PORTBbits.RB4=0;//Setting pin 4 of PORTB to low to turn off the blue LED
    clearScreen();
}
 /******************************************************************************
 *  void option_Keypad()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 *This function turns on the blue LED to let the user know that the selected
 * method of input is keypad
 ******************************************************************************/
void option_Keypad()
{
    inputMethod = "KEYPAD";//changing the status display of the 
                            //input method in the component status bar to KEYPAD
     //Make Keypad the input 
    write_EEPROM(inputTypeAddr, 'B');//Writing the status of input type 'B' 
                                     //to the EEPROM at address inputTypeAddr
    PORTBbits.RB4=1; //Setting pin 4 of PORTB to high to turn on the blue LED
    clearScreen();
}


 /******************************************************************************
 *  void halfSecDelay()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 *This function generates 0.5 sec delay by calling 10ms second predefined function
 *50 times
 ******************************************************************************/
void halfSecDelay()
{
    int i;
    for (i = 0; i < 50; i++)
    {
        __delay_ms(10);
    }
}

 /******************************************************************************
 *  void My_ISR_High()
 *
 *  Parameter: None
 *
 *  Returns: No returns 
 *
 *This is the Interrupt Service Routine for the high priority interrupt generated
 *by the motion sensor. INT0 Enable bit and Interrupt flag when set, red LED is turned on
 * to notify the user that the motion sensor has been tripped. PIR_reset() function is called that
 * verifies the eligibility of the user to reset the alarm by asking for password input. Upon successful 
 * verification user is provided with the option to either keep the alarm enabled or disable. Also INT0IF
 * is cleared at the end if the ISR
 ******************************************************************************/
void interrupt My_ISR_High(void)
{
   
    int check;
    if(INTCONbits.INT0IF == 1 && INTCONbits.INT0IE ==1 ) //checking if the interrupt enable bit and
                                                            //interrupt flag  has been set
    {
    PORTBbits.RB2 = 1;  //turn of red led
         
    
    int sec_correctPassword = PIR_reset(); //calls the PIR_reset() function to verify the user eligibility to 
                                            //reset the alarm

    do
    {
        check = 0;
  
    if(sec_correctPassword == 1)//if correct password is provided by the user
    {
        //Turn off red LED and provided the user with the option to enable or disable the PIR motion sensor
        PORTBbits.RB2 = 0;
        my_print("\n\rDisable the PIR Sensor Alarm?");
        my_print("\n\rEnable = 1 and Disable = 0");
        
        my_print("\n\rInput: ");
        halfSecDelay();
        
            // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        choiceKeypad();                   // calls the keypadOnly() function to get the key pressed
        while(TXSTAbits.TRMT == 0);     // wait until possible previous transmission data is done
        TXREG = keyVal;            // send one 8-byte
        choice = keyVal;           // sets the char variable option with the key pressed
        halfSecDelay();                        // calls a delay function for 1 sec delay
        while(keyVal != 'A'){      // checks if the 'A' is pressed in keypad, here pressing 'A' functions as enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        choice = reception();           // calls reception function to get the character input and store it to option
        while(TXSTAbits.TRMT == 0);     // wait until possible previous transmission data is done 
        TXREG = choice;                 // send one 8-byte
        while(RCREG!=13);               // checks if the user hit 'Enter'
    }

        
        if(choice == '1') //If user wants to enable the sensor
        {
        initialScreen();
        initialize_PIR();
        INTCONbits.INT0IE = 1; //Enable the INT0 interrupt
        write_EEPROM(PIRStatusAddr,'E');//save the status to EEPROM
        my_print("\n\r PIR Sensor Alarm Enabled.\n\r");
        statusOfPir = "ACTIVE";
        halfSecDelay();
        halfSecDelay();

        }
        else if(choice == '0') //if user wants to disable the sensor
        {
        initialScreen();
        INTCONbits.INT0IE = 0; //Disable the INT0 interrupt
        write_EEPROM(PIRStatusAddr,'D');//save teh status to EEPROM
        my_print("\n\r PIR Sensor Alarm Disabled.\n\r");
        statusOfPir = "INACTIVE";
        halfSecDelay();
        halfSecDelay();

        
        }
        else //if invalid selection is made alarm is kept enabled
        {
        my_print("\n\rInvalid Selection");
        my_print("\n\r PIR SENSOR ALARM IS KEPT ENABLED");
        statusOfPir = "ACTIVE";
         initialScreen();
        initialize_PIR();
        INTCONbits.INT0IE = 1; //Enable the INT0 interrupt
        write_EEPROM(PIRStatusAddr,'E');//save the status in EEPROM
        my_print("\n\r PIR Sensor Alarm Enabled.\n\r");
        statusOfPir = "ACTIVE";
        halfSecDelay();
        halfSecDelay();

        }
        
        
    }
    else 
    {
        my_print("\n\rIncorrect Password.\n\r");
        check = 1;
        halfSecDelay();
        halfSecDelay();
        sec_correctPassword = PIR_reset();   
    }
    }while(check == 1);
    
    
    if (read_EEPROM(tempStatusAddr) == 'E')
    {
        initialize_tempSensor();
         T0CONbits.TMR0ON = 1;
      //   PIE1bits.ADIE = 1; //Set ADIE enable bit
        
       
    }
    else if (read_EEPROM(tempStatusAddr) == 'D')
    {
        T0CONbits.TMR0ON = 0;
        PIE1bits.ADIE = 0; //Set ADIE enable bit
        PORTBbits.RB5 =0;
    }
    
    INTCONbits.INT0IF = 0; //Reset the interrupt flag 
    }
    
    if(tempIntSet== 1)
    {
    my_print("\n\r!!!!!!!!   TEMPERATURE SENSOR HAS BEEN TRIPPED    !!!!!!!!!!");
    
    my_print("\r\nEnter the password to reset the alarm: \n\r");
    }
    else
    {
      my_print("\n\n\n\rEnter 0 to go back!!!\n\r");    

    }
}

/******************************************************************************
 *  void interrupt low_priority My_ISR_Low_TempSensor(void)
 *
 *  No input parameters
 *
 *  No returns 
 *
 *  This is the Interrupt Service Routine for the high priority interrupt generated
 *  by TIMER0 and AD.
 *  ISR is entered if TMR0IF or ADIF is set. Timer is stopped if TMR0IF is set and
 *  calls the function to get the current temperature from the temperature sensor.
 *  The timer is started again with the preload value to count 1 second.
 *  Timer0 is set on again.
 *  If ADIF is set high if current temperature received from the temperature senor is compared
 *  to the threshold temperature set by the user. If the temperature is lower than
 *  the threshold temperature the yellow LED is toggled. If the temperature is greater
 *  than the threshold temperature yellow LED turns on solid yellow
 *  Also when the temperature alarm is tripped the user is provided with the option to 
 *  change the threshold temperature if they want. Also user is provided with the option
 *  to enable or disable the temperature sensor.
 ******************************************************************************/
void interrupt low_priority My_ISR_Low_TempSensor(void)
{
    inReset = 1;
    int thirdCorrectPassword = 0;
    thresholdAddr = 0xD0;

    if(INTCONbits.TMR0IF == 1) //if TIMER0 Interrupt flag is set
    {
        T0CONbits.TMR0ON = 0; //set timer on
        FindCurrentTemp();  //calls FindCurrentTemp() to find the current temperature using the temperature sensor
        INTCONbits.TMR0IF=0; //clear the TIMER0 interrupt flag
        TMR0H = 0x67;   //preload values for 1 second
        TMR0L = 0x69;
        T0CONbits.TMR0ON = 1; //Turning on the timer0
    }
    
    if(PIR1bits.ADIF == 1)
    {
        //ADC conversion done
        //Get result from ADRESH/L
        FindCurrentTemp();//calls FindCurrentTemp() to find the current temperature using the temperature sensor
        int i;
        for(int i = 0; i < 2; i++) //read threshold temperate from EEPROM
        {
            thresholdArray[i] = read_EEPROM(thresholdAddr);
            thresholdAddr++;
        }
       tempThreshold = atoi(thresholdArray); //convert the read array to integer
       
        if(temperature>tempThreshold){ //If current temperature is greater than the threshold temperature
            tempIntSet = 1; 
            PORTBbits.RB5 =1; //Turn on the Yellow LED
            T0CONbits.TMR0ON = 0; //Timer off
            int third_correctPassword = temp_reset(); //calling temp_reset() function to reset the timer
            
            do
    {
    int check = 0;
  
    if(third_correctPassword == 1) //If user successfully reseted the timer then providing the user with the option
                                    //to update the threshold temperature
    {
        my_print("\n\rWould you like to update the Threshold Temperature?");
        my_print("\n\rYES = 1 and NO = 0");
        my_print("\n\rInput: ");
        halfSecDelay();
                // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        choiceKeypad();                   // calling  choiceKeypad() function to get the key pressed
        while(TXSTAbits.TRMT == 0);     // waiting until possible previous transmission data is done
        TXREG = keyVal;            // sending one 8-bit byte
        choice = keyVal;           //saving the value to a char variable choice
        halfSecDelay();                 //Calling half second delay
        while(keyVal != 'A'){      //wait until user enters 'A'. 'A' is enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        choice = reception();           // calls reception function to get the character input and store it to choice
        while(TXSTAbits.TRMT == 0);     // wait until possible previous transmission data is done 
        TXREG = choice;                 // send one 8-bitbyte
        while(RCREG!=13);               // wait until user hits ENTER
    }
        
        if(choice == '1')
        {
            updateThreshold(); //calls updateThreshold temperature
            
        }
        else
        {
        }
        
        //providing the user with a option to enable or disabling the alarm 
        my_print("\n\rDisable the Temperature Sensor Alarm?");
        my_print("\n\rEnable = 1 and Disable = 0");
        
        my_print("\n\rInput: ");
        halfSecDelay();
        
   // checks if the input from the keypad only mode is on
   if(PORTBbits.RB4 == 1)
    {
        choiceKeypad();                   // calling  choiceKeypad() function to get the key pressed
        while(TXSTAbits.TRMT == 0);     // waiting until possible previous transmission data is done
        TXREG = keyVal;            // sending one 8-bit byte
        choice = keyVal;           //saving the value to a char variable choice
        halfSecDelay();                 //Calling half second delay
        while(keyVal != 'A'){      //wait until user enters 'A'. 'A' is enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        choice = reception();           // calls reception function to get the character input and store it to choice
        while(TXSTAbits.TRMT == 0);     // wait until possible previous transmission data is done 
        TXREG = choice;                 // send one 8-bitbyte
        while(RCREG!=13);               // wait until user hits ENTER
    }
        
        if(choice == '1')
        {
        //SET THRESHOLD   
        statusOfTemp = "ACTIVE"; 
        initialize_tempSensor();
        T0CONbits.TMR0ON = 1; //set timer0 on
        initialScreen();
        write_EEPROM(tempStatusAddr,'E');
        my_print("\n\r Temperature Sensor Alarm Enabled.\n\r");
        
        halfSecDelay();
        halfSecDelay();
      
        
        }
        else if(choice == '0')
        {
        T0CONbits.TMR0ON = 0; //set timer off
        PIE1bits.ADIE = 0; //Set ADIE enable bit
        PORTBbits.RB5 =0; //turn off yellow led
        write_EEPROM(tempStatusAddr,'D');//saving the status in EEPROM
        initialScreen();
        my_print("\n\r Temperature Sensor Alarm Disabled.\n\r");
        statusOfTemp = "INACTIVE";
        halfSecDelay();
        halfSecDelay();

        }
        else //if invalid option is selected keep the alarm on
        {
        statusOfTemp = "ACTIVE";
        my_print("\n\rInvalid Selection");
        my_print("\n\r TEMPERATURE SENSOR ALARM IS KEPT ENABLED");
        halfSecDelay();
        initialize_tempSensor();
        T0CONbits.TMR0ON = 1; //turn on timer0
        initialScreen();
        write_EEPROM(tempStatusAddr,'E');//saving the status in EEPROM
        my_print("\n\r Temperature Sensor Alarm Enabled.\n\r");
        halfSecDelay();
        halfSecDelay();

        }   
        
    }
    else if(third_correctPassword == 0) //if incorrect password is received while reseting the temperature sensor
    {
        my_print("\n\rIncorrect Password\n\r");
        check = 1;
        halfSecDelay();
        halfSecDelay();
      
        third_correctPassword = temp_reset(); 
        
        
    }
    }while(check == 1);
    
      my_print("\n\n\n\rEnter 0 to go back!!!\n\r");    
   }else{
            PORTBbits.RB5 = PORTBbits.RB5 ^ 1; //Toggling the Yellow LED
            
        }
        
  
       PIR1bits.ADIF = 0; //clear flag 
}
   
}
 /******************************************************************************
 *  int PIR_reset()
 *
 *  Parameter: None
 *
 *  Returns: c_correctPassword
 *
 * This function resets the PIR motion sensor. Once the motion alarm has been 
 * tripped user is asked to enter the password to reset the alarm. Upon reception
 * of a correct password the alarm the function returns a positive indication of receiving
 * a correct password for reset
 ******************************************************************************/
int PIR_reset()
{
    initialScreen();
    my_print("\n\r!!!!!!!!!!!!! PIR SENSOR HAS DETECTED MOTION  !!!!!!!!!!!!!!");
    
    my_print("\r\nEnter the password to reset the alarm: ");
    halfSecDelay();
    
     int i;
    // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        for(i=0; i<4; i++)
        {
            choiceKeypad();                   // calls the keypadOnly() function to get the key pressed
            tempPass[i] = keyVal;           // sets the char variable option with the key pressed
            halfSecDelay();                        // calls a delay function for 2 sec delay
            my_print("*");
            
        }
        while(keyVal != 'A'){      // checks if the 'A' is pressed in keypad, here pressing 'A' functions as enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        for(i=0; i<4; i++)
        {
            tempPass[i] = reception();           // calls reception function to get the character input and store it to option
            my_print("*");
            
        }
        while(RCREG!=13);               // checks if the user hit 'Enter'
    }
    
    
     int c_correctPassword = 1;
     for(i = 0, addr = 0x00; i < 4; addr++,i++)
    {
        if(read_EEPROM(addr) == tempPass[i])
        {
            
        }
        else
        {
            c_correctPassword = 0;
            
        }
    }
     return c_correctPassword;
}

 /******************************************************************************
 *  int option_TempSensor()
 *
 *  Parameter: None
 *
 *  Returns: b_correctPassword
 *
 * This function resets the temperature sensor. Once the temperature alarm has been 
 * tripped user is asked to enter the password to reset the alarm. Upon reception
 * of a correct password the alarm is set off and updateThreshold() function is 
  * called where user is provided with the option
 * to update the threshold temperature. User can update the threshold if they want or
 * leave it the same.This function returns if the user entered 
 * the correct password of not.
 ******************************************************************************/
int temp_reset()
{
    initialScreen();
    my_print("\n\r!!!!!!!!   TEMPERATURE SENSOR HAS BEEN TRIPPED    !!!!!!!!!!");
    
    my_print("\r\nEnter the password to reset the alarm: ");
    halfSecDelay();
    
     int i;
    // checks if the input from the keypad only mode is on
    if(PORTBbits.RB4 == 1)
    {
        for(i=0; i<4; i++)
        {
            choiceKeypad();                   // calls the keypadOnly() function to get the key pressed
            tempPass[i] = keyVal;           // sets the char variable option with the key pressed
            halfSecDelay();                        // calls a delay function for 2 sec delay
            my_print("*");
            
        }
        while(keyVal != 'A'){      // checks if the 'A' is pressed in keypad, here pressing 'A' functions as enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        for(i=0; i<4; i++)
        {
            tempPass[i] = reception();           // calls reception function to get the character input and store it to option
            my_print("*");
            
        }
        while(RCREG!=13);               // checks if the user hit 'Enter'
    }
    
   int b_correctPassword = 1;
    
      for(i = 0, addr = 0x00; i < 4; addr++,i++)
    {
        if(read_EEPROM(addr) == tempPass[i])
        {
            
        }
        else
        {
            b_correctPassword = 0;
           
        }
    }
    
    return b_correctPassword;
    
}

 /******************************************************************************
 *  void updateThreshold()
 *
 *  Parameter: None
 *
 *  Returns: None
 *
 * This function changes the threshold temperature of the temperature sensor.
 * User provides the new threshold temperature and it is saved on EEPROM
 * at thresholdAddr address
 ******************************************************************************/
void updateThreshold()
{
    my_print("\n\r Enter Temperature threshold in Fahrenheit(2 digits): ");
      
      halfSecDelay();
      
      int i;
      thresholdAddr = 0xD0; //Address to store Threshold temperature 0xD0 and 0xD1
      
      
       if(PORTBbits.RB4 == 1) //If the selected mode of input is keypad
    {
        for(i=0; i<2; i++)
        {
            choiceKeypad();                   // calls the keypadOnly() function to get the key pressed
            thresholdArray[i] = keyVal;           // sets the char variable option with the key pressed
            
            halfSecDelay();                        // calls a delay function for 2 sec delay
            TXREG = thresholdArray[i];
            write_EEPROM(thresholdAddr, thresholdArray[i]); //writing the new threshold temperature to EEPROM
            
            thresholdAddr++;
        }
        while(keyVal != 'A'){      // checks if the 'A' is pressed in keypad, here pressing 'A' functions as enter
            choiceKeypad();               
        }
    }
    else                                // if not keypad only mode, take input from keyboard
    {
        for(i=0; i<2; i++)
        {
            thresholdArray[i] = reception();           // calls reception function to get the character input and store it to option
            TXREG = thresholdArray[i];
            write_EEPROM(thresholdAddr, thresholdArray[i]);//writing the new threshold temperature to EEPROM
            thresholdAddr++;
        }
        while(RCREG!=13);               // checks if the user hit 'Enter'
    }
     
    int i;
        for(int i = 0; i < 3; i++) //Reading the new threshold temperature from the EEPROM and saving it in an array
        {
            if(i<3)
            {
            thresholdArray[i] = read_EEPROM(thresholdAddr);
            thresholdAddr++;
            }
            else
            {
                thresholdArray[i] = '\0';
            }
        }
        
       tempThreshold = atoi(thresholdArray);  //converting an integer array to an integer
       
    
}
 /******************************************************************************
 *  int changePassword()
 *
 *  Parameter: None
 *
 *  Returns: a_correctPassword
 *
 * This function checks if the password provided by the user matches the password 
 * on the EEPROM. This function is used to verify if the password provided by the user
 * while changing the passcode is valid. This function returns if the user provided password 
 * is valid or not
 ******************************************************************************/
int changePassword()
{
    int i;
    int a_correctPassword = 1;
    for(i = 0, addr = 0x00; i < 4; addr++,i++) //Iterating through the password saved address in EEPROM
                                                //to check if the password provided by the user matches the password 
                                                //in the EEPROM
    {
        if(read_EEPROM(addr) == tempPass[i])
        {
            
        }
        else
        {
            a_correctPassword = 0;
           
        }
    }
    return a_correctPassword;
}


/******************************************************************************
 *  void FindCurrentTemp()
 *
 *  Parameter: None
 *
 *  Return: None
 *
 *  This function reads the analog voltage from the temperature sensor. The received range
 *  is from 0 - 1023 which is then converted to desired range of degree Celsius which then 
 *  is converted to Fahrenheit. [10 mV = 1 degree celsius]. Also the offset of 0.5 is kept 
 *  in consideration since TMP36 is used as the temperature sensor.
 ******************************************************************************/
void FindCurrentTemp()
{
    ADCON0 = 0b00000001;                // sets the configuration of ADCON0 register to take the
                                        // input from port A0/channel AN0 for FOSC/4
    ADCON1 = 0b00001110;                // setting the A/D port configuration to take analog input from AN0
    ADCON2 = 0b10101100;                // sets the configuration of ADCON2 register to select
                                        // the result format as right justified, set the A/D Acquistion time selcet bits
                                        // and A/D conversion clock select bits
    TRISAbits.TRISA0 = 1;               // sets the PORT A0 as input

    ADCON0bits.GO = 1;                  // sets the current status of A/D conversion as in progress
    while(ADCON0bits.DONE == 1);        // do nothing while the A/D conversion as in progress
    temp1 = ADRESH;                     // stores the right justified high bit of the conversion result
    temp1 = temp1 << 8;                 // left shifts the value of the external potentiometer by 8
    temp1 += ADRESL;                    // adds the result with the low bit of the conversion result
                                        // to make it a 10-bit number
    temperature = temp1;        
    temperature = (double)((temperature/1023)*5000); //converts the ADC result to a range of 0 to 5000 mV  
    temperature = temperature - 500;        // subtracts the offset of 500 mV for TMP36
    temperature = temperature/10;           // converts the mV to degree celsius [10 mV = 1 degree celsius]
    temperature = (temperature)*1.8+32;     // converts degree celsius to farhenheit, [F = C*1.8 + 32]
   
    //ADCON1 = 0b10001110;                // revert back the settings of ADCON1 before displaying
    
}


/******************************************************************************
 *  void checkStatus()
 *
 *  Parameter: None
 *
 *  Return: None
 *
 *  Every time the user login this function checks the status of temperature sensor,
 *  motion sensor, input type from the EEPROM and set up the appropriate values
 ******************************************************************************/
void checkStatus()
{

    //Checking the values in EEPROM
    if(read_EEPROM(PIRStatusAddr) !=(char)255) //If value is present(not FF) in EEPROM address PIRStatusAddr
    {
        
        if (read_EEPROM(PIRStatusAddr) == 'E') //if 'E' in PIRStatusAddr
        {
        statusOfPir = "ACTIVE"; //PIR motion sensor is active
        initialize_PIR();
        INTCONbits.INT0IE = 1; //Enable the INT0 interrupt
        
        }
        else if(read_EEPROM(PIRStatusAddr) == 'D') //if 'D' in PIRStatusAddr
        {
            INTCONbits.INT0IE = 0; //Disable the INT0 interrupt
            statusOfPir = "INACTIVE";//PIR motion sensor is inactive
        }
    }
    
    if(read_EEPROM(tempStatusAddr) !=(char)255)//If value is present(not FF) in EEPROM address tempStatusAddr
    {
        if (read_EEPROM(tempStatusAddr) == 'E')//if 'E' in statusOfTemp
        {
        statusOfTemp = "ACTIVE";//Temperature sensor is active
        initialize_tempSensor();
        T0CONbits.TMR0ON = 1;//set timer0 on
        }
        else if(read_EEPROM(tempStatusAddr) == 'D')//if 'D' in statusOfTemp
        {
            statusOfTemp = "INACTIVE";//Temperature sensor is inactive
            T0CONbits.TMR0ON = 0;//set timer0 off
            PIE1bits.ADIE = 0; //Set ADIE enable bit
            PORTBbits.RB5 =0;//turn off yellow led
        }
    }
    //check stored method of input
    if(read_EEPROM(inputTypeAddr) !=(char)255)//If value is present(not FF) in EEPROM address inputTypeAddr
    {
        if (read_EEPROM(inputTypeAddr) == 'B') //for keypad
        {
             inputMethod = "KEYPAD"; //input method is keypad
             PORTBbits.RB4=1; //turn on blue LED
        }
        else if  (read_EEPROM(inputTypeAddr) == 'K')  //for keyboard
        {
            inputMethod = "KEYBOARD"; //input method is keyboard
            PORTBbits.RB4=0; //turn off blue LED
        }
    }
    
}

/******************************************************************************
 *  void write_EEPROM()
 *
 *  Parameter: address to write in EEPROM 
 *             character to write
 *
 *  Return: None
 *
 *  This function writes the user provided character to the user provided 
 *  address in EEPROM
 ******************************************************************************/
void write_EEPROM(int add, char c)
{
        EEADR = add; //Loading the user provided address to EEADR register
        EEDATA = c;  //Loading the EEDATA register with the user provided character
        //EECON1 configuration for writing to EEPROM
        EEPGD = 0;  
        CFGS = 0;
        WREN = 1;
        EECON2 = 0x55; //write 0x55 to EECON2
        EECON2 = 0xAA; //write 0xAA to EECON2
        EECON1bits.WR = 1;
        while(EECON1bits.WR == 1);//wait while writing. WR is cleared when writes finish
}

/******************************************************************************
 *  char read_EEPROM()
 *
 *  Parameter: address to read from EEPROM 
 *          
 *
 *  Return: EEDATA
 *
 *  This function reads the character stored in the user provided address of the
 *  EEPROM and returns the value.
 ******************************************************************************/
char read_EEPROM(int add)
{
    EEADR = add; //Loading user provided address to read to EEADR register
    //EECON1 configuration for reading from EEPROM
    EEPGD = 0;
    CFGS = 0;
    RD = 1;
    return EEDATA; //Return the read data

}

/******************************************************************************
 *  void choiceKeypad()
 *
 *  Parameter: None
 *
 *  Return: None 
 *
 *  This function saves the character received from 4*4 keypad and stores it in 
 *  keyVal.The value is received by setting all columns of PORTD as input and all 
 *  rows as outputs. Then the row is set high and it checks each column in that row 
 *  When a high column is found the key is determined from the corresponding
 *  row and column value.
 ******************************************************************************/
void choiceKeypad()
{
    TRISD = 0b11110000; //Setting PORTD top 4 MSB as input and lower 4 LSB as outputs
    
    while(1){
        //setting each row high at a time and checking all the columns. Turning off the row if no column found
        //Once high row and its corresponding high column is found, corresponding keyVal is the pressed key and
        //the function breaks.
        PORTD = 1; 
        if(PORTDbits.RD4==1)
        {
            keyVal = '1';
            break;
        }
        else if(PORTDbits.RD5==1)
        {
            keyVal = '2';
            break;
        }
        else if(PORTDbits.RD6==1)
        {
            keyVal = '3';
            break;
        }
        else if(PORTDbits.RD7==1)
        {
            keyVal = 'A'; 
            break;
        }
        PORTD = 2;
        if(PORTDbits.RD4==1)
        {
            keyVal = '4';
            break;
        }
        else if(PORTDbits.RD5==1)
        {
            keyVal = '5';
            break;
        }
        else if(PORTDbits.RD6==1)
        {
            keyVal = '6';
            break;
        }
        else if(PORTDbits.RD7==1)
        {
            keyVal = 'B'; 
            break;
        }
        PORTD = 4;
        if(PORTDbits.RD4==1)
        {
            keyVal = '7';
            break;
        }
        else if(PORTDbits.RD5==1)
        {
            keyVal = '8';
            break;
        }
        else if(PORTDbits.RD6==1)
        {
            keyVal = '9';
            break;
        }
        else if(PORTDbits.RD7==1)
        {
            keyVal = 'C';
            break;
        }
        PORTD = 8;
        if(PORTDbits.RD4==1)
        {
            keyVal = '*';
            break;
        }
        else if(PORTDbits.RD5==1)
        {
            keyVal = '0'; 
            break;
        }
        else if(PORTDbits.RD6==1)
        {
            keyVal = '#';
            break;
        }
        else if(PORTDbits.RD7==1)
        {
            keyVal = 'D';
            break;
        }
    }
    
    PORTBbits.RB4 =PORTBbits.RB4^1; //Turning off the blue LED when keypad is pressed
}



