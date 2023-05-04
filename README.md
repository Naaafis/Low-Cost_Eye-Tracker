Demo Video -> https://youtu.be/nrKnskTpATE

# Low-Cost_Eye-Tracker
Development of eye tracking software that can be used to control digital systems without the use of hands. 

Notable software files:

tracker_flask: API directory

tracker_flask/trackerAPI/app.py: api source file

eye_tracker.ipynb: notebook displaying the funcitonalities of the eye tracker

To launch API:

```
cd tracker_flask
pip install pipenv
pipenv shell
pipenv install
./bootstrap.sh
```

The PCB was devloped in Eagle and hold the following components.
- ESP32-S2-Saola
- MAX7219
- Arudcam Mini
- 1088AS LED Matrix

It utilized SPI to communicate across all the devices and HTTP to interact with the cloud application. The ESP is loaded with the program in the Arduino IDE and used the arducam mini pc host application to send the command to capture an image.
