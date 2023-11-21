import serial
import time
from datetime import datetime

TTY_possibilities = ['/dev/ttyUSB0', '/dev/ttyUSB1']

class ECU_Variables():
    def __init__(self) -> None:
        # input to controller
        self.inputSignals = {
            'sync_time':                'HH:MM:SS',
            'set_mode':                 0,
            'set_light':                0,
            'set_fan':                  0,
            'set_feeder':               0,
            'set_critical_temperature': 0.0,
            'set_interval_light':      [-1 for _ in range(10)],
            'set_interval_fan':        [-2 for _ in range(10)],
            'set_interval_feeder':     [-3 for _ in range(2)],
            }
        
        # output from controller
        self.outputSignals = {
            'read_status_mode':          0,
            'read_status_light':         0,
            'read_status_fan':           0,
            'read_status_feeder':        0,
            'read_current_temperature':  0.0,
            'read_critical_temperature': 0.0,
            'read_interval_light':      [],
            'read_interval_fan':        [],
            'read_interval_feeder':     [],
            }
    def convertOutputValue(self, strValue: str, referenceKey: str):
        if type(self.outputSignals[referenceKey]) != list:
            # check type of value in dict and set the same type to read value
            print(referenceKey, ''.join(strValue.split(' ')[1:]))
            # bei Umwandlug von Temperatur von float auf strig (arduio) wird ein leerzeiche 
            # zu viel hinzugefügt -> der muss da gefiltert werden 
            return type(self.outputSignals[referenceKey])(''.join(strValue.split(' ')[1:]))
            
        else:
            #TODO welcher Typ sollen in Intervallen verwendet werden? -> float
            item_list = []
            for item in strValue.split(' ')[1:]:
                print(referenceKey, item)
                item_list.append(item)
            
            return item_list
            #return [float(item) for item in strValue.split(' ')[1:]]

    # def setInputValues(self, 
    #                    set_mode : int = self.inputSignals['set_mode'],
    #                    set_light : int = self.inputSignals['set_light'],
    #                    set_fan : int = self.inputSignals['set_fan'],
    #                    set_feeder : int = self.inputSignals['set_feeder'],
    #                    set_critical_temperature : float = self.inputSignals['set_critical_temperature'],
    #                    set_interval_light : list = self.inputSignals['set_interval_light'],
    #                    set_interval_fan : list = self.inputSignals['set_interval_fan'],
    #                    set_interval_feeder : list = self.inputSignals['set_interval_feeder']):
    #     # ???
    #     self.inputSignals['sync_time'] = time.localtime()
    #     self.inputSignals['set_mode'] = set_mode
    #     self.inputSignals['set_light'] = set_light
    #     self.inputSignals['set_fan'] = set_fan
    #     self.inputSignals['set_feeder'] = set_feeder
    #     self.inputSignals['set_critical_temperature'] = set_critical_temperature
    #     self.inputSignals['set_interval_light'] = set_interval_light
    #     self.inputSignals['set_interval_fan'] = set_interval_fan
    #     self.inputSignals['set_interval_feeder'] = set_interval_feeder
    
    def setInputValues2(self, **kwargs):
        dict_kwargs = dict(kwargs)
        for key in kwargs:
            self.inputSignals[key] = kwargs[key]
            
    def timeUpdate(self):
        'give back a current time to sync_time in inputSignals (Format HH:MM:SS)'
        self.inputSignals['sync_time'] = datetime.now().strftime("%H:%M:%S")
    
    def clearIntervalValues(self, component = None, signalname = None):
        """clear the interval list found by component ['light', 'fan', 
        'feeder'] or by sigalname ['set_interval_light',
        'set_interval_fan','set_interval_feeder']"""
        dict_components = { 'light': 'set_interval_light',
                            'fan': 'set_interval_fan',
                            'feeder': 'set_interval_feeder',}
        if component in dict_components.keys():
            #TODO
            pass
        

