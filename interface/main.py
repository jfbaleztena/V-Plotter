import serial
import sys
import time

serial_port = serial.Serial('/dev/ttyACM0')  # open serial port

INFO = 0
ACK = 1
NACK = 2

CMD_IN = b'IN;\n'

def get_and_show_line():
    line = serial_port.readline()   # read a '\n' terminated line
    print("Device: ", line)
    if line == b'ACK\r\n':
        return ACK
    elif line == b'NACK\r\n':
        return NACK
    else:
        return INFO

def init_device():
    print("V-Plotter | Front End\r\n")
    get_and_show_line()
    get_and_show_line()
    time.sleep(2)

def send_command(command):
    serial_port.write(command)
    res = INFO
    while(res == INFO):
        res = get_and_show_line()
        if res == ACK:
            print("RES: ", command, "ACK")
        elif res == NACK:
            print("RES: ", command, "NACK")

star_coodinates = []
star_coodinates.append((5.5,0))
star_coodinates.append((1.8,-5.4))
star_coodinates.append((1.8,5.4))
star_coodinates.append((5.5,0))
star_coodinates.append((-4.2,3.3))
star_coodinates.append((1.7,5.3))
star_coodinates.append((-4.5,-3.2))
star_coodinates.append((-4.5,3.2))
star_coodinates.append((1.7,-5.3))
star_coodinates.append((-4.2,-3.3))
star_coodinates.append((10,3.3))
star_coodinates.append((4.2,-3.3))
star_coodinates.append((-10,3.3))
star_coodinates.append((-1.7,5.3))
star_coodinates.append((6.2,-8.5))
star_coodinates.append((-1.8,-5.4))
star_coodinates.append((-1.8,5.4))
star_coodinates.append((6.2,8.5))

BOARD_SCALER = 10 
star_coodinates = [(i*BOARD_SCALER, j*BOARD_SCALER) for i, j in star_coodinates]

init_device()

serial_port.write(b'IN;\n')   
get_and_show_line()
get_and_show_line()

send_command(CMD_IN)

for i in range(len(star_coodinates)):
    formated_command = 'PR{0},{1};\n'.format(star_coodinates[i][0], star_coodinates[i][1])
    send_command(formated_command.encode())

serial_port.close() 
sys.exit(0)

