# Arduino WiFi Alarm Clock

This project uses an Arduino&reg; UNO R4 WiFi and synchronizes to the internet to get the current time. This **does not** require a real time clock module since it synchronizes with the internet every hour.

## Required Libraries

- [R4HttpClient](https://github.com/piscodev/r4httpclient)
- [SimpleTime](https://github.com/physee/SimpleTime)

## Required hardware

- 8ohm speaker connected to ground pin and analog pin 3

## What this does

- Makes your arduino into an alarm clock
- Plays an alarm off of analog pin 3
- Synchronizes with daylight savings time automatically
- Max alarm time (defaults to 20 minutes)

## Features I want to implement

- Web interface to set alarms on and off, set time zone, etc.
- Set multiple alarms in one day
- More songs