from leaf import LED, Pin, UART
import time
import re

led = LED(1)
sw = Pin("SW")
flash_flag = False
uart = UART(6)
uart.init(115200)

data_type = None
raw_data = ''
data = None

def flash():
    global raw_data
    print("Reading from BLE")
    while True:
        value = uart.read(20) # read up to 5 bytes
        if value:
            value = value.decode()
            print("%s"%value, end='')
            raw_data += value
            if verified():
                command_handler()

def verified():
    global raw_data, data_type, data, uart
    data_start = re.search('[*][$][A-Z]+[_]START_\d+[$][*]', raw_data)
    data_end = re.search('[*][$][A-Z]+[_]END[$][*]', raw_data)
    if data_start and data_end:
        print("Tansfer finished!")
        data_start = data_start.group(0)
        data_end = data_end.group(0)
        data_start_info = data_start.strip('*').strip('$').split('_')
        data_start_type = data_start_info[0]
        data_end_info = data_end.strip('*').strip('$').split('_')
        data_end_type = data_end_info[0]
        #start = data_start.start()
        #end = data_end.end()
        #raw_data = raw_data[start, end]
        if data_start_type == data_end_type:
            data_type = data_start_type
            print("Tranfer type: %s"%data_type)
            expect_data_length = int(data_start_info[2])
            data = raw_data.replace(data_start, '').replace(data_end, '')
            data_length = len(data)
            if expect_data_length == data_length:
                print("Verified!")
                uart.writechar(1)
                return True
            else:
                print("Verified error, expecting %s, received %s"%(expect_data_length, data_length))
                uart.writechar(0)
                return False

def command_handler():
    global data_type
    if data_type == "FILE":
        file_handler()
    else:
        print("ERROR, unknown command: %s" % data_type)

def file_handler():
    write_script()
    reload()

def write_script():
    global data
    #print(script)
    print("Writing file...")
    import os
    with open('main.py', 'w') as f:
        f.write(data)
    print("Done!")

def reload():
    from machine import reset
    reset()


while sw.value() == 0:
    print("switch down!")
    led.on()
    time.sleep(0.1)
    led.off()
    time.sleep(0.1)
    flash_flag = True

if flash_flag:
    print("switch up!")
    flash()
else:
    print("skip")