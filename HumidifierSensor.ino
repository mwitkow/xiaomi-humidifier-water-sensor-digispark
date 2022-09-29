#include <SendOnlySoftwareSerial.h>
#include <CapacitiveSensor.h>

#define SENDER_PIN 0
#define SENSOR_PIN 2

#define SAMPLES_NUMBER 1000
#define TIMEOUT_MS 500

#define MIN_READING 2850 // for 1M resistor and 1000 samples with P2 as sensor pin
#define MAX_READING 9200 // for 1M resistor and 1000 samples with P2 as sensor pin

byte packet[] = {0xFA, 0x29, 0x03, 0x00, 0x00, 0x00, 0x00, 0x14, 0x9A, 0x00, 0x00, 0x00, 0x03, 0x77, 0x72, 0x71, 0x03, 0x00, 0x6C, 0x4C, 0x3B, 0x03, 0x2F, 0x15, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x03, 0xE8, 0x00, 0x2c, 0x02, 0x6D, 0x37, 0xD2, 0x00};

byte checksum;
byte mappedValue;
long readingRaw;

CapacitiveSensor sensor = CapacitiveSensor(SENDER_PIN,SENSOR_PIN); // 1M resistor between pins 0 & 2, pin 2 is sensor pin
SendOnlySoftwareSerial outSerial(1); // Software TX serial on pin 1, the LED is also on this pin. No problem for serial, but not good for capacitive sensor.

void setup()                    
{
   sensor.set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibrate
   sensor.set_CS_Timeout_Millis(TIMEOUT_MS);
   outSerial.begin(9600);
}

void loop()                    
{    
  checksum = 0;
  packet[11] = 0;
  
  readingRaw = sensor.capacitiveSensorRaw(SAMPLES_NUMBER);
  
  if(readingRaw == -2) mappedValue = 125; // tank full, water touches both metal probes, capacitance measurement not possible
  else mappedValue = constrain(map(readingRaw, MIN_READING, MAX_READING, 0, 120), 0, 120); //map sensor reading to humidifier water level range (0 - 120)
    
  packet[11] = mappedValue; //12th byte represents water level value

  /* checksum calculation (A XOR B XOR C ... XOR 0xA0) */
  for(int i = 0; i < 42; i++)
  {
      checksum ^= packet[i];
  } 
  checksum ^= 0xA0;
  /* checksum calculation */
    
  packet[42] = checksum; //last byte checksum
  
  // Real output:
  outSerial.write(packet, sizeof(packet));

  // For calibration/debugging:
  //outSerial.println(readingRaw);

  delay(100);
}
