# How to use this code
This code can visually represent real-time subsurface tilt sensor movements using 2 nodes.

To use this code, kindly follow the step-by step process:

1. Download any Bluetooth Terminal app from Google Play Store for android users or from App Store for ios users.
2. Using Arduino IDE, upload the bluetooth_serial.ino to the customDue.
4. Open Serial Monitor and type c to change the sensor version to 4 (default sensor version is 255).
5. Close the Serial Monitor and go to Spyder.
6. Open the Bluetooth Terminal app.
7. Connect to the bluetooth module using the Bluetooth Terminal app.
8. Run the read_bluetooth_data.py 
9. Enter the commands in the Bluetooth Terminal App (list of commands will be shown after running the read_bluetooth_data.py.
10. The program will plot the sensor movement in real-time whenever data is transmitted via the bluetoooth module.
