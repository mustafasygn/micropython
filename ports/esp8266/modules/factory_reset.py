
MAIN_PY = """
from machine import Pin
from time import sleep_ms

led = Pin(2, Pin.OUT)
print("Blink LEd Demo Runing!")
while True:
    led.value(0)
    sleep_ms(500)
    led.value(1)
    sleep_ms(500)
"""

SETTINGS_PY = """
# This file is to configure all settings

hostname = ''
webrepl_password = '12345678'

wifi_ssid = ''
wifi_password = ''

ap_ssid = ''
ap_password = ''

def set(name, value):
    f = open('settings.py', 'r')
    lines=f.readlines()
    f.close()
    file_len=len(lines)-1
    flag = False
    for i in range(file_len):
        if lines[i].startswith(name):
            lines[i] = "%s = '%s'\\n" % (name, value)
            flag = True
            break
    if not flag:
        raise ValueError('No setting param named: %s'%name)
    f = open('settings.py', 'w')
    f.write('')
    f.close()
    f = open('settings.py', 'w+')
    for line in lines:
        f.write(line)
    f.close()
"""
def reset():
    import uos
    factory_files = ["main.py", "settings.py"]
    for file in uos.listdir("/"):
        if file in factory_files:
            pass
        else:
            uos.remove("/"+file)

    with open("main.py", "w") as f:
        f.write(MAIN_PY)
    with open("settings.py", "w") as f:
        f.write(SETTINGS_PY)

    print("\n   Factory reset finished  \n")


def check_reset():
    from machine import Pin
    import time
    status_led = Pin(2, Pin.OUT)
    led_status = 0
    delay = 50
    btn = Pin(16,Pin.IN)
    delta = 0
    start = time.ticks_ms()
    print("Checking reset signal...")
    while not btn.value():
        delta = time.ticks_diff(time.ticks_ms(), start)
        print("Usr button holding...")
        if delta > 10000:
            print("Factory reseting...")
            status_led.value(0)
            time.sleep(1)
            reset()
            break
        led_status = not led_status
        status_led.value(led_status)
        time.sleep_ms(delay)
