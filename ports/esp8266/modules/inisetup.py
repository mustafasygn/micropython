import uos
from flashbdev import bdev


def wifi():
    from machine import Pin, PWM
    import time
    import settings
    import webrepl
    import ubinascii
    import network

    status_led = Pin(2, Pin.OUT)
    led_status = 0
    led_beat = [100, 100, 200]

    hostname         = settings.hostname
    webrepl_password = settings.webrepl_password
    wifi_ssid        = settings.wifi_ssid
    wifi_password    = settings.wifi_password
    ap_ssid          = settings.ap_ssid
    ap_password      = settings.ap_password

    ap_if = network.WLAN(network.AP_IF)
    ap_if.active(True)
    sta_if = network.WLAN(network.STA_IF)
    sta_if.active(True)

    if hostname == '':
        hostname = b"ELF_" + ubinascii.hexlify(sta_if.config("mac")[-3:]).upper()
    if ap_ssid == '':
        ap_ssid = b"ELF-%s"%ubinascii.hexlify(ap_if.config("mac")[-3:]).upper()
    if ap_password == '':
        ap_password = b"sunfounder"

    ap_if.config(essid=ap_ssid, authmode=network.AUTH_WPA_WPA2_PSK, password=ap_password)
    sta_if.config(dhcp_hostname=hostname)

    if not sta_if.isconnected() and wifi_ssid != '':
        print('connecting to network...')
        sta_if.connect(settings.wifi_ssid, settings.wifi_password)
        start = time.ticks_ms()
        while True:
            if sta_if.isconnected():
                print('network config:', sta_if.ifconfig())
                status_led.value(0)
                time.sleep(2)
                status_led.value(1)
                break
            else:
                delta = time.ticks_diff(time.ticks_ms(), start)
                if delta > 10000:
                    sta_if.disconnect()
                    print('Unable to connect to %s'%settings.wifi_ssid)
                    break

            for delay in led_beat:
                status_led.value(0)
                time.sleep_ms(delay)
                status_led.value(1)
                time.sleep_ms(delay)

    webrepl.start(password=webrepl_password)

def check_bootsec():
    buf = bytearray(bdev.SEC_SIZE)
    bdev.readblocks(0, buf)
    empty = True
    for b in buf:
        if b != 0xff:
            empty = False
            break
    if empty:
        return True
    fs_corrupted()

def fs_corrupted():
    import time
    while 1:
        print("""\
The FAT filesystem starting at sector %d with size %d sectors appears to
be corrupted. If you had important data there, you may want to make a flash
snapshot to try to recover it. Otherwise, perform factory reprogramming
of MicroPython firmware (completely erase flash, followed by firmware
programming).
""" % (bdev.START_SEC, bdev.blocks))
        time.sleep(3)

def setup():
    from machine import Pin
    import factory_reset
    import time
    check_bootsec()
    print("Performing initial setup")
    uos.VfsFat.mkfs(bdev)
    vfs = uos.VfsFat(bdev)
    uos.mount(vfs, '/')
    factory_reset.reset()
    return vfs
