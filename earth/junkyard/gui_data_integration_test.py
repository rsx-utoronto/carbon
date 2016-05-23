#!/usr/bin/env python

from Tkinter import *
import time
import serial
import pygame
import math

# Telemetry system connected, TEST_MODE = False
SERIAL_ON = True
GPS_ON = False

##############################
####### SERIAL CONFIG ########
##############################

if SERIAL_ON: ser = serial.Serial('/dev/ttyUSB0', 9600)

##############################
###### JOYSTICK CONFIG #######
##############################

pygame.init()

# Initialize the joystick (Logitech Extreme3D Pro)
pygame.joystick.init()
joystick = pygame.joystick.Joystick(0)
joystick.init()

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

def joystickInfo():

    pygame.event.pump()
    
    buttons = joystick.get_numbuttons()
    for i in range( buttons ):
        button = joystick.get_button(i)

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

    joystickTransmit(
        axis_x, 
        axis_y,
        axis_twist,
        axis_throttle)

    hats = joystick.get_numhats()
    for i in range( hats ):
        hat = joystick.get_hat(i)

def joystickTransmit(axis_x, axis_y, axis_twist, axis_throttle):

    TOPSPEED = 25
    direction = "0"
    
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

    ser.write('<%s,%s,%s,%s>' % (direction, right_speed, left_speed, speed))

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
rover = display_gps.create_oval(10, 10, 100, 100, width = 0, fill = 'blue')

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

# Main update loop task
def task():
    if GPS_ON:
        gps_data = gps.readline().replace('\r\n','').split(',')
        display_raw.insert(0, str(gps_data))

    display_gps.coords(rover, 990, 990, 1000, 1000)

    joystickInfo()

    # Reschedules itself to be called
    root.after(200, task)

# Execute update task loop
root.after(1000, task)

# Execute main loop (once)
root.mainloop()