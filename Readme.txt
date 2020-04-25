	WS2813 based RGB LED plant grow light

Introduction:

Having interest in hydroponics, I started a DIY project for growing lettuce and tomatoes. A small 10L system with plans of further expansions, if I manage to get positive results. For hydroponics, a certain wattage and wavelength of RGB light is required. In terms of measurement/parameters, temperature, humidity, pH and EC measurements are important. Finally, timing of e.g. plants exposed to light, number of days since a plant has been growing are also handy things to have. 
As I am trying hydroponics for the first time, decided to implement a microcontroller-based (mc-based) system for my hydroponic setup, as an mc is ideal for timing, measuring and controlling of such a small-scale system. I decided to buy RGB LEDs and make a DIY light also, saving the overall system cost. A commercial starting series (<10W) LED grow light costs around 350 kroner (35 euros), whereas 1m, 60 LEDs WS2813 strip (16W) costs 120 kroner (12 Euros) and approx. 200 kroners (20 euros) for additional microcontroller-based LED driver, which is providing by other functionalities also. I started with a single 30 LEDs per meter WS2813 strip (9W power), however, I realized that it is nice to have at least 15W power. Therefore, I added 32 more LEDs but of different type (WS2811 in form of SW402 LED board for drones/quadcopters) due to cost and logistics reasons. Here are some facts about the cost and also tips for DIY hydroponic and light related systems, which I learned from my first batch/harvest of lettuce which was not complete but almost a failure due to slow growth. However, it gave me invaluable experience.

System:
The system measures temp.  and humidity of the environment. In addition, it keeps a record of max., min. and avg. value of these parameters for the previous day. It also acts as a LED light driver and timer keeper, with a clock (24 hrs format) and number of days functionality. I have not added pH and EC probes yet, due to cost reasons (approx. 1500 NOKs/ 150 euros for both), but if I do manage to get the hydroponic thingy working, I can add these two since the system is flexible. Following are components of the system:

EFM32GG based STK3800 kit, any mc e.g. Arduino nano could be used
DHT22 ambient sensor, x1 sensors
WS2813 RGB LED strip, 20 LEDs/m, x1 strip
SW402 boards, WS2811 based LED, x4 boards each with 8 LEDs
USB charger, 2.4 A x2 (approx. 25W)
Cost:
MC 20 Euros (200 NOK, 20 euros)
WS2813 RGB LED (100 NOK, 10 euros) (9W)
SW402 RGB LED board (100 NOK, 10 euros) (8.88W)
DHT22 sensor (70 NOKs, 7 euros)
USB charger (170 NOKs, 17 euros)
Testing a WS2813 based grow light for a hydroponic system. The reason for using a DIY LED grow light is cost of the light. 

Conclusion:
The repo shows C code for the overall system. Currently, I have approx. 18W LED light. My first batch of lettuce was not a success (I did manage to harvest some), but I guess the reason was too low light power (9W) and improper planting of sees in rockwool (at surface of rockwool, which should have been at a depth of 8-10 mm). However, I recently started a second batch with proper 18 W LED power. One difference I could tell is that my plants are growing relatively faster this time. I am still not sure whether WS2813 provide 100% percent of the required RB wavelengths for photosynthesis, since the datasheets are a bit vague but I think they are working quite OK for my 2nd batch. Finally comparison with a commercial system having same functionality: a commercial system with such wattage of LED and temp. and humidity measurement, the cost is around 2000 to 2500 kroner (200 to 250 euros), which is not flexible. And my system is highly flexible and is evolving constantly with new experiences and things I learn ??
