// This script runs on a Sleepy-Pi add-on board for the Raspberry Pi.
//
// The script boots the Raspberry Pi hourly, and also 
// allows boot and shutdown by a button-press.
//
// harald.vanderwerff@utwente.nl
// May 2020

// **** INCLUDES *****
#include "SleepyPi2.h"
#include <Time.h>
#include <LowPower.h>
#include <PCF8523.h>
#include <Wire.h>

// States
typedef enum {
  eWAIT = 0,
  eBUTTON_PRESSED,
  eBUTTON_WAIT_ON_RELEASE,
  eBUTTON_HELD,
  eBUTTON_RELEASED
}eBUTTONSTATE;

typedef enum {
   ePI_OFF = 0,
   ePI_BOOTING,
   ePI_ON,
   ePI_SHUTTING_DOWN
}ePISTATE;

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
}; 

const int LED_PIN = 13;
const int RELAY_PIN = 9;

tmElements_t   tm;
volatile bool  button_Pressed = false;
eBUTTONSTATE   button_State = eWAIT;
ePISTATE       pi_state = ePI_OFF;
bool           state = LOW;
unsigned long  time, timePress;

//+++ Edit

// .. Setup the Periodic Timer
unsigned long    MAX_RPI_TIME_TO_STAY_AWAKE_MS  = 600000;
uint8_t          wakeMinute = 50;    // the minute of the hour to wake up on the alarm

#define          kBUTTON_POWEROFF_TIME_MS   2000
#define          kBUTTON_FORCEOFF_TIME_MS   8000
#define          kPI_CURRENT_THRESHOLD_MA   200
#define          kSUPPLY_VOLTAGE_MAXIMUM_V  24
#define          kSUPPLY_VOLTAGE_MINIMUM_V  8
// --- Edit

void button_isr()
{
    // A handler for the Button interrupt.
    button_Pressed = true;
}

void alarm_isr()
{
    // A handler for the Alarm interrupt.
}


void setup()
{
  //bool pi_running;
  //SleepyPi.simulationMode = false;  // Don't actually shutdown
  
  //delay(5000);
  
  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  Serial.println("SETUP: Starting...");
  
  Serial.print("SETUP: The time is now ");
  printTimeNow();
  
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);    
  digitalWrite(LED_PIN,HIGH);   // Switch on LED
  
  // Configure our RELAY pin
  pinMode(RELAY_PIN, OUTPUT);    
  analogWrite(RELAY_PIN,0);     // Switch off RELAY

  SleepyPi.enablePiPower(false);  
  SleepyPi.enableExtPower(false);
  
  // Allow wake up triggered by button press
  attachInterrupt(1, button_isr, LOW);       // button pin  

  SleepyPi.rtcInit(true);
 
  // Setup the Alarm Time
  SleepyPi.setAlarm(wakeMinute);
  
  Serial.print("SETUP: Wake time is at ");
  Serial.print(wakeMinute);
  Serial.println(" minutes of the hour");
  
}

