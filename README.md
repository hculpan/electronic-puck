# Electronic Puck
This is a repository for my electronic puck project. This is based on
an existing project, [LED Reactive Light-Up Hockey Puck in MakeCode](https://learn.adafruit.com/led-hockey-puck/overview).

I am using the 3D objects as presented in the project, but with some modifications. I have modified the bottom piece
to accomodate the SW-18010OP vibration sensor (more on this below). I've also basically closed one of the two openings, 
the one for the JST connector, because it can be charged through the USB micro connector.

I have two main overall design changes from the original project. First, I'm using the Adruino IDE to program the device, 
not MakeCode. And, second, I've added a SW-18010P vibration sensor. If this sensor is not triggered within 5 minutes, the 
device will go into a deep sleep, awaking only when the sensor goes off again. Thus there's no reason to unplug the JST
connector to the battery and no need for an on/off switch.

In the description of the project I linked to above, it referred to an earlier project the author based his design on.
This earlier project used an accelerometer for much the same reason as I'm using the vibration sensor, 
except it had the added benefit of informing the microcontroller of which direction it was moving, so the pixel display 
could reflect this. The reason I decided not to go this route is because I'm using the STL files from the project I based 
this on, and that didn't have enough room to fit an accelerometer. The Circuit Playground Express is quite a big 
development board, and for a second attempt I'd probably use a different, smaller microcontroller with an external 
NeoPixel ring. 