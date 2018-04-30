/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This 
    library has basic methods of various sensors and communications so that you can make what you want with them. <br> 
    <br><br>
    </p>
    
    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>
    
    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/

#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include "Systick.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "I2C_made.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "IR.h"
#include "Ambient.h"
#include "Beep.h"
#include <time.h>
#include <sys/time.h>
int rread(void);

void turnLeftSoft();
void turnLeftMed();
void turnLeftHard();
void turnRightSoft();
void turnRightMed();
void turnRightHard();
void uTurn(int direction, uint8 speed);
float getUltraAverage();

/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/

//viivanseuranta
#if 0

    //viivanseuranta logiikka.
    
int main()
{
    //struct sensors_ ref;
    struct sensors_ dig;
    
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    IR_Start();
    IR_flush();
    
    ADC_Battery_Start();        

    bool ir = true;
    int stop = 0;
    int direction = 0;  // 1 for left, 2 for right.
    int16 adcresult =0;
    float volts = 0.0;
    float blinking =0;
    float scaling_factor = 0;

    printf("\nBoot\n\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed
    
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    printf("\n testi \n");
    while(ir == true)
    {
        printf("\n testi1 \n");
        motor_start();
        MotorDirLeft_Write(0);      //left motor frwd (1 = backwards)
        MotorDirRight_Write(0);     //right motor frwd (1 = backwards)
        PWM_WriteCompare1(50);
        PWM_WriteCompare2(50);
        reflectance_digital(&dig);
        if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            motor_stop();
            ir = false;
            break;
            printf("\n testi2 \n");
        }
    }
    printf("Waiting for IR command.\n");
    IR_wait();
    printf("IR command received\n");
    
    for(;;)
    {
        reflectance_digital(&dig);      //print out 0 or 1 according to results of reflectance period
        //printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);
        CyDelay(1);
        PWM_WriteCompare1(250);
        PWM_WriteCompare2(250);
        
        if(dig.l1 == 1 && dig.r1 == 1/* && dig.l2 == 0&& dig.r2 == 0*/)
        {
            if((stop == 0 || stop == 1 || stop == 2) 
            && (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
            {
                CyDelay(120);
                stop++;
                
                if(stop >= 3)
                {
                    stop = 3;
                }
                printf("%d\n", stop);
            }
            reflectance_digital(&dig); 
            if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1 && stop == 3)
                {
                    printf("motor stop\n");
                    motor_stop();
                    IR_wait();
                }
            motor_start();
            MotorDirLeft_Write(0);      //left motor frwd (1 = backwards)
            MotorDirRight_Write(0);     //right motor frwd (1 = backwards)
            PWM_WriteCompare1(250);
            PWM_WriteCompare2(250);
            printf("\n straight\n");
        }
        else if((dig.r1 == 1 && dig.l1 == 0) || (dig.r1 == 1 && dig.r2 == 1))
        {
            turnRightSoft();
            direction = 2;
        }
        else if((dig.l1 == 1 && dig.r1 == 0) || (dig.l1 == 1 && dig.l2 == 1))
        {
            turnLeftSoft();
            direction = 1;
        }
        else if((dig.r2 == 1 && dig.r1 == 0) || (dig.r2 == 1 && dig.r3 == 1))
        {
            turnRightMed();
            direction = 2;
        }
        else if((dig.l2 == 1 && dig.l1 == 0) || (dig.l2 == 1 && dig.l3 == 1))
        {
            turnLeftMed();
            direction = 1;
        }
        else if(dig.r3 == 1 && dig.r2 == 0 && dig.r1 == 0)
        {
            turnRightHard();
            direction = 2;
        }
        else if(dig.l3 == 1 && dig.l2 == 0 && dig.l1 == 0)
        {
            turnLeftHard();
            direction = 1;
        }
        else 
        {
            if(direction == 1)
            {
                turnLeftHard();
            }
            else if(direction == 2)
            {
                turnRightHard();
            }
            else
            {
                motor_stop();
            }
        }
        
        // read digital values that are based on threshold. 0 = white, 1 = black
        // when blackness value is over threshold the sensors reads 1, otherwise 0
        //printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);
        //print out 0 or 1 according to results of reflectance period
        
        ADC_Battery_StartConvert();
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) 
        {   // wait for get ADC converted value
            adcresult = ADC_Battery_GetResult16(); // get the ADC value (0 - 4095)
            // convert value to Volts
            // you need to implement the conversion
            volts = (float)adcresult / (float)4095 * (float)5 * scaling_factor;
            // Print both ADC results and converted value
            //printf("%d %.4f\r\n", adcresult, volts);
            
            if (volts < 4.25 && blinking == 0)
            {
                BatteryLed_Write(1);
                blinking = 1;
            } 
            else if(volts < 4.25 && blinking)
            {
                BatteryLed_Write(0);
                blinking = 0;
            }
            else
            { 
                BatteryLed_Write (0);
            }
            
        }
    }
}
#endif

