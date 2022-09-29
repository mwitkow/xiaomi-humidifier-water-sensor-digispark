# xiaomi-humidifier-water-sensor-digispark
Xiaomi SmartMi Evaporative Humidifier water level sensor for digispark ATTiny85

## Description
This project is modified from the original to work with a digispark clone.

### Benefits
- Based on cheap clone
- Fits fully inside the case
- In-circuit programmable and calibratable

### Parts list
- Digispark clone, get the blue board with the microUSB socket from e.g. <a href="https://www.aliexpress.com/wholesale?SearchText=attiny85+blue">AliExpress</a>.
- USB to microUSB cable for loading the software
- 1M resistor
- USB to TTL adapter if you want to perform calibration

### Notes
This project uses P2 as the sense pin, so the calibration is different to the original upstream project. Probably because P2 is an analog pin.
If you are worried about this influence and want to have the same calibration as the original project, then you can use P0 as the sense pin and P2 as the sender, but the wire routing is less convenient.  

P1 is used for serial because it has a LED on it.

Sensor response is not very linear below 0.5L, but because below 0.5L the drum can't really reach the water anyway, it does not make any sense to linearize it:  
![Sensor linearization](pics/sensor-lin.png)

### Compiling
You need the CapacitiveSensor libary - just install it in Arduino.  
Also grab and install the SendOnlySoftwareSerial library here: https://github.com/nickgammon/SendOnlySoftwareSerial

## Step by step guide
1. Disassemble the humidifer and remove the power board (located in the back where AC cord goes in)
2. Desolder the blue rectangular component on the power board, marked U03. Good idea to clear some of the lacquer off, and after that desoldering wick and some wiggling of the pins should do it.
3. Cut off 2 pins of the header that came with your digispark and solder them to GND and 5V pads.
4. Connect wires to the TP and TXD pads.  
![PCB front](pics/pcb-front.jpg)
5. Solder one end of 1M resistor to P0 and clip the lead.
6. Fit and solder in the digispark.
- Angle the USB a little towards the top of the PCB.
- Leave longer leads on the through-hole wires on the digispark. On P1 to be able to attach the USB-TTL and on P2 to solder the resistor to the wire.  
![PCB finished](pics/pcb-finished.jpg)
7. Connect the sensor and the main ground on the back of the PCB, because the ATTiny85 obviously does not have a separate analog ground.  
![PCB rear](pics/pcb-rear.jpg)
8. (Optional calibration) Comment in the calibration code instead of the normal code and program the ATTiny85. Connect only sensor to power board, slide pins from TTL adapter to 5V, GND and TXD pins of ATTiny85 and perform min/max calibration. Note that minimum value should be recorded with the bottom base connected. If you completely disconnect the sensor the value will be much lower and you will not have correct empty container detection.
9. Program controller with normal code, and you are done.