class AquariumECU(ECU_Variables):
    def __init__(self) -> None:
        super().__init__()
        self.currentTTY = None
        self.connectDevice()

    def sendSerialDataCheck(self):
        'send check request to the controller and await update of variables'
        if self.currentTTY:
            self.ECU.write(b"check\n")
            time.sleep(0.1)
            #while True:
            for key in self.outputSignals:
                self.outputSignals[key] = \
                    self.convertOutputValue(
                        self.ECU.read_until().decode("utf-8".strip()).replace('\r\n', ''),
                        key
                    )

        return self.outputSignals

    def sendSerialDataSet(self):
        #TODO
        self.ECU.write(b"set\n")
        time.sleep(0.1)
        # self.ECU.write(b"sync_time 1\n")
        # self.ECU.write("set_mode 1".encode())
        # if self.currentTTY:
        #     for key in self.inputSignals:
        #         if type(self.inputSignals[key]) != list:
        #             message = f"{key} {self.inputSignals[key]}"
        #         else:
        #             item_list = ""
        #             for item in self.inputSignals[key]:
        #                 item_list += str(item) + " "
        #             message = f"{key} {item_list}"
        #         print(message)
        #         self.ECU.write((message + "\n").encode())
                
        #         time.sleep(0.5)

        if self.currentTTY:
            self.timeUpdate()
            message = ""
            for key in self.inputSignals:
                if key == 'sync_time':
                    for val in self.inputSignals[key].split(':'):
                        message += f"{val}#"
                elif type(self.inputSignals[key]) != list:
                    message += f"{self.inputSignals[key]}#"
            print(message)
            self.ECU.write((message + "\n").encode())
            time.sleep(0.1)
            for _ in range(7):
                print(self.ECU.read_until().decode("utf-8".strip()).replace('\r\n', ''))
    
    def sendSerialDataIntervalSet(self, component : str):
        'for component use ["light", "fan", "feeder"]'
        #TODO
        dict_components = { 'light': 'set_interval_light',
                            'fan': 'set_interval_fan',
                            'feeder': 'set_interval_feeder',}
        if component in dict_components.keys():
            
            self.ECU.write(f"set_{component}_interval\n".encode())
            time.sleep(0.05)
            if self.currentTTY:
                self.timeUpdate()
                message = ""
                
                for val in self.inputSignals['sync_time'].split(':'):
                    message += f"{val}#"
                
                intervalSignal = self.inputSignals[dict_components[component]]
                for val in intervalSignal:
                    message += f"{val}#"
                
                
                
                # for key in self.inputSignals:
                #     if key == 'sync_time':
                #         for val in self.inputSignals[key].split(':'):
                #             message += f"{val}#"
                #     #elif type(self.inputSignals[key]) == list:
                #     elif key == dict_components[component]:
                #         for val in self.inputSignals[key]:
                #             message += f"{val}#"
                        
                print(message)
                self.ECU.write((message + "\n").encode())
                time.sleep(0.1)
                for _ in range(len(intervalSignal)+1):
                    print(self.ECU.read_until()
                        .decode("utf-8".strip())
                        .replace('\r\n', ''))
                #print('ende')
        else:
            print("wrong componet. Use one od ['light', 'fan', 'feeder']")
    
    def sendSerialTimeUpdate(self):
        "send current time to ECU"
        self.ECU.write(b"sync_time\n")
        time.sleep(0.05)
        self.timeUpdate()
        message = ""
        for val in self.inputSignals['sync_time'].split(':'):
            message += f"{val}#"
        
        print(message)
        self.ECU.write((message + "\n").encode())
        print(self.ECU.read_until()
                        .decode("utf-8".strip())
                        .replace('\r\n', ''))
        
    
    def connectDevice(self):
        try:
            self.ECU = serial.Serial(TTY_possibilities[0], 9600)
            self.currentTTY = TTY_possibilities[0]
        except:
            try:
                self.ECU = serial.Serial(TTY_possibilities[1], 9600)
                self.currentTTY = TTY_possibilities[1]
            except:
                print('no USB device found')
        if self.currentTTY:
            print('connected to', self.currentTTY)
    
    def disconnectDevice(self):
        if self.currentTTY:
            try:
                self.ECU.close()
                print('disconnected from', self.currentTTY)
            except:
                print('error by disconnecting from', self.currentTTY)
        else:
            print('no device to disconnect')

if __name__ == "__main__":
    print('hi')
    test = AquariumECU()
    print(test.convertOutputValue('read_interval_light 20 30 40 50', 'read_interval_light'))
    
    test.setInputValues2(set_mode = 1, read_interval_light = [20, 30, 40])
    print(test.inputSignals)
