# Pool Manager Customization
Wilcome to the Pool Manager customization part.
In order to customize your own Pool Manager usage, you have to customize 3 files in the include directory:
- [PM_Parameters.h](## PM_Parameters.h) 
- [PM_Pool_Configuration.h](## PM_Pool_Configuration.h)
- [PM_Secrets.h](## PM_Secrets.h)

Those 3 files are already delivered with my own customization for my pool and my needs. In your case, you'll have your own material which could be a little bit different or less complete. If you have more needs, you'll probably have to add more devices and, for sure, some additional code to manage them.

## PM_Parameters.h 
This [file](PM_Parameters.h) contains 2 mains parts
- the hardware parameters
- The software parameters

### Hardware parameters
It is the list of parameters to customize Pool Manager such as:
- LCD device type and size
- Your known Wifi access points (pool manager wil connect on the strongest one)
- Web server configuration: IP address, port, gateway, DNS, ....
- Temperature sensor list with their name and address
- The list of used GPIO with their affectation

### Software parameters
It is the list of parameters to customize Pool Manager such as:
- Date and time default format (local and UTC time)
- Timezone of your location (see https://sites.google.com/a/usapiens.com/opnode/time-zones to get yours)
- Timeout, duration or frequency of the different sensors, display, etc ... 

## PM_Pool_Configuration.h 
This [file](PM_Pool_Configuration.h) contains:
- [The filtration duration per day abaqus](### Filtration duration abaqus)
- [The periods of the day when the filtration must be ON](### Filtration periods)
- [The pH min and max limits to either inject pH minus solution or raise an alert](### pH (potential hydrogen) limits)
- [The ORP (Oxidation-reduction potential) min and max limits to either inject Chlorine or raise an alert](### ORP limits)
- [The max pressure in the filtration circuit before raising an alert](### Filtration circuit pressure)

### Filtration duration abaqus
The filtration duration per day depends on lot of factors. The main one is the water temperature. 
In order to let you deciding the duration of the filtration, I propose you an abaqus where you can pilot precisly the number of hours, the filtration must be activated.

You have to adapt the default adapted for my pool, to yours.
Look for on internet, you'll find a lot of abaqus. Choose the one you think it will be the good one or build yours.

The abaqus consists on an array of triple values: range of temperatures (min and max) and a duration in hours.

Here are the rules to follow for filling up the abaqus:
- Rule 1 : the lowest temperature must be -20 and the highest 99
- Rule 2 : temperatures in the temperature ranges must be consecutives (aka TempMin of a line must be equal to TempMax of the previous line)
- Rule 3 : the filtration duration of consecutive lines must increase 

For example, here is the abaqus for my pool :
```
static std::vector<PM_FiltrationDuration> PM_FiltrationDuration_Abaqus = {
// { TempMin , TempMax, FiltrationTime }  
  {-20, 14,  2} ,       //             <14°C  ==>  2 hours
  { 14, 15,  3} ,       //  >=14°C and <15°C  ==>  3 hours
  { 15, 16,  4} ,       //  >=15°C and <16°C  ==>  4 hours
  { 16, 17,  5} ,       //  >=16°C and <17°C  ==>  5 hours
  { 17, 18,  6} ,       //  >=17°C and <18°C  ==>  6 hours
  { 18, 19,  7} ,       //  >=18°C and <19°C  ==>  7 hours
  { 19, 20,  8} ,       //  >=19°C and <20°C  ==>  8 hours
  { 20, 21,  9} ,       //  >=20°C and <21°C  ==>  9 hours
  { 21, 22, 10} ,       //  >=21°C and <22°C  ==> 10 hours
  { 22, 24, 11} ,       //  >=22°C and <24°C  ==> 11 hours
  { 24, 26, 12} ,       //  >=24°C and <26°C  ==> 12 hours
  { 26, 28, 13} ,       //  >=26°C and <28°C  ==> 13 hours
  { 28, 30, 14} ,       //  >=28°C and <30°C  ==> 14 hours
  { 30, 32, 15} ,       //  >=30°C and <32°C  ==> 15 hours
  { 32, 99, 16}         //  >=32°C            ==> 16 hours
};
```
I tried another possibility found on internet provide by [Desjoyaux](https://www.desjoyaux.fr/faq/combien-de-temps-pour-la-filtration-de-ma-piscine/#:~:text=La%20r%C3%A8gle%20la%20plus%20simple,filtrer%2012%20heures%20par%20jours) (a famous French seller of pool) but it does not give satisfaction for my pool.
```
  static std::vector<PM_FiltrationDuration> PM_FiltrationDuration_Abaqus = {
  // { TempMin , TempMax, FiltrationTime }  
  {-20, 10,  2} ,       //             <10°C  ==>  2 hours
  { 10, 12,  4} ,       //  >=10°C and <12°C  ==>  4 hours
  { 12, 16,  6} ,       //  >=12°C and <16°C  ==>  6 hours
  { 16, 24,  8} ,       //  >=16°C and <24°C  ==>  8 hours
  { 24, 27, 12} ,       //  >=24°C and <27°C  ==> 12 hours
  { 27, 30, 20} ,       //  >=27°C and <30°C  ==> 20 hours
  { 30, 99, 24}         //  >=30°C            ==> 24 hours
  };
```
### Filtration periods
You can spread the filtration duration all along the day based on periods.
A period of time is a couple of start time and end time.
The duration of the filtration is spread on these periods depending on the priority you affected to each period. 
If several periods have the same priority, and if the duration is less than the sum of the periods with this same priority, then the duration will be spread on these periods for the same time
NB: The filtration period is evaluated once a day at midnight just after the day change or at the first time of the day after a reboot
NB: It is better to filter during the daylight

The filtration period abaqus consists on an array of triple values: start and end times of the period (hour based) and a priority (lower value is the greater priority).
Here are the rules to follow:
- Rule 1: a period cannot recover another one
- Rule 2: all periods have to cover the whole 24h of the day (aka from 0h to 24h)

NB: The array does not need to be sorted and priority begin from 1.

```
static std::vector<PM_FiltrationPeriod> PM_FiltrationPeriod_Abaqus = {
// { Start , End, Priority }  
  {  8, 12,  1},  // The main priority 1 is allocated to the morning
  { 14, 20,  1},  // ... and for the afternoon (priority 1 too)
  { 20, 24,  2},  // The evening will be taken if the filtration time is greater than the sum of the morning and the afternoon period
  {  0,  8,  3},  // The night period is considered as less priority than the light day.
  { 12, 14,  4}   // During lunch time, the filtration is on only when necessary 
};
// Example 1: if the duration of the filtration is  6h per day, the filtration will at 8h-11h and 14h-17h as the morning and the afternoon have the same priority
// Example 2: if the duration of the filtration is 12h per day, the filtration will at 8h-12h, 14h-20h and 20h-22h
// Example 3: if the duration of the filtration is 16h per day, the filtration will at 0h-2h, 8h-12h, 14h-20h and 20h-24h
// Example 4: if the duration of the filtration is 23h per day, the filtration will at 0h-2h, 8h-12h, 12h-13h, 14h-20h and 20h-24h
```

### pH (potential hydrogen) limits 
The pH level of pool water is a measure of its acidity. It is noted on a scale of 0 to 14, with a pH below 7.0 indicating the water is acidic. A pH of 8.0 means the water is basic or alkaline. Precisely between these two points is the proper pool pH level: 7.4 to 7.8.

There are five main reasons why keeping your pool water in the target pH range is so important.

1. Low pH causes eye and skin irritation
Acidic water not only stings the eyes and nasal passages, it strips the body of its natural oils leaving skin dry and itchy, and hair brittle. Interestingly (and unfortunately) the same physical issues can arise if pH gets too high. The human body is naturally near the neutral point on the scale.

2. High pH in pool inactivates chlorine
The further your pH level goes into the basic or alkaline range, the less effect your chlorine will have. Consequently, you find yourself spending far too much money on chlorine and still having water that isn’t clear.

3. Unbalanced pH can damage vinyl liners. 
With low or high pH, a vinyl liner can actually grow, creating unsightly wrinkles by absorbing water (up to 10% of its weight). If pH gets too high, a vinyl liner will also age more rapidly and have to be replaced sooner.

4. Low pH in pool can lead to corrosion. 
Acidic pool water can begin to corrode everything from pumps and other mechanical systems to ladders, slides and diving board bases. Low pH water can even corrode or etch the plaster in gunite pools.

5. High pH causes cloudy water.
Many minerals come out of solution as pH rises. This can lead to murky water and mineral deposits.

Even if the min and max values are commonly admitted, you can choose your own limits.
To do that you can customize **PM_pH_Min** and **PM_pH_Max** variables.
```
const float PM_pH_Min = 7.4;
const float PM_pH_Max = 7.8;
```

### ORP limits
Oxidation-Reduction Potential (ORP) measures the true efficacy of the oxidisers present in the water in millivolts. An ORP reading of 700mV to 720mV is considered adequate for fast and effective disinfection in pools.
ORP will rise and fall according to the level of cyanuric acid, pH and contaminants in the water. For example, a pool showing a Free Chlorine level of 2ppm may show an ORP reading of 720 at a pH level of 7.4. As the pH level changes so too will the ORP. At a pH of 7.8 the ORP may read below 700 or at a pH of 7 may read above 750.
Nevertheless, the ORP measure depends on the sensor you use. 
so you'll have to do some tests on your pool with your own sensors to find the ideal range of ORP.

I propose two values to be customized:
1. **PM_ORP_Min** (in mV) for triggering the injection of Chlorine. Under this value, an alert will be also raised.
2. **PM_ORP_Max** (in mV) for trigerring the stop of injection of Chlorine.
```
const int PM_ORP_Min = 650;
const int PM_ORP_Max = 750;
```

### Filtration circuit pressure
The pressure of the filtration circuit is taken with a pressure sensor installed on the filtration group.
This pressure is near null when the filtration is OFF, but between 1 and 2 bars when it is ON.
In case of surpressure, an alert must be raised on the buzzer and on the web site.
The pressure values are taken into account to control if the filtration is really ON when it should be and also to count the real duration of the filtration during the day as the filtration pump can be set ON by other system not controlled by Pool Manager application like a heater or a people to force the water circulation.

I propose two values to be customized:
1. **PM_Pressure_Min** (in hPa) Over this value, the system considers the filtration is ON. Under, the filtration is OFF.
2. **PM_Pressure_Max** (in hPa) for trigerring an alert for over-pressure on the system.
```
const float PM_Pressure_Min = 0.5;
const float PM_Pressure_Max = 2.0;
```

## PM_Secrets.h 
In order to store, the different passwords or secret credentials used in Pool Manager, you have to create a file named **PM_Secrets.h** in the same directory as the [PM_Secrets_Example.h](PM_Secrets_Example.h). To do so, copy the example file with the name: **PM_Secrets.h** and changed the differents values corresponding to your needs.

For example, in the file: [PM_Parameters.h](PM_Parameters.h), you provided earlier the list of your known Wifi networks.
For each of them, you had to provide their SSID and the PASSWORD. 
So either you provided in place those credentials, or you can use variables and put their values in the PM_Secrets.h file you just created.