//sumopaini
#if 1
    
    //sumo paini koodi.
    
int main()
{
    struct sensors_ dig;
    
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    IR_Start();
    IR_flush();
    Ultra_Start();                          // Ultra Sonic Start function
    
    bool line = true;
    bool ir = true;
    //int16 adcresult =0;
    //float volts = 0.0;
    //float blinking =0;
    //float scaling_factor = 0;
    float UltraAverage;
    int breaker;
    int loop;
    int move;
    int dir = 0;
   
    printf("Boot\n");
    
    ADC_Battery_Start();
    
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    
    motor_start();
        
    /*
    motor_forward(100,2000);     // moving forward
    motor_turn(200,50,2000);     // turn
    motor_turn(50,200,2000);     // turn
    motor_backward(100,2000);    // movinb backward
    */
    while(ir == true)
    {
        reflectance_digital(&dig);
        motor_forward(50, 10);
        printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);
        if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            motor_stop();
            ir = false;
            break;
        }
    }
    printf("Waiting for IR command.\n");
    IR_wait();
    printf("IR command received\n");
    motor_start();
    motor_forward(125,400);
    
    for(;;) 
    {
        move = rand() % 3 + 1;
        switch(move)
        {
            case 1:
                motor_forward(125, 50);
                dir = 3;
                break;
            case 2:
                turnLeftMed();
                CyDelay(50);
                dir = 1;
                break;
            case 3:
                turnRightMed();
                CyDelay(50);
                dir = 2;
                break;
        }
        
        reflectance_digital(&dig);
        
        if(dig.l3 != 0 || dig.l2 != 0 || dig.l1 != 0 || dig.r1 != 0 || dig.r2 != 0 || dig.r3 != 0)
        {
            /*if(line == true)
            {
                CyDelay(150);
                line = false;
                printf("line crossed\n");
            }
            else
            {*/
                motor_backward(250, 150);
                
                int random = rand() % 2;
                uTurn(random, 125);
                CyDelay(700);
                
                motor_forward(125, 1);
            //}
        }
        else
        {
            UltraAverage = getUltraAverage();
            loop = 1;
            //timer = 0;
            if(UltraAverage <= 30 )//&& loop == 1 && timer < 2000)
            {
                breaker = 1;
                while(loop == 1)
                {
                    UltraAverage = getUltraAverage();
                    
                    motor_forward(250, 1);
                    if(UltraAverage > 30)
                    {
                        switch(dir)
                        {
                            case 1:
                                //uTurn(0, 200);
                                turnLeftHard();
                                CyDelay(15);
                                breaker++;
                                break;
                            case 2:
                                //uTurn(1, 200);
                                turnRightHard();
                                CyDelay(15);
                                breaker++;
                                break;
                            case 3:
                                loop = 0;
                                break;
                        }
                        if(breaker > 50)
                        {
                            loop = 0;
                        }
                    }
                reflectance_digital(&dig);
                if(dig.l3 != 0 || dig.l2 != 0 || dig.l1 != 0 || dig.r1 != 0 || dig.r2 != 0 || dig.r3 != 0)
                {
                    printf("loop -> 0\n");
                    loop = 0;
                }
                }
            }
        }
        
        /*ADC_Battery_StartConvert();
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) 
        {   // wait for get ADC converted value
            adcresult = ADC_Battery_GetResult16(); // get the ADC value (0 - 4095)
            // convert value to Volts
            // you need to implement the conversion
            volts = (float)adcresult / (float)4095 * (float)5 * scaling_factor;
            // Print both ADC results and converted value
            //printf("%d %.4f\r\n", adcresult, volts);
            
            if (volts < 4.25 && blinking == 0)
            {
                BatteryLed_Write(1);
                blinking = 1;
            } 
            else if(volts < 4.25 && blinking)
            {
                BatteryLed_Write(0);
                blinking = 0;
            }
            else
            { 
                BatteryLed_Write (0);
            }
        }*/
    }  
}
    
