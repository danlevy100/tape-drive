import PySimpleGUI as sg
import serial

ser = serial.Serial('COM4')

sg.theme('Dark Amber')  # Let's set our own color theme

# STEP 1 define the layout
layout = [ 
            [sg.Text('This is a very basic PySimpleGUI layout')],
            [sg.Input()],
            [sg.Button('Start', key='-START-'), sg.Button('Stop', key='-STOP-')],            
            [sg.Button('Exit')]
         ]

#STEP 2 - create the window
window = sg.Window('Tape control', layout)

# STEP3 - the event loop
while True:
    event, values = window.read()   # Read the event that happened and the values dictionary
    print(event, values)
    if event == sg.WIN_CLOSED or event == 'Exit':     # If user closed window with X or if user clicked "Exit" button then exit
        break
    if event == '-START-':
      print('You pressed the START button')
      ser.write(b'downward\n')
      
    if event == '-STOP-':
      print('You pressed the STOP button')      
      ser.write(b'stop\n')

window.close()
ser.close()