from Tkinter import *
import time

root = Tk()
root.title('MRSX-3 Carbon GUI')

display_gps = Canvas(root, width = 800, height = 800, background = 'white')
display_gps.pack(side = LEFT)

display_sensors = Canvas(root, width = 400, height = 400, background = 'gray')
display_sensors.pack()

display_raw = Listbox(root, background = 'gray')
display_raw.config(width = 50, height = 25)
display_raw.pack()

def task():
    print('hello')
    root.after(200, task)

root.after(1000, task)
root.mainloop()