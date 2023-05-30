# atemCHOP
TouchDesigner plugin for controlling bmd atem switchers
This code uses the structure of camikura's atemCHOP as a starting point. While camikura's code uses the UDP protocol from Skaarhoj, which is old and unmaintained, we've started replacing that protocol with functions from the official BlackMagicDesign Switcher SDK, which should hopefully be more stable with new releases of BMD switcher firmware. Instead of message inputs, it now works with operator parameters.

Requirements:

- Windows 10 (probably works on 11)
- sdk files you need are included in the atemSDK folder but you can get newer versions at https://www.blackmagicdesign.com/developer/product/atem

Usage:

- Copy Release/AtemCHOP.dll into a folder called Plugins in the same directory as your toe.
- Select it from the Custom Operators menu.
- Set ip address.

Some features are still in developmemt and only building for Windows is supported. The following features have been implemented:

- set ip address of the atem switcher you want to connect to with the General>Atem IP parameter.
- set program input for a Mixer Effect(ME) by setting ME#>Program# where # is the number of the ME. (There are pages for four MEs, though how many are useable depends on your switcher) 
- set preview input for an ME by setting ME#>Preview# where # is the number of the ME. 
- fade between program and preview input (and swap them) with ME#>Fader# (0.0-1.0). With General>FaderMirroring, you can toggle whether the swap point alternates between 0.0 and 1.0 or not.
- cut from program input to preview input (and swap them) with ME#>Cut#
- auto-fade from program input to preview input (and swap them) with ME#>Auto#
- cut and auto for up to two DownstreamKeys on the DSK page
- see CHOP output values for the program and preview inputs and the DSK status (as 0 or 1)
- tries to reconnect periodically when disconnected
