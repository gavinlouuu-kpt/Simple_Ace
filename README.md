# Piccolo
## About Piccolo
Piccolo is an miniature gas chrmatography device to evaluate sensor sensitivty against mixture of gas. It integrated gas flow control, self-regulating heating element and modular sensor attachment to provide a wide range gas detection. 

![TAI_8477](https://user-images.githubusercontent.com/78717679/222635831-722b8976-42af-47da-9b0c-7ac461033310.jpg)

Gas samples are highly complex in nature. The tradition way to evaluate sensors sensitivity exposes them in ideal environment with only relative high concenration of pure target gas and non-reactive carrier gas. However, sensors in real-application has **1. much lower concentration compare to laboratory scenario** and **2. much noises contributed by surrounding environment**. Target gas are mixed up with surrounding gas, for example water vapour, CO2, N2,etc. Therefore, in application scenario, observing sensors' response against a certain gas consistuents would expect high intereference from the environment. This could be found in MEMS sensor and PID (Photon Ionization detector) sensor, which have limited selectivity given the nature of architecture. 

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
- 4.2 V Li rechargeable battery
- Gas channelling mouth piece

## Get started
Insert sensor PCB board into the bottom slot (You can order on the website or produce by yourself. The schematics are included).

Enter developer mode in setting page, press "default setting" to check the flow rate and column temperature. The default setting is (120) and (6000). !They are customizable based on the concentration and size of the target gas. A faster flow rate has greater dilution effect, result in a weaker signal!. A greater column temperature reduce gas seperation and shorten the time of observation. 

Switch on the device and select caliibration in setting page. 

Prepare your reference liquid solution. 

Pressing the start button would setup the internal system. After the screen instructed "Sample in 3,2,1" , expose your reference solution into the gas entrance for three seconds. After another three seconds, the screen would instruct you to stop the exposure and move away the device from the reference solution.
The calibration take 45 seconds, please wait patiently until result is prompted on the screen.

Press live plot to start monitoring your sensor behaviour. The moving graph respsents the adc value of the sensor across a 3.3V voltage divider. The interaction with particles would lead to rise of drop of the sensor response (less resistance increase adc value, vice versa). 

There are several points to be noted about the sensor graph:

1. The baseline value is not fixed due to the unstable enivronment, but it negligilbe influence on the performance. 

2. It is not recommended to perform mutiple exposure as this would interfere with the resposne profile. Sample again when the sensor response reach a stable baseline.
    
It is highly recommended to install "Serial Plot", a opensource serial monitor software for thorough monitoring. The system provided mutiple channels of data to monitor , including sensor response, but also humidity, battery power and temperature of sensor heater, board envrionment and heating column.
 

## About the team
KingsPhase Technologies 

## License

