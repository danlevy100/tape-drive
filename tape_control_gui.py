import PySimpleGUI as sg
import serial

import time

def step(cmd):    

    t_start = time.time()    
    ser.write(bytes(cmd, 'utf-8'))    
    t_end = time.time()
    
    time.sleep(1-(t_end-t_start))    
    

ser = serial.Serial('COM4')

sg.theme('Dark Amber')  # Let's set our own color theme

# STEP 1 define the layout
layout = [ 
            [sg.Text('Mode'), sg.Combo(['Single step', 'Continuous'], key='-MODE-', default_value='Single step')],        
            [sg.Text('Rotation direction'), sg.Combo(['Top to bottom', 'Bottom to top'], key='-DIRECT-', default_value='Top to bottom')],            
            [sg.Button('Start', key='-START-'), sg.Button('Stop', key='-STOP-')],            
            [sg.Button('Exit', key = '-EXIT-')]
         ]

#STEP 2 - create the window
window = sg.Window('Tape control', layout)


# STEP3 - the event loop
while True:
    event, values = window.read()   # Read the event that happened and the values dictionary
    print(event, values)
    
    if event == sg.WIN_CLOSED or event == '-EXIT-':     # If user closed window with X or if user clicked "Exit" button then exit
        ser.write(b'stop:0')
        continuous = False
        break
    
    if event == '-START-':      
        if values['-DIRECT-'] == 'Top to bottom':
            direct = 1
        else:
            direct = -1               
                
        continuous = True if values['-MODE-'] == 'Continuous' else False
        single_step = False if values['-MODE-'] == 'Continuous' else True
        
        cmd = 'step:' + str(direct)              
        
        window.perform_long_operation(lambda: step(cmd), '-STEP DONE-')        
    
    elif event == '-STOP-':
      print('You pressed the STOP button')          
      ser.write(b'stop:0')
      continuous = False
      
    elif event == '-STEP DONE-':
        if continuous:
            window.perform_long_operation(lambda: step(cmd), '-STEP DONE-')        
        elif single_step:
            pass
        else:
            ser.write(b'stop:0')        
        
window.close()
ser.close()          
    
         
    