#endif

void turnLeftSoft()
{
    printf("\nturnLeftSoft\n");
    MotorDirLeft_Write(0);
    MotorDirRight_Write(0);
    PWM_WriteCompare1(150); //left
    PWM_WriteCompare2(250); //right
}
void turnLeftMed()
{
    printf("\nturnLeftMed\n");
    MotorDirLeft_Write(0);
    MotorDirRight_Write(0);
    PWM_WriteCompare1(25);
    PWM_WriteCompare2(250);
}
void turnLeftHard()
{
    printf("\nturnLeftHard\n");
    MotorDirLeft_Write(1);
    MotorDirRight_Write(0);
    PWM_WriteCompare1(0);
    PWM_WriteCompare2(250);
}

void turnRightSoft()
{
    printf("\nturnRightSoft\n");
    MotorDirLeft_Write(0);
    MotorDirRight_Write(0);
    PWM_WriteCompare2(150);
    PWM_WriteCompare1(250);
}
void turnRightMed()
{
    printf("\nturnRightMed\n");
    MotorDirLeft_Write(0);
    MotorDirRight_Write(0);
    PWM_WriteCompare2(25);
    PWM_WriteCompare1(250);
}
void turnRightHard()
{
    printf("\nturnRightHard\n");
    MotorDirLeft_Write(0);
    MotorDirRight_Write(1);
    PWM_WriteCompare2(0);
    PWM_WriteCompare1(250);
}
void uTurn(int direction, uint8 speed)
{
    printf("uTurn\n");
    if(direction == 0)
    {
        MotorDirLeft_Write(0);
        MotorDirRight_Write(1);
        PWM_WriteCompare2(speed);
        PWM_WriteCompare1(speed);
    }
    else if(direction == 1)
    {
        MotorDirLeft_Write(1);
        MotorDirRight_Write(0);
        PWM_WriteCompare2(speed);
        PWM_WriteCompare1(speed); 
    }
}
float getUltraAverage()
{
    int UltraValues[20];
    float UltraAverage = 0;
    for(int i = 0; i <=19; i++)
            {
                UltraValues[i] = Ultra_GetDistance();
                printf("distance = %d\r\n", Ultra_GetDistance()); 
            }
            for(int i = 0; i <=19; i++)
            {
                UltraAverage += UltraValues[i];
            }
            UltraAverage = UltraAverage / 20;
            printf("Average: %.2f\r\n", UltraAverage);
            return UltraAverage;
}

