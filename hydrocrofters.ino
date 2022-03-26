
//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP8266_HARDSERIAL_POINT

#include <RemoteXY.h>

// RemoteXY connection settings 
#define REMOTEXY_SERIAL Serial
#define REMOTEXY_SERIAL_SPEED 115200
#define REMOTEXY_WIFI_SSID "RemoteXY"
#define REMOTEXY_WIFI_PASSWORD "123456789"
#define REMOTEXY_SERVER_PORT 6377
#define REMOTEXY_ACCESS_PASSWORD "123456789"


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 98 bytes
  { 255,0,0,33,0,91,0,16,31,1,67,4,12,29,20,5,2,26,11,129,
  0,13,8,39,6,17,72,121,100,114,111,99,114,111,102,116,101,114,115,0,
  129,0,12,23,30,5,17,84,101,109,112,101,114,97,116,117,114,101,0,129,
  0,12,44,18,6,17,84,68,83,0,67,4,12,51,20,5,2,26,11,129,
  0,12,64,18,6,17,112,72,0,67,4,12,72,20,5,2,26,11 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // output variables
  char tempVar[11];  // string UTF8 end zero 
  char tdsVar[11];  // string UTF8 end zero 
  char phVar[11];  // string UTF8 end zero 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

/////////////////////////////////////////////
//           START Temperature include     //
/////////////////////////////////////////////
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
double tempValue=0.0;

// Temperature Feedback pins
int tempHighLED = 3;
int tempNormalLED = 4;
int tempLowLED = 5;

// Temperature threshold
double tempLow = 15.0;
double tempHigh = 30.0;

/////////////////////////////////////////////
//            END Temperature include      //
/////////////////////////////////////////////

/////////////////////////////////////////////
//            START TDS include            //
/////////////////////////////////////////////

#define TdsSensorPin A1
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

/////////////////////////////////////////////
//             END TDS include             //
/////////////////////////////////////////////

// Temporary pH value
double phValue = 7.0;


void setup() 
{
  RemoteXY_Init (); 
  
  // Starting the temperature sensor
  sensors.begin();

  // Temperature feedback LEDs
  pinMode(tempHighLED, OUTPUT);
  pinMode(tempNormalLED, OUTPUT);
  pinMode(tempLowLED, OUTPUT);

  // Setting up the TDS sensor
  pinMode(TdsSensorPin, INPUT);
  
}

void loop() 
{ 
  RemoteXY_Handler ();
  
  // Updating the temperature sensor values
  sensors.requestTemperatures();
  dtostrf(tempValue, 0, 2, RemoteXY.tempVar);

  // Conditionally switching on the temp lights
  if(tempValue <= tempLow) {
    digitalWrite(tempLowLED, HIGH);
    digitalWrite(tempHighLED, LOW);
    digitalWrite(tempNormalLED, LOW);
  }
  else if (tempValue > tempHigh) {
    digitalWrite(tempHighLED, HIGH);
    digitalWrite(tempLowLED, LOW);
    digitalWrite(tempNormalLED, LOW);
  }
  else {
    digitalWrite(tempNormalLED, HIGH);
    digitalWrite(tempLowLED, LOW);
    digitalWrite(tempHighLED, LOW);
  }

  // Updating the TDS sensor values
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value

      dtostrf(tdsValue, 0, 2, RemoteXY.tdsVar);
   }

   // Defaulting ph to 7.0
   dtostrf(phValue, 0, 2, RemoteXY.phVar);
   
}

/////////////////////////////////////////////
//        HELPER FUNCTIONS TDS             //
/////////////////////////////////////////////

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}
