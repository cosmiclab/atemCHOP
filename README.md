# AtemCHOP
TouchDesigner plugin for controlling Blackmagic Design Atem switchers.
This code uses the official BlackMagicDesign Switcher SDK, which should be more stable with new releases of BMD switcher firmware.

Requirements:

- Windows 10 (probably works on 11)
- SDK files you need for building are included in the atemSDK folder but you can update them to the latest version at https://www.blackmagicdesign.com/developer/product/atem

Usage:

- Copy Release/AtemCHOP.dll into a folder called Plugins in the same directory as your TOE.
- Select it from the Custom Operators menu.
- Set IP address.

Some features are still in developmemt and only building for Windows is supported. The following features have been implemented:

- Set IP address of the Atem switcher you want to connect to with the General>Atem IP parameter.
- Set program input for a Mixer Effect(ME) by setting ME#>Program# where # is the number of the ME. (There are pages for four MEs, though how many are useable depends on your switcher) .
- Set preview input for an ME by setting ME#>Preview# where # is the number of the ME. 
- Fade between program and preview input with ME#>Fader# (0.0-1.0).
- Cut from program input to preview input (and swap them) with ME#>Cut#.
- Auto-fade from program input to preview input (and swap them) with ME#>Auto#.
- Cut and auto (with adjustable rate in frames) for up to two DownstreamKeys on the DSK page.
- See CHOP output values for the program and preview inputs and the DSK status (as 0 or 1).
- Tries to reconnect periodically when disconnected.
