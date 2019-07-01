MakeSense
=========

This repository contains the source code developed for the ESRC funded project HomeSense.
It helps social researchers to set up a data collection network/platform by deploying sensors into indoor environment such as homes and offices.

Folder Structure
----------------
`server` - The source code to deploy on a Linux server for receiving, storing and visualising sensor data.

`sensor-box` - The source code for IoTEggs which collects data from the physical environment and sends them back to the data servers via HTTP.

`pi` - The source code to deploy on a Raspberry Pi (TM) for transferring energy data collected with CurrentCost (TM) devices.

`wakaama` - The source code of LWM2M based IoTEgg firmwares and data collectors. 
