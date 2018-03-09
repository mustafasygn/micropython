from machine import *
from machine import ADC as _adc
import ads1015

ADC = ads1015.ADS1015
i2c = I2C(scl=Pin(5),sda=Pin(4),freq=80000000)

def mapping(value, in_min, in_max, out_min, out_max):
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

def bytearray_from_hex(byte_list):
    buf = bytearray()
    for x in byte_list:
        buf.append(x)
    return buf

def soft_reset():
	import sys
	for key in sys.modules.keys():
		del sys.modules[key]
	import main

def get_battery():
    adc = _adc(0)
    analog_value = adc.read()/1024.0
    vbat = (120 * analog_value) / 20.0
    return vbat

class I2C(object):
    def __init__(self, addr):
        self.addr = addr

    def write(self, *datas):
        if len(datas) == 1:
            data = bytes([datas[0]]) # hex to bytes
            i2c.writeto(self.addr, data)
        elif len(datas) == 2:
            reg = datas[0]
            data = bytes([datas[1]])
            i2c.writeto_mem(self.addr, reg, data)
        else:
            print("ERROR: write() takes one or two arguements, example: write(reg, data) or write(data)")

    def read(self, *datas):
        if len(datas) == 1:
            nbytes = datas[0] # hex to bytes
            result = i2c.readfrom(self.addr, nbytes)
        elif len(datas) == 2:
            reg = datas[0]
            nbytes = datas[1]
            result = i2c.readfrom_mem(self.addr, reg, nbytes)
        else:
            print("ERROR: read() takes one or two arguements, example: read(reg, nbyte) or read(nbyte)")
            return False
        result = list(result)
        return result

    def scan(self):
        i2c.scan()
