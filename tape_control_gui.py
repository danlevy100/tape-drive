import PySimpleGUI as sg
import serial

import time

def single_step(cmd, delay):    

    t_start = time.time()    
    ser.write(bytes(cmd, 'utf-8'))    
    t_end = time.time()
    
    time.sleep(1-(t_end-t_start))

def read_arduino():
    ard_response=str(ser.readline())
    print(ard_response[2:][:-5])
    time.sleep(5)
    

ser = serial.Serial('COM4', 9600)

sg.theme('Dark Amber')  # Let's set our own color theme

# STEP 1 define the layout
layout = [ 
            [sg.Text('Mode'), sg.Combo(['Single step', 'Continuous', 'Fast forward'], key='-MODE-', default_value='Single step')],
            [sg.Text('Rotation direction'), sg.Combo(['Top to bottom', 'Bottom to top'], key='-DIRECT-', default_value='Top to bottom')],            
            [sg.Button('Start', key='-START-'), sg.Button('Stop', key='-STOP-')],            
            [sg.Button('Release tension', key='-RELEASE TENSION-')],
            [sg.Button('Exit', key = '-EXIT-')],
            [sg.Text(size=(12,1), key='-COUNTER-')]
         ]

#STEP 2 - create the window
window = sg.Window('Tape control', layout)


# STEP3 - the event loop
while True:
    
    #window.perform_long_operation(read_arduino, '-READ ARDUINO DONE-')
    #print(ard_response[2:][:-5])    
    
    event, values = window.read(timeout=100)   # Read the event that happened and the values dictionary
    
    ard_response=str(ser.readline())
    ard_response = ard_response[2:][:-5]
    ser.reset_input_buffer()
    
    window['-COUNTER-'].update(ard_response)    
    
    #print(event, values)
    
    if event == sg.WIN_CLOSED or event == '-EXIT-':     # If user closed window with X or if user clicked "Exit" button then exit
        ser.write(b'stop:0')
        continuous = False
        break
    
    if event == '-STOP-':
        print('You pressed the STOP button')          
        ser.write(b'stop:0')
        continuous = False
        fast_forward = False        
    
    elif event == '-START-':      
        if values['-DIRECT-'] == 'Top to bottom':   
            direct = 1
        else:
            direct = -1               
                
        continuous = True if values['-MODE-'] == 'Continuous' else False
        step = True if values['-MODE-'] == 'Single step' else False
        fast_forward = True if values['-MODE-'] == 'Fast forward' else False
        
        delay = 1
        
        if continuous or step:
            cmd = 'single_step:' + str(direct)
            window.perform_long_operation(lambda: single_step(cmd, delay), '-SINGLE STEP DONE-')        
             
        elif fast_forward:
            cmd = 'ff_step:' + str(direct)            
            ser.write(bytes(cmd, 'utf-8'))
            
    elif event == '-RELEASE TENSION-':
        cmd = 'release_tension:0'
        ser.write(bytes(cmd, 'utf-8'))        
      
    elif event == '-SINGLE STEP DONE-':
        if continuous:
            window.perform_long_operation(lambda: single_step(cmd, delay), '-SINGLE STEP DONE-')        
        elif single_step:
            pass
        else:
            ser.write(b'stop:0')    
        
ser.close()
window.close()

    
         
    
