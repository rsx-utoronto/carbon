#!/usr/bin/env python

from Tkinter import *
import ttk 
import time
import serial
import pygame
import math

# sudo apt-get install python-imaging-tk
# -or- pip install pillow

from PIL import ImageTk
from PIL import Image

import socket, select, string, sys

##############################
######## BASIC CONFIG ########
##############################

SERIAL_ON = False 
SOCKET_ON = True
JOYSTICK_ON = True
KEYBOARD_ON = True

print "SERIAL: %s" % SERIAL_ON
print "JOYSTICK: %s" % JOYSTICK_ON
print "KEYBOARD: %s" % KEYBOARD_ON

ser = None
joystick = None
sock = None

#TODO Try multiple serial ports
ser_serial_port = '/dev/ttyUSB0'
ser_baud = 9600

# socket settings
host = '192.168.1.109'
port = 5000

buffer_flushed = False

#TODO Setting for north for rover location
NORTH_DEGREES = 0

keyboard_input_array = [0] * 8
joystick_input_array = [0, 0, 0, 0, 0, 0]

#########################################
###### JOYSTICK / KEYBOARD CONFIG #######
#########################################

pygame.init()

def joystickGUIUpdate(axis_x, axis_y, axis_twist, axis_throttle):

    joystick_axis_x = joystick_axis_center + 40 * axis_x
    joystick_axis_y = joystick_axis_center + 40 * axis_y * -1

    display_joystick.coords(
        joystick_blip,
        joystick_axis_x,
        joystick_axis_y,
        joystick_axis_x + joystick_axis_radius,
        joystick_axis_y + joystick_axis_radius)

    display_joystick.coords(
        joystick_throttle,
        100, 90 - 80 * axis_throttle, 150, 90)

def joystickPump():
    JOYSTICK_THRESHOLD = 0.03
    TOPSPEED = 10
    direction = 0
    THROTTLE_FACTOR = 15

    pygame.event.pump()

    buttons = joystick.get_numbuttons()
    for i in range( buttons ):
        button = joystick.get_button(i)

    hats = joystick.get_numhats()
    for i in range(hats):
        hat = joystick.get_hat(i)

    axes = joystick.get_numaxes()
    axis_x = joystick.get_axis(0)
    axis_y = joystick.get_axis(1) * -1
    axis_twist = joystick.get_axis(2)
    axis_throttle = (-1 * joystick.get_axis(3) + 1) / 2.0

    joystickGUIUpdate(
        axis_x,
        axis_y,
        axis_twist,
        axis_throttle)

    if axis_y > JOYSTICK_THRESHOLD: direction = 2 # Drive forward
    elif axis_y < -JOYSTICK_THRESHOLD: direction = 1 # Drive backwards
    else: direction = 0 # Don't move

    speed = int(TOPSPEED * abs(axis_y)) + axis_throttle * THROTTLE_FACTOR
    left_speed = speed
    right_speed = speed

    if axis_x > JOYSTICK_THRESHOLD:
        left_speed = speed * (1 - abs(axis_x))

    elif axis_x < -JOYSTICK_THRESHOLD:
        right_speed = speed * (1 - abs(axis_x))

    twist_dir = 0
    twist_speed = abs(axis_twist) * TOPSPEED

    if axis_twist > 0.25:
        twist_dir = 1
    elif axis_twist < -0.25:
        twist_dir = 2
    else: 
        twist_dir = 0 

    joystick_input_array[0] = direction
    joystick_input_array[1] = round(right_speed, 2)
    joystick_input_array[2] = round(left_speed, 2)
    joystick_input_array[3] = round(speed, 2)
    joystick_input_array[4] = round(twist_dir, 2)
    joystick_input_array[5] = round(twist_speed, 2)

# def clearInputArrays():

#     for i in xrange(len(keyboard_input_array)):
#         keyboard_input_array[i] = 0

#     for i in xrange(len(joystick_input_array)):
#         #joystick_input_array[i] = 0
#         pass

def keyUp(e):
    key = e.char
    for index, (cw_key, ccw_key) in enumerate(zip(list('12345890'), list('qwertiop'))):
        if key == cw_key or key == ccw_key:
            keyboard_input_array[index] = 0

