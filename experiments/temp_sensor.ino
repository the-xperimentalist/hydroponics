#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
double val=0.0;

void setup(void)
{
  Serial.begin(9600);
  Serial.println("Dallas Temperature Library");
  sensors.begin();
}

void loop(void)
{
  Serial.print("Requesting Temp: ");
  sensors.requestTemperatures();
  val=sensors.getTempCByIndex(0);
  Serial.println("DONE");
  Serial.print("Temperature is: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.print(val);
  delay(1000);
}
