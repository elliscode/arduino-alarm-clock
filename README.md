# Arduino WiFi Alarm Clock Without RTC

## Why I made this

- My KaiOS flip phone alarms don't reliably work, some days they will either be late or just never go off
- I don't want smartspeakers in my house
- The wifi clocks on amazon were either not wifi clocks, or way too expensive and feature-bloated

## Required Libraries

- [R4HttpClient](https://github.com/piscodev/r4httpclient)
- [SimpleTime](https://github.com/physee/SimpleTime)

## Required hardware

- 8ohm speaker connected to ground pin and analog pin 3

## What this does

- **Does not require RTC** since it synchronizes with the internet every hour
- Makes your arduino into an alarm clock
- Plays an alarm off of analog pin 3
- Synchronizes with daylight savings time automatically
- Max alarm time (defaults to 20 minutes)

## Features I want to implement

- Web interface to set alarms on and off, set time zone, etc.
- Set multiple alarms in one day
- More songs