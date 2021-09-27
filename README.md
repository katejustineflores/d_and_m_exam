# How to use this code?
This code can visually represent real-time subsurface tilt sensor movements using 2 nodes.

To use this code, kindly follow the step-by-step process:

1. Download any Bluetooth Terminal app from Google Play Store for android users or from App Store for ios users.
2. Using Arduino IDE, upload the bluetooth_serial.ino to the customDue.
3. Open the Bluetooth Terminal app.
4. Scan and pair with HC-05 bluetooth (if asked for password, type 1234 or 0000). 
5. Connect to the bluetooth module using the Bluetooth Terminal app.
6. Run the read_bluetooth_data.py 
7. Enter the commands in the Bluetooth Terminal app (list of commands will be shown after running the read_bluetooth_data.py).
8. The program will plot the sensor movement in real-time whenever data is transmitted via the bluetoooth module.
