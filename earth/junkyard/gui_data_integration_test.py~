#!/usr/bin/env python

from Tkinter import *
import time
import serial
import pygame
import math

# sudo apt-get install python-imaging-tk
from PIL import ImageTk
from PIL import Image

##############################
######## BASIC CONFIG ########
##############################

SERIAL_ON = False
GPS_ON = False
JOYSTICK_ON = True
KEYBOARD_ON = True

global gps
ser = None
joystick = None

gps_serial_port = '/dev/ttyACM0'
gps_baud = 9600

ser_serial_port = '/dev/ttyUSB0'
ser_baud = 9600

buffer_flushed = False

NORTH_DEGREES = 0

keyboard_input_array = ['0', '0', '0', '0', '0']
joystick_input_array = ['0', '0', '0', '0']

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

    TOPSPEED = 25
    direction = "0"

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

    if axis_y > 0.01: direction = "2" # Drive forward
    elif axis_y < -0.01: direction = "1" # Drive backwards
    else: direction = "0" # Don't move

    speed = int(TOPSPEED * abs(axis_y))
    left_speed = speed
    right_speed = speed

    if axis_x > 0.01:
        right_speed = speed * (1 - math.pow(abs(axis_x), 2.5)) + 5

    elif axis_x < -0.01:
        left_speed = speed * (1 - math.pow(abs(axis_x), 2.5)) + 5
     
    joystick_input_array[0] = str(direction)
    joystick_input_array[1] = str(round(right_speed, 2))
    joystick_input_array[2] = str(round(left_speed, 2))
    joystick_input_array[3] = str(speed)
        
def clearInputArrays():

    for i in xrange(len(keyboard_input_array)):
        keyboard_input_array[i] = '0' 
        
    for i in xrange(len(joystick_input_array)):
        #joystick_input_array[i] = '0'                    
        pass
               
def keyUp(e):
    pass
    
def keyDown(e):
    
    key = e.char
    
    if key == '1':
        keyboard_input_array[0] = '1'
        
    elif key == 'q':
        keyboard_input_array[0] = '2'
        
    elif key == '2':
        keyboard_input_array[1] = '1'
        
    elif key == 'w':
        keyboard_input_array[1] = '2'        

    elif key == '3':
        keyboard_input_array[2] = '1'
        
    elif key == 'e':
        keyboard_input_array[2] = '2'  

    elif key == '4':
        keyboard_input_array[3] = '1'
        
    elif key == 'r':
        keyboard_input_array[3] = '2'
        
    elif key == '5':
        keyboard_input_array[4] = '1'
        
    elif key == 't':
        keyboard_input_array[4] = '2'   

def commandSend():

    if JOYSTICK_ON:
        joystickPump()
    
    # Keyboard input handled by KeyDown()
    # asynchronously   

    keyboard_csv = str(keyboard_input_array).replace('[', '').replace(']','').replace('\n','').replace(' ', '')
    joystick_csv = str(joystick_input_array).replace('[', '').replace(']','').replace('\n','').replace(' ', '')
        
    print "<%s,%s>" % (joystick_csv, keyboard_csv)    
        
    clearInputArrays()

##############################
######## GUI CONFIG ##########
##############################

# Setting up the GUI
root = Tk()
root.title('MRSX-3 Carbon GUI')

# Callback methods

def quit():
    root.quit()

# Setting up the GPS panel
display_gps = Canvas(root, width = 1000, height = 1000, background = 'white')
display_gps.pack(side = LEFT)

# Setting up the rover in the GPS panel
global rover, rover_image
lat_start = 0
long_start = 0
heading = None

# Setting up the UI buttons frame
button_panel = Frame(root, width = 400, height = 40)
button_panel.pack()

redbutton = Button(button_panel, text = "Quit", width = 400, command = quit)
redbutton.pack( side = LEFT)

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
display_raw.pack()

# Rover positioning on map

GPS_MAPPING_RES = 100000
ROVER_DIMENSION = 10
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
            
        image = Image.open('../assets/rover.png')
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

        plot_point_on_map(name, 'green', latitude, longitude)
        
##############################
########### TASKS ############
##############################

# Initialization task
def init():

    global gps, ser, joystick

    if JOYSTICK_ON:

        # Initialize the joystick (Logitech Extreme3D Pro)
        pygame.joystick.init()    
        joystick = pygame.joystick.Joystick(0)
        joystick.init()    

    if SERIAL_ON: 
        ser = serial.Serial(ser_serial_port, ser_baud)
        ser.flush()
        
    if GPS_ON:
        gps = serial.Serial(gps_serial_port, gps_baud)
        gps.flushInput()

# Main update loop task
def task():

    global gps, ser, joystick, buffer_flushed
    global lat_start, long_start, heading
    
    if not buffer_flushed:
        
        if GPS_ON: 
            gps.flushInput()
        
        buffer_flushed = True
    
    if GPS_ON:
        gps_data = gps.readline().replace('\r\n','').split(',')
        if len(gps_data) == 3:
            display_raw.insert(0, str(gps_data))
            
            try:
                latitude = round(float(gps_data[0]), 5)
                longitude = round(float(gps_data[1]), 5)
                heading = int(float(gps_data[2]))
                
                if lat_start == 0 and long_start == 0:
                        lat_start = latitude
                        long_start = longitude
                        plot_waypoints('utias.waypoints')

                else:
                    plot_point_on_map('rover', 'black', latitude, longitude, heading = heading)
                    plot_point_on_map('path', 'gray', latitude, longitude)
                    
            except Exception as e:
                print e
                pass

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