void loop() 
{
    bool pi_running;
    float pi_voltage;
    float pi_current;
    unsigned long buttonTime;
    unsigned long TimeOutStart,ElapsedTimeMs,TimeOutEnd ;

    switch(button_State)
    {
        case eWAIT:
        
            SleepyPi.rtcClearInterrupts();  
        
            // Enter power down state with ADC and BOD module disabled.
            // Wake up when wake button is pressed.
            // Once button is pressed stay awake - this allows the timer to keep running
            
            Serial.println("ACTION: Going to go to sleep for a while...\n");

            // switch everything off
            //turnRelayOffNow();
            //turnPowerOffNow(); 

            // Allow wake up alarm to trigger interrupt on falling edge.
            attachInterrupt(0, alarm_isr, FALLING);   // Alarm pin

            // Allow to wake up by the alarm
            SleepyPi.enableWakeupAlarm(true);
    
            // Setup the Alarm Time
            SleepyPi.setAlarm(wakeMinute);
    
            // delay a little to let this sink in....
            delay(500);

            // Enter power down state with ADC and BOD module disabled.
            // Wake up when wake up pin is low (which occurs when our alarm clock goes off)
            SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);  
            // GO TO SLEEP...
            // ...
            // ...
            
            // I'm awake !!!     
            digitalWrite(LED_PIN,LOW);   // Switch off LED
            // What woke me up? Was it a button press or a scheduled wake?
            // Lets check on button press
            if(button_Pressed == false)
            {           
                // This was an alarm interrupt and not a button press

                //delay(10);  // current reading is artificially high if we don't delay first
               // pi_current = SleepyPi.rpiCurrent();
                //Serial.print("ALARM: RPI current is ");
                //Serial.print(pi_current);  
                //Serial.println(" mA");
                
                // read the supply voltage
                delay(10);  // voltage reading is artificially high if we don't delay first
                pi_voltage = SleepyPi.supplyVoltage();
                //Serial.print("ALARM: Supply voltage is ");
                //Serial.print(pi_voltage);
                //Serial.println(" V");

                // switch on if the supply voltage is withn a safe range
                //if((pi_voltage < kSUPPLY_VOLTAGE_MAXIMUM_V) && (pi_voltage > kSUPPLY_VOLTAGE_MINIMUM_V)) 
                //{
                //     //Serial.println("ALARM: Warming up the camera");  
                //     turnRelayOnNow();
                //}
                //else
                //{
                //     //Serial.println("ALARM: Not starting the camera");  
                //}
                
                // turn on the relay when voltage is within safe range
                turnRelayOnSafely();
                
                // delay ~5 minutes to stabilize the camera
                delay(300000); 
                
                //Serial.println("ALARM: Booting the rPI");
                turnPowerOnNow();
                
                // delay 15 secs more to have the rPI fully up
                delay(15000);
      
                // check that the pi is running
                pi_running = SleepyPi.checkPiStatus(kPI_CURRENT_THRESHOLD_MA,false); 
                TimeOutStart = millis();
                ElapsedTimeMs = TimeOutStart;
                TimeOutEnd = TimeOutStart + MAX_RPI_TIME_TO_STAY_AWAKE_MS;
                while((pi_running == true) && (ElapsedTimeMs < TimeOutEnd))
                {
                    pi_running = SleepyPi.checkPiStatus(kPI_CURRENT_THRESHOLD_MA,false); 
                    ElapsedTimeMs = millis();
                    digitalWrite(LED_PIN,HIGH); // Switch on LED 
                    delay(500);
                    digitalWrite(LED_PIN,LOW); // Switch off LED
                    delay(500);
                    Serial.flush(); 
                    Serial.println(pi_voltage);
                }
                //Serial.println("ALARM: Cutting power to the camera");
                turnRelayOffNow();
                        
                //Serial.println("ALARM: Shutting down the rPI");
                turnPowerOffSafely();
                
                //Serial.println("ALARM: Cutting power to the rPI");
                turnPowerOffNow();   
          
                digitalWrite(LED_PIN,HIGH);   // Switch on LED      

                // ...Check on our RPi
                pi_running = SleepyPi.checkPiStatus(kPI_CURRENT_THRESHOLD_MA,false); 
                switch(pi_state)
                {            
                  case ePI_BOOTING:
                       // Check if we have finished booting
                       if(pi_running == true)
                       {
                           // We have booted up!
                           pi_state = ePI_ON;                  
                       }
                       else 
                       {
                           // Still not completed booting so lets carry on waiting
                           pi_state = ePI_BOOTING;                   
                       } 
                       break;
                  case ePI_ON:
                       // Check if it is still on?
                       if(pi_running == false)
                       {
                           // Shock horror! it's not running!!
                           // Assume it has been manually shutdown, so lets cut the power
                           // Force Pi Off  
                           turnRelayOffNow();             
                           turnPowerOffNow();                           
                       }
                       else 
                       {
                           // Still on - all's well - keep this state                           
                           pi_state = ePI_ON;                   
                       } 
                       break;
                  case ePI_SHUTTING_DOWN:
                      // Is it still shutting down? 
                       if(pi_running == false)
                       {
                           // Finished a shutdown
                           // Force the Power Off
                           turnRelayOffNow();             
                           turnPowerOffNow();                          
                       }
                       else 
                       {
                           // Still shutting down - keep this state
                           pi_state = ePI_SHUTTING_DOWN;                   
                       } 
                      break;
                  case ePI_OFF:             
                  default:       // intentional drop thru
                     // RPi off, so we'll continue to wait
                     // for a button press to tell us to switch it on
                     delay(10);
                     pi_state = ePI_OFF;    
                     break; 
                }
                button_State = eWAIT; // Loop back around and go to sleep again
                detachInterrupt(0);
                SleepyPi.ackAlarm();                   
           }
           else
           {
              // This was not an alarm but a button press
              
              // Change the button state (and stay awake)
              button_Pressed = false;
  
              // Disable the alarm interrupt
              detachInterrupt(0);
        
              // Disable external pin interrupt on wake up pin.
              detachInterrupt(1);
              button_State = eBUTTON_PRESSED; 
           }
           break;
           
        case eBUTTON_PRESSED:
            button_Pressed = false;  
            timePress = millis();    // Log Press time                
            pi_running = SleepyPi.checkPiStatus(kPI_CURRENT_THRESHOLD_MA,false); 
            if(pi_running == false)
            {  
                // Switch on the Pi
                //Serial.println("BUTTON: Booting the rPI");           
                turnPowerOnNow();
                turnRelayOnSafely();

            }          
            button_State = eBUTTON_WAIT_ON_RELEASE;
            digitalWrite(LED_PIN,LOW);         
            attachInterrupt(1, button_isr, HIGH);    // Will go high on release           
            break;
            
        case eBUTTON_WAIT_ON_RELEASE:
            if(button_Pressed == true)
            {
                detachInterrupt(1); 
                button_Pressed = false;                            
                time = millis();     //  Log release time
                button_State = eBUTTON_RELEASED;
            }
            else
            {
                // Carry on waiting
                button_State = eBUTTON_WAIT_ON_RELEASE;  
            }
            break;
            
        case eBUTTON_RELEASED:           
            pi_running = SleepyPi.checkPiStatus(kPI_CURRENT_THRESHOLD_MA,false); 
            if(pi_running == true)
            {
                // Check how long we have held button for
                buttonTime = time - timePress;
                if(buttonTime > kBUTTON_FORCEOFF_TIME_MS)
                {
                   // Force it off immediately
                   turnRelayOffNow();
                   turnPowerOffNow(); 
        
                } 
                else if (buttonTime > kBUTTON_POWEROFF_TIME_MS)
                {
                    // Start a gentle shutdown
                    turnPowerOffSafely();
                    // Then turn it off completely
                    turnRelayOffNow();
                    turnPowerOffNow();  
                } 
                else 
                { 
                     // Button not held off long - Do nothing
                } 
            } 
            else 
            {
                // Pi is not running - do nothing
            }
            digitalWrite(LED_PIN,HIGH);            
            attachInterrupt(1, button_isr, LOW);    // button pin
            button_State = eWAIT;        
            break;
        default:
            break;
    }
}

