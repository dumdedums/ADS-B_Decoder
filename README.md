# Yet Another ADS-B Decoder

## Why?
There are much better tools out there to decode ADS-B signals from SDRs, take for example [readsb](https://github.com/Mictronics/readsb), so why create a new one?
Well, I want to create this tool as a project to educate and train myself on decoding signals from an SDR in general, and I have an interest in both radios and airplanes.
This project probably never reach the capabilities of other existing projects, and that is ok. I only plan on making this program for my Raspberry Pi 3b and RTL-SDR V4,
but I might try and make it work for Windows as well since I will be doing most of my programming on a Windows system.

## What Exactly Does This Project Entail?
This project should contain three major components. The first component will be the communication with the SDR, which then requires me to pull the 1090 Mhz signal feed, identify ADS-B broadcast frames,
convert it into binary frames or messages, and send that data to be parsed.
The second component will be parsing te ADS-B message frames, and logging the broadcast messages to CSV file. The third component will be displaying the logged data as an image.

### Communication with the SDR
This will definitely be the most difficult part of the project, but the good news is that the SDR-RTL V4 already has drivers that exist.
This part of the project will require me to interact with the SDR driver to obtain a waveform of the 1090 Mhz signal.
I will then have to identify the start of ADS-B broadcast messages, after that I will have to extract the binary data using the rising and falling edges of the signal, along with the timing after the initiation of the broadcast.
I will then have to log each ADS-B message in a file. I am reading up various sources on how to parse the ADS-B messages from the analog signal. I am currently reading up on [The 1090Mhz Riddle](https://mode-s.org/1090mhz/index.html).

### Taking the ADS-B Message Frames, Deciphering and Logging
The reason why this is a separate component is because it is the easiest to potentially debug, as I can input test ADS-B frames, and I can then see the outputted log data in the CSV format and in the console.
This component should be platform independent, unlike the first component, and therefore by making it separate it will make the build process easier for *potentially* multiple systems.
This component is also the first component that you will see progress on. The components are ordered from the lowest level (as in closest to the hardware) up,
not order of completion.

### Displaying the Data
I am still unsure of how to impement this, but this component will parse the logs into an image (a map) of the locations of all the airplanes in the area.
I will probably try and use [GMT (Generic Mapping Tools)](https://github.com/GenericMappingTools/gmt) to help me out with this.
I currently only plan on creating static maps showing the most recent locations of each plane, the inputs of this component will include things such as showing *all* location logs of planes,
or possibly paths if GMT makes this easy enough, along with a timeout so extremely old location data doesn't show up. Maybe even show locations for a time frame.
