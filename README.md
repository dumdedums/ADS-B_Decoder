# Yet Another ADS-B Decoder

## Current Project Situation
Currently the project can decode hex messages written into a log file by `rtl_adsb.` or `dump1090 --raw` (I am unsure if dump1090 dumps the hex messages in the
same format of *\*\<hex_message\>;*). You can also pipe live input by putting a `-` in place of the filename option when using `-b` or `-p`.
The current stdout display format fits within an 80 character display (barely). I might try clearing the screen with each update so that the output
is less cluttered and at the bottom of the screen.\
The current plan is to make sure that live piped data is correctly read and logged, and then to work on displaying the data as an image,
and lastly to be able to read the binary data from pipes or files along with interacting with the RTL-SDR using the drivers.

## Building and Using the Project
main \[\-d\]\[\-r *latitude* *longitude*\]\[\-p|\-b *filename*\]\[\-s *filename*\]\[\-c *size*\]\
\-r *latitude* *longitude*\
	Change the relative latitude and longitude to your location. (The default location is O'Hare Airport.)\
\-p *filename*\
	Specify an input file that contains a hex message data dump. This could be FIFO file or "\-" for stdin.\
\-b *filename*\
	Input file for binary stream of I and Q values. Filename syntax is the same as \-p.\
\-s *filename*\
	Specifies a save file to save data in a CSV format. The ordering is ICAO, callsign, aircraft type, latitude, longitude, track, speed, altitude, vertical rate, timestamp.\
\-d\
	Turns on debug mode which prints extra messages (useless and lots of clutter).
\-c *size*\
	Specifies the size of the airplane cache (how many airplanes can be tracked at once before overwriting old airplane entries).
	If logging is turned on the whole cache is logged at the same time the display is updated, and log entries are appended, not erased.\
If no options are used the program will try and communicate with the RTL-SDR directly (Not yet implemented).

## Why?
There are much better tools out there to decode ADS-B signals from SDRs, take for example [readsb](https://github.com/Mictronics/readsb), so why create a new one?
Well, I want to create this tool as a project to educate and train myself on decoding signals from an SDR in general, and I have an interest in both radios and airplanes.
This project probably never reach the capabilities of other existing projects, and that is ok. I only plan on making this program for my Raspberry Pi 3b and RTL-SDR V4,
but I might try and make it work for Windows as well since I will be doing most of my programming on a Windows system.

## What Exactly Does This Project Entail?
This project should contain three major components. The first component will be the communication with the SDR, which then requires me to pull the 1090 Mhz signal feed,
identify ADS-B broadcast frames,
convert it into binary frames or messages, and send that data to be parsed.
The second component will be parsing te ADS-B message frames, and logging the broadcast messages to CSV file. The third component will be displaying the logged data as an image.

### Communication with the SDR
This will definitely be the most difficult part of the project, but the good news is that the SDR-RTL V4 already has drivers that exist.
This part of the project will require me to interact with the SDR driver to obtain a waveform of the 1090 Mhz signal.
I will then have to identify the start of ADS-B broadcast messages, after that I will have to extract the binary data using the rising and falling edges of the signal,
along with the timing after the initiation of the broadcast.
I will then have to log each ADS-B message in a file. I am reading up various sources on how to parse the ADS-B messages from the analog signal. I am currently reading up on
[The 1090Mhz Riddle](https://mode-s.org/1090mhz/index.html).

### Taking the ADS-B Message Frames, Deciphering and Logging
The reason why this is a separate component is because it is the easiest to potentially debug, as I can input test ADS-B frames,
and I can then see the outputted log data in the CSV format and in the console.
This component should be platform independent, unlike the first component, and therefore by making it separate it will make the build process easier for *potentially* multiple systems.
This component is also the first component that you will see progress on. The components are ordered from the lowest level (as in closest to the hardware) up,
not order of completion. Although there is a description of CPR (Compact Position Reporting) in The 1090Mhz Riddle, there is an in depth paper about the formulae
[here](https://shemesh.larc.nasa.gov/fm/papers/VSTTE2017-draft.pdf). (This paper only describes the airborne version of the algorithm.)

### Displaying the Data
I am still unsure of how to impement this, but this component will parse the logs into an image (a map) of the locations of all the airplanes in the area.
I will probably try and use [GMT (Generic Mapping Tools)](https://github.com/GenericMappingTools/gmt) to help me out with this.
I currently only plan on creating static maps showing the most recent locations of each plane,
the inputs of this component will include things such as showing *all* location logs of planes,
or possibly paths if GMT makes this easy enough, along with a timeout so extremely old location data doesn't show up. Maybe even show locations for a time frame.