#if 0
    MotorDirLeft_Write(0);      //left motor frwd (1 = backwards)
    PWM_WriteCompare1(120);     //left speed
    MotorDirRight_Write(0);     //right motor frwd (1 = backwards)
    PWM_WriteCompare2(120);     //right speed
    CyDelay(3000);              // time for motor
    MotorDirLeft_Write(0);
    MotorDirRight_Write(1);
    PWM_WriteCompare2(120);
    PWM_WriteCompare1(120);
    CyDelay(440);               //first 90 deg turn.
    MotorDirLeft_Write(0);
    PWM_WriteCompare1(120);
    MotorDirRight_Write(0);
    PWM_WriteCompare2(120);
    CyDelay(2600);
    MotorDirLeft_Write(0);
    MotorDirRight_Write(1);
    PWM_WriteCompare2(120);
    PWM_WriteCompare1(120);
    CyDelay(440);
    MotorDirLeft_Write(0);
    PWM_WriteCompare1(120);
    MotorDirRight_Write(0);
    PWM_WriteCompare2(120);
    CyDelay(2600);
    MotorDirLeft_Write(0);
    MotorDirRight_Write(1);
    PWM_WriteCompare2(80);
    PWM_WriteCompare1(120);
    CyDelay(600);
    MotorDirLeft_Write(0);
    PWM_WriteCompare1(120);
    MotorDirRight_Write(0);
    PWM_WriteCompare2(60);
    CyDelay(3100);
    MotorDirLeft_Write(0);
    PWM_WriteCompare1(120);
    MotorDirRight_Write(0);
    PWM_WriteCompare2(120);
    CyDelay(1000);
    
    motor_stop();
    
    }
    motor_stop();
 }  

#endif

#if 0
// button
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    
    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed
    
    bool led = false;
    
    for(;;)
    {
        // toggle led state when button is pressed
        if(SW1_Read() == 0) {
            led = !led;
            BatteryLed_Write(led);
            ShieldLed_Write(led);
            if(led) printf("Led is ON\n");
            else printf("Led is OFF\n");
            Beep(1000, 150);
            while(SW1_Read() == 0) CyDelay(10); // wait while button is being pressed
        }        
    }
 }   
#endif


#if 0
//ultrasonic sensor//
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    Ultra_Start();                          // Ultra Sonic Start function
    
    float UltraAverage;
    
    while(1) {
        UltraAverage = getUltraAverage();
        printf("%f\n", UltraAverage);
        CyDelay(500);
    }
}   
#endif


#if 0
//IR receiver//
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    IR_Start();
    
    uint32_t IR_val; 
    
    printf("\n\nIR test\n");
    
    IR_flush(); // clear IR receive buffer
    printf("Buffer cleared\n");
    
    IR_wait(); // wait for IR command
    printf("IR command received\n");
    
    // print received IR pulses and their lengths
    for(;;)
    {
        if(IR_get(&IR_val)) {
            int l = IR_val & IR_SIGNAL_MASK; // get pulse length
            int b = 0;
            if((IR_val & IR_SIGNAL_HIGH) != 0) b = 1; // get pulse state (0/1)
            printf("%d %d\r\n",b, l);
            //printf("%d %lu\r\n",IR_val & IR_SIGNAL_HIGH ? 1 : 0, (unsigned long) (IR_val & IR_SIGNAL_MASK));
        }
    }    
 }   
#endif


#if 0
//reflectance//
int main()
{
    //struct sensors_ ref;
    struct sensors_ dig;

    Systick_Start();

    CyGlobalIntEnable; 
    UART_1_Start();
  
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    
        motor_start();
        MotorDirLeft_Write(1);
        MotorDirRight_Write(1);
        PWM_WriteCompare1(255);
        PWM_WriteCompare2(255);

    for(;;)
    {
        // read raw sensor values
        //reflectance_read(&ref);
        //printf("%5d %5d %5d %5d %5d %5d\r\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);       // print out each period of reflectance sensors
        
        // read digital values that are based on threshold. 0 = white, 1 = black
        // when blackness value is over threshold the sensors reads 1, otherwise 0
        reflectance_digital(&dig);      //print out 0 or 1 according to results of reflectance period
        printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);        //print out 0 or 1 according to results of reflectance period
        
        CyDelay(200);
        
    }
}   
#endif


#if 0
//motor//
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();

    motor_start();              // motor start

    motor_forward(100,2000);     // moving forward
    motor_turn(200,50,2000);     // turn
    motor_turn(50,200,2000);     // turn
    motor_backward(100,2000);    // movinb backward
       
    motor_stop();               // motor stop
    
    for(;;)
    {

    }
}
#endif


/* [] END OF FILE */
