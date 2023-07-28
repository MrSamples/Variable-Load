Custom SparkFun Variable Load
==========================================

* Up to 30 Volts (steps of milliVolts (30,001 different steps))
* Up to 4 Amps (steps of milliAmps (4,001 different steps))
* Limit of 15 Watts
* For Duration, Interval, and Log Time: steps of centiseconds (1 centisecond == 10 milliseconds)
* 128 KB usable/storable flash memory (22.222... days (32,000 minutes) (~2/3 month) worth of storage at 1 minute log time, ~7.5 months worth of storage at 10 minute log time)

Original functionality of the Variable Load Kit (VLK) allowed for a quiescent (constant and steady; "rest") current draw on a hooked-up battery.

I have added the functionality for the battery to be pulsed while still having an underlying quiescent (rest) current draw on said battery. Two different pulses can be set, with varying current draws, duration of pulse, and interval between each pulse on it. These pulses have also been grouped so that there can be a period of rest between the "group" of pulses in order to mimic a device waking up, doing something that requires the pulsing/using of the battery, and then going back into sleep state. There are also shut-off "zones" for each pulse: imagine a device in the field that tries to run a process 3 times but fails each time, so in order to save battery, the device stops trying to run this process until it recognizes that it's battery(ies) has(ve) been changed or charged. If a pulse on the VLK causes the connected battery to drop below the user-set minimum voltage for the pulse, then that pulse turns itself off similar to a recognizable brown out state and ceasing of the process that causes the strain on the device.

I have also added set profiles to be used on the board so that the VLK can be used without being connected to a computer but rather straight from a wall-plug. Trying to run a month long test and having to keep a computer on for that long is annoying to say the least.

The last main feature I have added to the Variable Load Kit is the ability to log data on the board itself. The minimum and maximum voltage between set times (ex: 10 minutes) are stored in the flash memory of the board itself. This way, after a test is conducted, the VLK can be turned off, hooked up to a pc, and its data can be read from the board and stored for analyzing.

I have also created a small C# program that connects to the board and reads the flash off of it, and stores it in a csv file. This way the user does not need to use a programmer to connect to the chip itself and read the literal flash into an Intel hex file, having to convert the hex values into decimal to interpret the data. I have placed this in a separate repository so that the building of the project does not break from a random C# project in the folder.

![PuTTY Screenshot of custom VLK Control Panel (https://imgur.com/a/UHOGVdm)](https://i.imgur.com/dlT7Rh2.png)

Screenshot of the serial port control panel if the user decides to use the board on a computer rather than standalone.

Follow SparkFun's tutorial to build the project and load this code into your Variable Load Kit.

I have not edited any of the README past this line. Under this line is all original text from SparkFun.

SparkFun Variable Load
==========================================

![Variable Load](https://cdn.sparkfun.com/assets/parts/1/2/4/7/9/14449-SparkFun_Variable_Load_Kit-01.jpg)

[*Variable Load (KIT-14449)*](https://www.sparkfun.com/products/14449)

The Variable Load is a digitally controllable load for testing power supplies.
It can test up to 30V at 4A, with a limit of 15W.

Repository Contents
-------------------
* **/Hardware** - All Eagle design files (.brd, .sch)
* **/Firmware** - Firmware files for this project.
* **/Production** - Test bed files and production panel files


Documentation
--------------

* **[Hookup Guide - Revised](https://learn.sparkfun.com/tutorials/variable-load-hookup-guide---revised)** - Latest basic hookup guide for the Variable Load.
* ***[Hookup Guide (RETIRED)](https://learn.sparkfun.com/tutorials/variable-load-hookup-guide)** - Basic hookup guide for the Variable Load for boards purchased purchased prior to 24 May 2018.*

License Information
-------------------

This product is _**open source**_! 

Please review the LICENSE.md file for license information. 

If you have any questions or concerns on licensing, please contact techsupport@sparkfun.com.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
