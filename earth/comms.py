#!/usr/bin/env python

from Tkinter import *
import time
import serial
import pygame
import math

# sudo apt-get install python-imaging-tk
# -or- pip install pillow

from PIL import ImageTk
from PIL import Image

##############################
######## BASIC CONFIG ########
##############################

SERIAL_ON = True
JOYSTICK_ON = True
KEYBOARD_ON = True

ser = None
joystick = None

#TODO Try multiple serial ports
ser_serial_port = '/dev/ttyUSB1'
ser_baud = 9600

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
    TOPSPEED = 25
    direction = 0

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
    else: direction = "0" # Don't move

    speed = int(TOPSPEED * abs(axis_y))
    left_speed = speed
    right_speed = speed

    if axis_x > JOYSTICK_THRESHOLD:
        right_speed = speed * (1 - math.pow(abs(axis_x), 3.5)) + 5

    elif axis_x < -JOYSTICK_THRESHOLD:
        left_speed = speed * (1 - math.pow(abs(axis_x), 3.5)) + 5

    twist_dir = 0
    twist_speed = abs(axis_twist) * TOPSPEED

    if axis_twist > 0.25:
        twist_dir = 1

    elif axis_twist < -0.25:
        twist_dir = 2

    joystick_input_array[0] = direction
    joystick_input_array[1] = round(right_speed, 2)
    joystick_input_array[2] = round(left_speed, 2)
    joystick_input_array[3] = speed
    joystick_input_array[4] = twist_dir
    joystick_input_array[5] = twist_speed

# def clearInputArrays():

#     for i in xrange(len(keyboard_input_array)):
#         keyboard_input_array[i] = 0

#     for i in xrange(len(joystick_input_array)):
#         #joystick_input_array[i] = 0
#         pass

def keyUp(e):
    key = e.char
    for index, (cw_key, ccw_key) in enumerate(zip(list('12345890'), list('qwertyiop'))):
        if key == cw_key or key == ccw_key:
            keyboard_input_array[index] = 0

def keyDown(e):
    key = e.char

    for index, (cw_key, ccw_key) in enumerate(zip(list('12345890'), list('qwertyiop'))):
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

    # clearInputArrays()