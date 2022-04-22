# Dist2ObsLogger

Dist2ObsLogger is a device that uses a HC-SR04 ultrasonic distance sensor and M5Stack basic model to log the distance between the sensor and an obstacle.
The pin connections between the sensor and the M5Stack is shown below.

| HC-SR04 | M5Stack |
| :---: | :---: |
| Echo  | 5 |
| Trig  | 2 |
| Gnd | G |
| Vcc  | 5V |

The device first connects to a 2.4 Gz Wi-Fi connection.
This sends the device to the idle state. 
Next, by pressing the button B, distance data collection can be started.
At the press of button B, the speaker of the M5Stack emits a beep indicating that it has started the data collection.
Along with each distance data point, it also records the number of milliseconds elapsed since the beep was emitted.
By pressing the button B again the data collection can be stopped. 
At the sametime, M5Stack connects to a NTP server via the Wi-Fi connection and collects a timestamp. (With accuracy upto a second)
This becomes the filename for the file that contain the collected data.
The collected data is then saved in the csv format in a microSD card.
