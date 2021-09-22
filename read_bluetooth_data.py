import serial
import numpy as np
import matplotlib.pyplot as plt
from drawnow import *
import pandas as pd

#part 1: initialize serial comms with bluetooth, get data from sensor
#part 2: compute for xy and xz
#part 3: real-time plotting

list_acc = []
xy_val = []
xz_val = []

# x,y,z values
xa = []
ya = []
za = []

xz1 = []
xz2 = []

xy1 = []
xy2 = [] 

seg_len = 1


plt.ion()

print ("""List of commands: 
    a - accel 1
    b - accel 2
    Type the command in the bluetooth terminal.\n""")
    
    
    
comPort = 'COM3'
arduinoData = serial.Serial(
                port = (comPort),\
                baudrate = 9600,\
                parity = serial.PARITY_NONE,\
                stopbits = serial.STOPBITS_ONE,\
                bytesize = serial.EIGHTBITS,\
                timeout = None)        
    
def realtimePlot():
    
    ax1 = plt.subplot(2,2,1)
    plt.ylabel('Node 1', color='black', fontsize=8)
    ax1.plot(xz1, 'red')
    plt.grid()
    plt.title('Horizontal Displacement, Across Slope (m)', color='black',fontsize=8,verticalalignment='bottom')


#    plt.title(stdvx,color='red',fontsize=8,verticalalignment='top')

    ax2 = plt.subplot(2,2,2, sharex = ax1)
    ax2.plot(xy1, 'blue')
    plt.grid()
    plt.title('Horizontal Displacement, Downslope (m)', color='black',fontsize=8,verticalalignment='bottom')


#    plt.title(stdvy, color='blue',fontsize=8,verticalalignment='top')
    
    ax1 = plt.subplot(2,2,3)
    plt.ylabel('Node 2', color='black', fontsize=8)
    ax1.plot(xz2, 'red', marker = 'o')
    plt.grid()


    ax2 = plt.subplot(2,2,4, sharex = ax1)
    ax2.plot(xy2, 'blue', marker = 'o')
    plt.grid()

 
    
def accel_to_lin_xz_xy(seg_len,xa,ya,za): #from genproc_netvel.py

    #DESCRIPTION
    #converts accelerometer data (xa,ya,za) to corresponding tilt expressed as horizontal linear displacements values, (xz, xy)
    
    #INPUTS
    #seg_len; float; length of individual column segment
    #xa,ya,za; array of integers; accelerometer data (ideally, -1024 to 1024)
    
    #OUTPUTS
    #xz, xy; array of floats; horizontal linear displacements along the planes defined by xa-za and xa-ya, respectively; units similar to seg_len
    

    theta_xz = np.arctan2(za,(np.sqrt(xa**2 + ya**2)))
    theta_xy = np.arctan2(ya,(np.sqrt(xa**2 + za**2)))
    xz = seg_len * np.sin(theta_xz)
    xy = seg_len * np.sin(theta_xy)
    
    return xz, xy
    

try:
    while True: # While loop that loops forever
        while (arduinoData.inWaiting()==0): #Wait here until there is data
            pass
#              print ("a") #do nothing
        arduinoString = arduinoData.readline().decode() #read the line of text from the serial port
#        print (arduinoString)
        arduinoString = arduinoString.strip("\r\n")
        print (arduinoString)
        
        dataArray = arduinoString.split(',')
        
        nid = dataArray[0].split("\t")
        nid = nid[0][-4:]
        
        type_num = dataArray[0].split("\t")
        type_num = type_num[1]
            
        x = dataArray[0].split("\t")
        x = int(x[2])
        xa.append(x)
        
        y = int(dataArray[1])
        ya.append(y)

        z = int(dataArray[2])
        za.append(z)


        print(accel_to_lin_xz_xy(seg_len,x,y,z))
                
        
#       store xz data
        xz = list((accel_to_lin_xz_xy(seg_len,x,y,z)))
        xz = float(xz[0])
        xz_val.append(xz)
        
        for i in range(0, len(xz_val)):
            if i % 2:
                xz1.append(xz_val[i])
            else:
                xz2.append(xz_val[i])
#
##      store xy data
        xy = list((accel_to_lin_xz_xy(seg_len,x,y,z)))
        xy = float(xy[1])
        xy_val.append(xy)
        
        for i in range(0, len(xy_val)):
            if i % 2:
                xy1.append(xy_val[i])
            else:
                xy2.append(xy_val[i])     

        drawnow(realtimePlot)
        plt.pause(.000001)

        
        print ("node_id: " + nid)
        print ("type_num: " + type_num)
        print ("xa: " + str(x))
        print ("ya: " + str(y))
        print ("za: " + str(z))
        print("=============")        

        

except KeyboardInterrupt:
    arduinoData.close()
finally:
    arduinoData.close()


