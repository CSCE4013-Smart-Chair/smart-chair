import pyfirmata
import time

# Variables ###############################
board = pyfirmata.Arduino("COM8")

# Setup ###################################
a0 = board.get_pin('a:0:i')
it = pyfirmata.util.Iterator(board) 
it.start()

# Loop ####################################
while True:
    #Update Analog
    print(a0.read())
    #Blink the on-board LED
    board.digital[13].write(1)
    time.sleep(0.5)
    board.digital[13].write(0)
    time.sleep(0.5)






# loopTimes = input ('Input number of blinks: ')
# print("Blinking " + loopTimes + " times.")

# for i in range (int(loopTimes)):
#     board.digital[13].write(1)
#     time.sleep(1)
#     board.digital[13].write(0)
#     time.sleep(1)

# board.digital[10].mode = pyfirmata.INPUT
# sw = board.digital[10].read()
# if sw:
#     board.digital[13].write(1)
# else:
#     board.digital[13].write(0)