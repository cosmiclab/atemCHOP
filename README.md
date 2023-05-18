# atemCHOP
TouchDesigner plugin for controlling bmd atem switchers
This code uses the structure of camikura's atemCHOP as a starting point. While camikura's code uses the UDP protocol from Skaarhoj, which is old and unmaintained, we've started replacing that protocol with functions from the official BlackMagicDesign Switcher SDK, which should hopefully be more stable with new releases of BMD switcher firmware.

The development is still in the beginning stages and only building for Windows is supported. The following features have been implemented:

- set ip address of the atem switcher you want to connect to with the atem IP parameter.
- set program input for a Mixer Effect(ME) by inputting the message cpgi* into the atemCHOP, where * = the number of the ME.
- set preview input for an ME by inputting cpvi* into the atemCHOP, where * = the number of the ME.
- cut from program input to preview input (and swap them) by inputting dcut* into the atemCHOP. This should be input as a momentary value.
- auto-fade from program input to preview input (and swap them) by inputting daut* into the atemCHOP. This should be input as a momentary value.
- set position of the fader by inputting values between 0 and 1000. Nb. when the fader hits 1000 the ME's program and preview inputs will be swapped and you'll want to start fading again from 0 (not from 999).
- see CHOP output values for the program and preview inputs
- tries to reconnect periodically when disconnected
