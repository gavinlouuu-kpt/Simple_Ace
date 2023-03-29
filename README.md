# Piccolo

## About the project

This project developed for Open-souce Gas Chramtography (OpenGC). OpenGC is designed to separate and analyze the components of complex chemical mixtures. Unlike traditional GC systems that are proprietary and closed, OpenGC is designed to be transparent, customizable, and accessible to the broader scientific community.

OpenGC have the potential to democratize access to advanced chromatography technology, particularly in resource-limited settings or for researchers who may not have access to traditional proprietary systems. They also offer a way to promote collaboration and innovation in the field of analytical chemistry, and to advance the development of new analytical techniques and methods.

## About Piccolo

Piccolo is an miniature gas chrmatography device to evaluate sensor sensitivty. It integrated gas flow control, self-regulating heating element and modular sensor attachment to provide a wide range gas detection.

![TAI_8477](https://user-images.githubusercontent.com/78717679/222635831-722b8976-42af-47da-9b0c-7ac461033310.jpg)

Gas samples are highly complex in nature. The traditional way to evaluate sensors sensitivity exposes them in ideal environment compossed by high concentration target gas and non-reactive carrier gas. However, sensors in real-application has **1. much lower concentration compare to laboratory scenario** and **2. much noises contributed by surrounding environment**. Target gas are mixed up with surrounding gas, for example water vapour, CO2, N2,etc. Therefore, in application scenario, observing sensors' response against a certain gas consistuents would expect high intereference from the environment. This could be found in MEMS sensor and PID (Photon Ionization detector) sensor, which have limited selectivity given the nature of architecture.

Piccolo is porgrammed with C language, under Arduino framework. Clone this project into your local repository on visual studio code to customize the system.

## How it works

Piccolo seperates gas mixture by ambient air dilution and heated elusion and reveals your sensors' sensitivity toward the target gas.

Exposing gas samples to Piccolo, a diapragm pump draws and separates gas particles by continous ambient air influx. Passing through the cloumn, Tenex filament segregates gas particles regarding their polarity while the column path separates them regarding their size. Different gas, therfore, possesed diffrerent retention time, the time required to migrate through the system, before being captured by the sensor, giving rise to the muti-peak response profile.

By measuring the rention time of different gas in varying temerpature and flow rate using solution of known concentration, each peak could be reference accordingly.

## Feature Describtion

- Tenex elusion column
- self-regulating heating column
- USB-C charging, Serial communication, and Compilation protocol
- Attachable sesnor testing module
- TFT screen navigationpanel
- 3.7 V Li rechargeable battery
- Gas channelling mouth piece
- Gas channelling Sensor cover

## Board information

The device is installed with ESP32 PICO D4 MCU chip. It supports SPI Flash File System (SPIFFS), I2C communciation protocol and Wifi protocol. X V is required to power the system and control logic.

## Get started

Fix sensor cover onto the board and connect to the outlet of diaphragm pump. Please ensure the sensor cover is the outlet of the gaseous pathway.

Insert sensor PCB board into the bottom slot (You can order on the website or produce by yourself. The schematics are included).

Enter developer mode in setting page, press "default setting" to check the flow rate and column temperature. The default setting is 88mL/min and 47<sup>o</sup>C. This parameter is specifed for acetone observation.

Both parameters are customizable based on the concentration and size of the target gas. Please refer to the elution table for optimized gas sepration.

![Settings](https://user-images.githubusercontent.com/78717679/223623694-25d4c44a-1d6f-4687-bf49-55f7ef92d335.jpg)
![default setting](https://user-images.githubusercontent.com/78717679/223623646-0b8d332c-53ab-4d40-9056-9b5e2709b582.jpg)

Switch on the device and select caliibration in setting page.

![calibration](https://user-images.githubusercontent.com/78717679/223623664-d43c9aa0-7ff5-499a-855d-edf785c04802.jpg)

Prepare your reference liquid solution.

Pressing the start button would setup the internal system. After the screen instructed "Sample in 3,2,1" , expose your reference solution into the gas entrance for three seconds. After another three seconds, the screen would instruct you to stop the exposure and move away the device from the reference solution.
The calibration take 45 seconds, please wait patiently until result is prompted on the screen.

Press live plot to start monitoring your sensor behaviour. The moving graph respsents the adc value of the sensor across a 3.3V voltage divider. The interaction with particles would lead to rise of drop of the sensor response (less resistance increase adc value, vice versa).

![Live plot](https://user-images.githubusercontent.com/78717679/223623717-679ed078-0bf3-4168-afa9-610c760599b1.jpg)
![Liveplot](https://user-images.githubusercontent.com/78717679/223653145-31181973-6d11-4407-836a-fa1e5864021f.jpg)

There are several points to be noted about the sensor graph:

1. The baseline value is not fixed due to the unstable enivronment, but it negligilbe influence on the performance.

2. It is not recommended to perform mutiple exposure as this would interfere with the resposne profile. Sample again when the sensor response reach a stable baseline.

It is highly recommended to install "Serial Plot", a opensource serial monitor software for thorough monitoring. The system provided mutiple channels of data to monitor , including sensor response, but also humidity, battery power and temperature of sensor heater, board envrionment and heating column.

## About the team

King's Phase Technologies is a device manufacturer aimed at democratising gaseous informatics in biochemistry. We minimize laboratory gas chromatography mass spectrometry system to provide light-weight tools for gaseous analysis.

## License

=======

# Beagle -- miniature gas sensor validation device

## About

Beagle is

## Function

Beagle specifies in minture gas chromatography.

## Feature

Tenex elusion column
self-regulating temeprature system
USB-C charging, Serial communication, and compiling protocol
Plug-and-play sesnor testing module
TFT navigation panel
3.3V charging battery

## Assembly

## Components

## Application
