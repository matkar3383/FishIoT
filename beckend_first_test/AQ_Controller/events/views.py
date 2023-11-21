from django.shortcuts import render

from .modules import *

# initialize controller at start of the app
controller = AquariumECU()

# Create your views here.
def home(request):
    print('bin in home')
    #s = serial.Serial('/dev/ttyUSB0', 9600)
    return render(request, 'home.html', {})


def send_check_request(request):
    print('test von check request')
    dict_read_values = controller.sendSerialDataCheck()
    print(dict_read_values)
    return render(request, 'home.html', dict_read_values)


def send_set_request(request):
    controller.inputSignals['set_mode'] = int(not controller.inputSignals['set_mode'])
    controller.inputSignals['set_light'] = int(not controller.inputSignals['set_light'])
    controller.inputSignals['set_fan'] = int(not controller.inputSignals['set_fan'])
    controller.inputSignals['set_feeder'] = int(not controller.inputSignals['set_feeder'])
    controller.inputSignals['set_critical_temperature'] = 23.5
    
    #controller.sendSerialData()
    controller.sendSerialDataIntervalSet(component='light')
    #controller.sendSerialDataIntervalSet(component='fan')
    #controller.sendSerialDataIntervalSet(component='feeder')
    #controller.sendSerialTimeUpdate()
    
    return render(request, 'home.html', {})

def disconnect(request):
    controller.disconnectDevice()
    return render(request, 'home.html', {})