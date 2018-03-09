# This file is executed on every boot (including wake-boot from deepsleep)

#import esp
#esp.osdebug(None)
import gc
import factory_reset
import inisetup

factory_reset.check_reset()
gc.collect()
inisetup.wifi()

