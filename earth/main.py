from Tkinter import *
import time
import serial
import pygame
import math

# sudo apt-get install python-imaging-tk
# -or- pip install pillow
from PIL import ImageTk
from PIL import Image
from comms import *

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

    global ser, joystick

    if JOYSTICK_ON:

        # Initialize the joystick (Logitech Extreme3D Pro)
        pygame.joystick.init()
        joystick = pygame.joystick.Joystick(0)
        joystick.init()

    if SERIAL_ON:
        ser = serial.Serial(ser_serial_port, ser_baud, timeout = 0.01)
        ser.flushInput()

# Main update loop task
def task():

    global ser, joystick, buffer_flushed
    global lat_start, long_start, heading

    if not buffer_flushed:

        if SERIAL_ON:
            ser.flushInput()

        buffer_flushed = True

    if SERIAL_ON:

        ser_data = ser.readline().replace('\r\n','').split(',')
        if len(ser_data) > 2:
            display_raw.insert(0, str(ser_data))

            try:
                latitude = round(float(ser_data[0]), 5)
                longitude = round(float(ser_data[1]), 5)
                heading = int(float(ser_data[2]))

                if lat_start == 0 and long_start == 0:
                        lat_start = latitude
                        long_start = longitude
                        plot_waypoints('junkyard/utias.waypoints')

                else:
                    plot_point_on_map('rover', 'black', latitude, longitude, heading = heading)
                    plot_point_on_map('path', 'gray', latitude, longitude)

            except Exception as e:
                print e
                pass

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
