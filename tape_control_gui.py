'''
Dan Levy, July 2022.

Python GUI for controlling the tape.
A serial connection to the Arduino is created via the USB interface.
The Arduino listens to commands and responds back continuously with the current
tension of the tape.

'''


import PySimpleGUI as sg
import serial

import time

def single_step(cmd, delay=1):    

    t_start = time.time()    
    ser.write(bytes(cmd, 'utf-8'))    
    t_end = time.time()
    
    time.sleep(1-(t_end-t_start))

def read_arduino():
    ard_response=str(ser.readline())
    print(ard_response[2:][:-5])
    time.sleep(5)
    

ser = serial.Serial('COM9', 115200)

sg.theme('Reddit')  # Let's set our own color theme

# STEP 1 define the layout
layout = [ 
            [sg.Text('Mode', size=(16,1)), sg.Combo(['Single step', 'Continuous', 'Fast forward'], key='-MODE-', default_value='Single step', size=(12,1))],
            [sg.Text('Rotation direction', size=(16,1)), sg.Combo(['Top to bottom', 'Bottom to top'], key='-DIRECT-', default_value='Top to bottom', size=(12,1))],
            [sg.Text('Tension [0-100]', size=(16,1)), sg.InputText(default_text=25, size=(3,1), key='-TENSION-')],
            [sg.Text('Step counter: ', size=(12,1)), sg.Text(size=(6,1), key='-STEP COUNTER-'), sg.Button('Reset counter', key='-RESET COUNTER-')],
            [sg.Text("Tape tension: ", size=(12,1)), sg.Text(size=(6,1), key='-TENSION READ-'), sg.Button('Release tension', key='-RELEASE TENSION-')],
            [sg.Text('Arduino status: ', size=(12,1)), sg.Text(size=(12,1), key='-ARDUINO STATUS-')],            
            [sg.Text("Tape status: ", size=(12,1)), sg.Text(size=(12,1), key='-TAPE STATUS-')],
            #[sg.Text("Motor X temperature: ", size=(12,1)), sg.Text(size=(12,1), key='-MOTOR X TEMP-')],
            [sg.Button('Go', key='-GO-'), sg.Button('Stop', key='-STOP-')],                        
            [sg.Button('Exit', key = '-EXIT-')]
         ]

#STEP 2 - create the window
window = sg.Window('Tape control', layout)

# STEP3 - the event loop
comm_loss = 0
step_counter = 0
while True:    
    
    event, values = window.read(timeout=100) # Read the event that happened and the values dictionary
    
    try:
        ard_response = str(ser.readline())
        ard_response = ard_response[2:][:-5]
        ard_response = ard_response.split(",")    
        tension_read = ard_response[0]
        #EOT = ard_response[1]    
        ser.reset_input_buffer()
        ard_status = 'OK'
        tape_status = 'READY'
        comm_loss = 0
        tempX = ard_response[2]
        
    except:
        comm_loss += 1
        if comm_loss > 2:
            ard_status = 'COMM LOST'                
    
    #if EOT=='1':
    #    ard_status = 'END OF TAPE'
    
    try:
        if fast_forward:
            tape_status = 'MOVING'
    except:
        pass
    
    window['-TENSION READ-'].update(tension_read)       
    window['-ARDUINO STATUS-'].update(ard_status)       
    #window['-TAPE STATUS-'].update(tape_status)       
    window['-STEP COUNTER-'].update(step_counter)
    #window['-MOTOR X TEMP-'].update(tempX)
    
    #print(event, values)
    
    if event == sg.WIN_CLOSED or event == '-EXIT-':     # If user closed window with X or if user clicked "Exit" button then exit
        ser.write(b'stop::')
        continuous = False
        break
    
    if event == '-RESET COUNTER-':
        step_counter = 0
    
    if event == '-STOP-':
        print('You pressed the STOP button')          
        ser.write(b'stop::')
        continuous = False
        fast_forward = False        
    
    elif event == '-GO-':      
        if values['-DIRECT-'] == 'Top to bottom':   
            direct = 1
        else:
            direct = -1               
                
        tension = values['-TENSION-']        
        
        continuous = True if values['-MODE-'] == 'Continuous' else False
        step = True if values['-MODE-'] == 'Single step' else False
        fast_forward = True if values['-MODE-'] == 'Fast forward' else False        
        
        tape_status = 'ROLLING'
        window['-TAPE STATUS-'].update(tape_status)
        
        if continuous or step:
            step_counter += 1
            cmd = f'single_step:{direct}:{tension}'            
            window.perform_long_operation(lambda: single_step(cmd), '-SINGLE STEP DONE-')            
             
        elif fast_forward:
            cmd = f'ff_step:{direct}:{tension}'
            ser.write(bytes(cmd, 'utf-8'))            
            
    elif event == '-RELEASE TENSION-':
        cmd = 'release_tension::'
        ser.write(bytes(cmd, 'utf-8'))        
        continuous = False
        
      
    elif event == '-SINGLE STEP DONE-':        
        tape_status = 'READY'
        if continuous:
            window.perform_long_operation(lambda: single_step(cmd), '-SINGLE STEP DONE-')
            tape_status = 'MOVING'
        elif single_step:
            pass
        else:
            ser.write(b'stop::')    
        
        window['-TAPE STATUS-'].update(tape_status)              
        
ser.close()
window.close()

    
         
    