def keyDown(e):
    key = e.char

    for index, (cw_key, ccw_key) in enumerate(zip(list('12345890'), list('qwertiop'))):
        if key == cw_key:
            keyboard_input_array[index] = 1
        elif key == ccw_key:
            keyboard_input_array[index] = 2

def commandSend():

    if JOYSTICK_ON:
        joystickPump()

    # Keyboard input handled by KeyDown()
    # asynchronously

    keyboard_csv = str(keyboard_input_array).replace('[', '').replace(']','').replace('\n','').replace(' ', '')
    joystick_csv = str(joystick_input_array).replace('[', '').replace(']','').replace('\n','').replace(' ', '')

    command = "<%s,%s>" % (joystick_csv, keyboard_csv)

    if SERIAL_ON:
        ser.write(command)
        
    if SOCKET_ON:
        sock.send(command)
        
    print "Writing: ", command    

    # clearInputArrays()
    
    
##############################
######## GUI CONFIG ##########
##############################

# Setting up the GUI
root = Tk()
root.title('MRSX-3 Carbon GUI')

# Callback methods
def set_serial_port(event):
    global ser_serial_port
    prev_port = ser_serial_port
    try: 
        ser_serial_port = serialport.get() 
        # ser = serial.Serial(ser_serial_port, ser_baud, timeout = 0.01)
        print "Serial port was successfully changed" 
        ser.flushInput()
    except OSError as e: 
        print "Serial port was not updated", e
        ser_serial_port = prev_port
        # ser = serial.Serial(ser_serial_port, ser_baud, timeout=0.01)
        ser.flushInput()


# Setting up the GPS panel
display_gps = Canvas(root, width = 1000, height = 1000, background = 'white')
display_gps.pack(side=LEFT)

# Setting up the rover in the GPS panel
global rover, rover_image
lat_start = 0
long_start = 0
heading = None

# Setting up the UI buttons frame
button_panel = Frame(root, width = 400, height = 40)
button_panel.pack()
        
serialport = StringVar() 
serialselector = ttk.Combobox(button_panel, width=200, textvariable=serialport) 
serialselector.bind( '<<ComboboxSelected>>', lambda event: set_serial_port(event)) 
serialselector['values'] = ('/dev/ttyUSB0', '/dev/ttyUSB1',) 
# serialselector['state'] = 'normal' # not readonly. still doesn't work, idk 
serialselector.current(0)
serialselector.pack() 

redbutton = Button(button_panel, text = "Quit", width = 200, command=root.quit)
redbutton.pack()

# Setting up the joystick display panel
display_joystick = Canvas(root, width = 400, height = 100, background = 'white')
display_joystick.pack()

# Setting up the rover in the GPS panel
display_joystick.create_rectangle(10, 10, 90, 90, width = 0, fill = 'gray')

joystick_axis_radius = 15
joystick_axis_center = 42.5
joystick_blip = display_joystick.create_oval(
    joystick_axis_center,
    joystick_axis_center,
    joystick_axis_center + joystick_axis_radius,
    joystick_axis_center + joystick_axis_radius,
    width = 0,
    fill = 'red')

joystick_throttle = display_joystick.create_rectangle(
    100, 70, 150, 90, width = 0, fill = 'red')

# Setting up the sensors panel
display_sensors = Canvas(root, width = 400, height = 400, background = 'gray')
display_sensors.pack()

# Setting up the box where the raw telemetry data will be listed
display_raw = Listbox(root, background = 'gray')
display_raw.config(width = 50, height = 25)
display_raw.pack(side=LEFT)

# Rover positioning on map

GPS_MAPPING_RES = 100000
ROVER_DIMENSION = 5
ROVER_START_X = 475
ROVER_START_Y = 475

