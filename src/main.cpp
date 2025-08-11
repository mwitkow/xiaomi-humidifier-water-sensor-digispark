#include <SendOnlySoftwareSerial.h>
#include <CapacitiveSensor.h>
#include <EEPROM.h>

// #define DEBUG 1

#define SENDER_PIN 0
#define SENSOR_PIN 2
#define AUTOCAL_PIN 3
#define AUTOCAL_HYSTERESIS 50

#define SAMPLES_NUMBER 1000
#define TIMEOUT_MS 500

#define NO_CHANGE_SENSING -2 // value returned when no change in SAMPLE NUMBER happened

// for 1M resistor and 1000 samples with P2 as sensor pin, defaults used when no calibration has been performed
#define FAILSAFE_MIN_READING 3000
#define FAILSAFE_MAX_READING 9000
#define EEPROM_SUM_XOR 123456789

byte packet[] = {0xFA, 0x29, 0x03, 0x00, 0x00, 0x00, 0x00, 0x14, 0x9A, 0x00, 0x00, 0x00, 0x03, 0x77, 0x72, 0x71, 0x03, 0x00, 0x6C, 0x4C, 0x3B, 0x03, 0x2F, 0x15, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x03, 0xE8, 0x00, 0x2c, 0x02, 0x6D, 0x37, 0xD2, 0x00};

byte mappedValue;
long readingRaw = 0;
long prevReading = 0;
long minreading = FAILSAFE_MIN_READING;
long maxreading = FAILSAFE_MAX_READING;
bool calibrationRunning = false;

// Function prototypes
void sendpacket(byte value);
void readcaldata();
void writecaldata();

CapacitiveSensor sensor = CapacitiveSensor(SENDER_PIN, SENSOR_PIN); // 1M resistor between pins 0 & 2, pin 2 is sensor pin
SendOnlySoftwareSerial outSerial(1);                                // Software TX serial on pin 1, the LED is also on this pin. No problem for serial, but not good for capacitive sensor.

void setup()
{
  sensor.set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibrate
  sensor.set_CS_Timeout_Millis(TIMEOUT_MS);
  outSerial.begin(9600);
  pinMode(AUTOCAL_PIN, INPUT); // digispark has pullup resistor on pin 3, no need to use internal one.
  if (digitalRead(AUTOCAL_PIN) == 0)
  {
    readingRaw = 0;

    // Send full data for 6.8 seconds, to show we are in calibration mode.
    for (int i = 0; i < 68; i++)
    {
      sendpacket(120);
      delay(100);
    }

    // Measure and send full data for 3.2 seconds
    for (int i = 0; i < 32; i++)
    {
      readingRaw += sensor.capacitiveSensorRaw(SAMPLES_NUMBER);
      if (readingRaw < 0)
        break;
      sendpacket(120);
      delay(100); // Take new sample every 100 ms
    }

    if (readingRaw > 0)
    {
      // Start calibration
      calibrationRunning = true;
      minreading = (readingRaw >> 5) + AUTOCAL_HYSTERESIS;
    }
    else
    {
      // Something wrong with circuit or tank full. Show 3 bars forever
      while (true)
      {
        sendpacket(75);
        delay(100);
      }
    }
  }
  else
  {
    readcaldata();
  }
}

void loop()
{
  prevReading = readingRaw;
  readingRaw = sensor.capacitiveSensorRaw(SAMPLES_NUMBER);

  if (calibrationRunning && readingRaw == NO_CHANGE_SENSING && prevReading > minreading)
  {
    maxreading = prevReading - AUTOCAL_HYSTERESIS;
    writecaldata();
    calibrationRunning = false;
  }

  if (readingRaw == NO_CHANGE_SENSING)
    mappedValue = 125; // tank full, water touches both metal probes, capacitance measurement not possible
  else
    mappedValue = constrain(map(readingRaw, minreading, maxreading, 0, 120), calibrationRunning ? 24 : 0, 120); // map sensor reading to humidifier water level range (0 - 120), 24-120 when calibrating.

  // Output value
  sendpacket(mappedValue);

  // For debugging:
#ifdef DEBUG
  outSerial.print(readingRaw);
  outSerial.print(" ");
  outSerial.print((long)mappedValue);
  outSerial.print(" ");
  outSerial.print(calibrationRunning);
  outSerial.print(" ");
  outSerial.print(minreading);
  outSerial.print(" ");
  outSerial.println(maxreading);
#endif

  delay(100);
}

void sendpacket(byte value)
{
  byte checksum = 0;
  packet[11] = value; // 12th byte represents water level value

  /* checksum calculation (A XOR B XOR C ... XOR 0xA0) */
  for (int i = 0; i < 42; i++)
  {
    checksum ^= packet[i];
  }
  checksum ^= 0xA0;
  /* checksum calculation */

  packet[42] = checksum; // last byte checksum

#ifndef DEBUG
  outSerial.write(packet, sizeof(packet));
#else
  outSerial.print("Packet: ");
  outSerial.println((long)value);
#endif
}

void readcaldata()
{
  long eepmin;
  long eepmax;
  long eepsum;
  EEPROM.get(0, eepmin);
  EEPROM.get(sizeof(eepmin), eepmax);
  EEPROM.get(sizeof(eepmin) + sizeof(eepmax), eepsum);
  if (eepsum == ((eepmin + eepmax) ^ EEPROM_SUM_XOR))
  {
    minreading = eepmin;
    maxreading = eepmax;
  }
}

void writecaldata()
{
  long eepsum;
  eepsum = ((minreading + maxreading) ^ EEPROM_SUM_XOR);
  EEPROM.put(0, minreading);
  EEPROM.put(sizeof(minreading), maxreading);
  EEPROM.put(sizeof(minreading) + sizeof(maxreading), eepsum);
}
