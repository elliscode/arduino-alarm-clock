# Why I made this

- My KaiOS flip phone alarms don't reliably work, some days they will either be late or just never go off
- I don't want smartspeakers in my house
- The wifi clocks on amazon were either not wifi clocks, or way too expensive and feature-bloated

# Required Libraries

- [R4HttpClient](https://github.com/piscodev/r4httpclient)
- [SimpleTime](https://github.com/physee/SimpleTime)

# What this does

- Makes your arduino into an alarm clock
- Plays an alarm off of analog pin 3

# Features I want to implement

- Web interface to set alarms on and off, set time zone, etc.
- Synchronize with daylight savings time automatically (right now I hardcoded -05:00 even though I get that info back in my API call)
- Set a max alarm time (so a missed alarm doesnt go off forever, maybe a max time of 20 minutes or so by default)
- Set multiple alarms in one day
- More songs