void turnRelayOnSafely()
{
    Serial.println("ACTION: Turn on relay safely"); 
    float pi_voltage;
    // read the supply voltage (artificially high if we don't delay first)
    delay(10);
    pi_voltage = SleepyPi.supplyVoltage();
    // switch on if the supply voltage is within a safe range
    if((pi_voltage < kSUPPLY_VOLTAGE_MAXIMUM_V) && (pi_voltage > kSUPPLY_VOLTAGE_MINIMUM_V)) 
    {
        turnRelayOnNow();
    } 
}

void turnRelayOnNow()
{
    SleepyPi.enableExtPower(true);
    analogWrite(RELAY_PIN,255); // Switch on relay
    Serial.println("ACTION: Turn on relay now"); 
}

void turnRelayOffNow()
{  
    analogWrite(RELAY_PIN,0);   // Switch off relay
    SleepyPi.enableExtPower(false);
    Serial.println("ACTION: Turn off relay now");
}

void turnPowerOnNow()
{
    Serial.println("ACTION: Turn on power now");
    SleepyPi.enablePiPower(true);
    pi_state = ePI_BOOTING;
}

void turnPowerOffNow()
{
    Serial.println("ACTION: Turn off power now");
    SleepyPi.enablePiPower(false);
    pi_state = ePI_OFF;  
}

void turnPowerOffSafely()
{
    bool pi_running;
    Serial.println("ACTION: Turn off power safely");
    pi_state = ePI_SHUTTING_DOWN; 
    SleepyPi.piShutdown(kPI_CURRENT_THRESHOLD_MA);   
    // Wait until it is shutdown
    while(pi_running == true) {
        pi_running = SleepyPi.checkPiStatus(kPI_CURRENT_THRESHOLD_MA,false);
        delay(1000);   
    }
    turnPowerOffNow(); 
}

void printTimeNow()
{
    // Read the time
    DateTime now = SleepyPi.readTime();
    
    // Print out the time
    Serial.print("Time = ");
    print2digits(now.hour());
    Serial.write(':');
    print2digits(now.minute());
    Serial.write(':');
    print2digits(now.second());
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(now.day());
    Serial.write('/');
    Serial.print(now.month()); 
    Serial.write('/');
    Serial.print(now.year(), DEC);
    Serial.println();

    return;
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
