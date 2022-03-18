from pyfirmata import Arduino, util
import time
board = Arduino("COM8")

loopTimes = input ('Input number of blinks: ')
print("Blinking " + loopTimes + " times.")

for i in range (int(loopTimes)):
    board.digital[13].write(1)
    time.sleep(0.2)
    board.digital[13].write(0)
    time.sleep(0.2)