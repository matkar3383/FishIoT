import serial
import time
 
#s = serial.Serial('/dev/ttyAMA0', 9600) # Namen ggf. anpassen
s = serial.Serial('/dev/ttyUSB0', 9600) # Namen ggf. anpassen
#s.open()
time.sleep(5) # der Arduino resettet nach einer Seriellen Verbindung, daher muss kurz gewartet werden
 
#s.write("test")
try:
    while True:
        #response
        response = s.readline().decode("utf-8".strip())
        if response == "":
            print(".")
        else:
            print(response)
        
        #send
        s.write(b"das ist ein Text\n")
        
        time.sleep(5)
        
except KeyboardInterrupt:
    s.close()