def plot_point_on_map(name, colour, latitude, longitude, heading = 0):

    global lat_start, long_start
    global rover, rover_image

    x_coord = (latitude - lat_start) * GPS_MAPPING_RES + ROVER_START_X
    y_coord = (longitude - long_start) * GPS_MAPPING_RES + ROVER_START_Y

    if name == 'rover':

        try:
            display_gps.delete(rover)
        except Exception as e:
            pass

        image = Image.open('rover_dot.png')
        angle = heading

        rover_image = ImageTk.PhotoImage(image.rotate(angle))

        rover = display_gps.create_image(
            x_coord,
            y_coord,
            image = rover_image)

        '''
        rover = display_gps.create_oval(
            ROVER_START_X,
            ROVER_START_Y,
            ROVER_START_X + ROVER_DIMENSION,
            ROVER_START_Y + ROVER_DIMENSION,
            width = 0,
            fill = 'red')
         '''

         # display_gps.coords(rover, 990, 990, 1000, 1000)

    elif latitude != 0.0 and longitude != 0.0:

        display_gps.create_oval(
                x_coord,
                y_coord,
                x_coord + ROVER_DIMENSION,
                y_coord + ROVER_DIMENSION,
                width = 0,
                fill = colour)

def plot_waypoints(filename):

    with open(filename) as f:
        waypoints = f.readlines()

    for waypoint in waypoints:

        data = waypoint.replace('\n', '').split(',')
        name = data[0]
        latitude = float(data[1])
        longitude = float(data[2])
        
        print data

        plot_point_on_map(name, 'green', latitude, longitude)

##############################
########### TASKS ############
##############################

# Initialization task
def init():

    global ser, joystick, sock

    if JOYSTICK_ON:

        # Initialize the joystick (Logitech Extreme3D Pro)
        pygame.joystick.init()
        joystick = pygame.joystick.Joystick(0)
        joystick.init()

    if SERIAL_ON:
        print "Serial initialize: ", ser_serial_port	
        ser = serial.Serial(ser_serial_port, ser_baud, timeout = 0.01)
        ser.flushInput()
        print (ser)
        
    if SOCKET_ON:
        print "Socket initialize: ", host	    
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(0.2)
     
        try :
            sock.connect((host, port))
        except :
            print 'Unable to connect to socket'
            sys.exit()
     
        print 'Connected to remote socket host. Start sending commands!'

# Main update loop task
def task():

    global ser, joystick, sock, buffer_flushed
    global lat_start, long_start, heading
    
    f = open('data', 'a')
    
    data = None

    if not buffer_flushed:

        if SERIAL_ON:
            ser.flushInput()

        buffer_flushed = True

    if SERIAL_ON:
        data = ser.readline()
        
    if SOCKET_ON:
        '''
        socket_list = [sys.stdin, sock]
        read_sockets, write_sockets, error_sockets = select.select(socket_list , [], [])
         
        for sock_in in read_sockets:
            if sock_in == sock:
                data = sock_in.recv(4096)  
                print data 
        '''
        try:
            data = sock.recv(1024)
        except Exception as e:
            pass
            
    if data:
    
        data = data.replace('\r\n','').split(',')
        display_raw.insert(0, str(data))
        f.write(str(data))
        f.close()        
        
        if len(data) >= 4 and data[0] == 'GPS':
            try:
                latitude = round(float(data[1]), 5)
                longitude = round(float(data[2]), 5)
                heading = int(float(data[3]))

                if lat_start == 0 and long_start == 0:
                        lat_start = latitude
                        long_start = longitude
                        plot_waypoints('junkyard/utias.waypoints')

                else:
                    plot_point_on_map('rover', 'black', latitude, longitude, heading = heading)
                    plot_point_on_map('path', 'gray', latitude, longitude)

            except Exception as e:
                print e
                
        elif len(data) >= 5 and data[0] == 'SENSORS':
            try: 
                sensor_data = data[1:5]
                print "sensors", sensor_data
            except Exception as e: 
                print e
                
        elif len(data) >= 3 and data[0] == 'GPSP':
            try:
                latitude = round(float(data[1]), 5)
                longitude = round(float(data[2]), 5)

                if lat_start == 0.0 and long_start == 0.0:
                        lat_start = latitude
                        long_start = longitude
                        plot_waypoints('terrain.waypoints')

                else:
                    plot_point_on_map('rover', 'black', latitude, longitude, heading = heading)
                    plot_point_on_map('path', 'gray', latitude, longitude)

            except Exception as e:
                print e            

    # Compile packet of data to send
    commandSend()

    # Reschedules itself to be called
    root.after(200, task)

root.bind("<KeyPress>", keyDown)
root.bind("<KeyRelease>", keyUp)

# Execute update task loop
root.after(5000, task)
root.after(1000, init)

# Execute main loop (once)
root.mainloop()
