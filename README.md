# roombactl

Sends commands to a Roomba device using the Open Interface specification.

## Usage

    usage: roombactl -d device [-cprtv] [-l led] [-s schedule]

    	-d /dev/ttyUSB0
    	-c (clean) -p (poweroff) -r (reset) -t (set time) -v (verbose)
    	-l [check,dock,spot,debris,colour:[0-255],intensity:[0-255]]
    	-s [day:HH:MM[,day:HH:MM]...]

### Options

-c Initiate immediate clean.  Equivalent to pressing the "clean" button.

-d Specify the device used for serial communications.  Must be specified before
   other options.  If -d is not specified, the ROOMBA_DEVICE environment
   variable will be used if present.

-l Set LED state.  The tokens 'check', 'dock', 'spot' and 'debris' will light
   the corresponding LED.  The tokens 'colour:N' and 'intensity:M' will set
   the centre LED to the colour and intensity specified by N and M.  Passing
   -l an empty string will reset all LEDs to off.

-p Power off.

-t Set time.

-s Set cleaning schedule.  Takes a comma separated list of day:HH:MM tokens
   to start cleaning on that day and time, in 24 hour format.  Use of this
   option will automatically set the time.

-v Verbose.

## Examples

Enable scheduled cleaning on Tuesday and Friday at 2pm.

    roobactl -d /dev/ttyUSB0 -s tue:14:00,fri:14:00

Initiate immediate clean.

    export ROOMBA_DEVICE=/dev/ttyUSB0
    roombactl -c

## History

I bought Roomba 530 devices knowing they had scheduling hardware but not
having an interface to enable it.  I looked at the existing software
which seemed quite featureful, but the instructions started with "Install
this Java SDK" which seemed a little heavyweight for sending a handful
of specific bytes to a serial port.

## References

 * [Rooba 530 scheduling](http://www.robotreviews.com/chat/viewtopic.php?t=9235)
 * [Roomba Open Interface spec](http://www.irobot.lv/uploaded_files/File/iRobot_Roomba_500_Open_Interface_Spec.pdf)
 * [RooStick USB serial interface](
    https://www.sparkfun.com/products/retired/670) This is the USB serial
    adapter that I use, but it has since been discontinued.
