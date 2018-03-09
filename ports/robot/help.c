/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/builtin.h"

const char stm32_help_text[] =
"Welcome to MicroPython!\n"
"\n"
"For online help please visit http://micropython.org/help/.\n"
"\n"
"Quick overview of commands for the board:\n"
"  robot.info()    -- print some general information\n"
"  robot.delay(n)  -- wait for n milliseconds\n"
"  robot.millis()  -- get number of milliseconds since hard reset\n"
"  robot.Switch()  -- create a switch object\n"
"                   Switch methods: (), callback(f)\n"
"  robot.LED(n)    -- create an LED object for LED n (n=1,2,3,4)\n"
"                   LED methods: on(), off(), toggle(), intensity(<n>)\n"
"  robot.Pin(pin)  -- get a pin, eg robot.Pin('X1')\n"
"  robot.Pin(pin, m, [p]) -- get a pin and configure it for IO mode m, pull mode p\n"
"                   Pin methods: init(..), value([v]), high(), low()\n"
"  robot.ExtInt(pin, m, p, callback) -- create an external interrupt object\n"
"  robot.ADC(pin)  -- make an analog object from a pin\n"
"                   ADC methods: read(), read_timed(buf, freq)\n"
"  robot.DAC(port) -- make a DAC object\n"
"                   DAC methods: triangle(freq), write(n), write_timed(buf, freq)\n"
"  robot.RTC()     -- make an RTC object; methods: datetime([val])\n"
"  robot.rng()     -- get a 30-bit hardware random number\n"
"  robot.Servo(n)  -- create Servo object for servo n (n=1,2,3,4)\n"
"                   Servo methods: calibration(..), angle([x, [t]]), speed([x, [t]])\n"
"  robot.Accel()   -- create an Accelerometer object\n"
"                   Accelerometer methods: x(), y(), z(), tilt(), filtered_xyz()\n"
"\n"
"Pins are numbered X1-X12, X17-X22, Y1-Y12, or by their MCU name\n"
"Pin IO modes are: robot.Pin.IN, robot.Pin.OUT_PP, robot.Pin.OUT_OD\n"
"Pin pull modes are: robot.Pin.PULL_NONE, robot.Pin.PULL_UP, robot.Pin.PULL_DOWN\n"
"Additional serial bus objects: robot.I2C(n), robot.SPI(n), robot.UART(n)\n"
"\n"
"Control commands:\n"
"  CTRL-A        -- on a blank line, enter raw REPL mode\n"
"  CTRL-B        -- on a blank line, enter normal REPL mode\n"
"  CTRL-C        -- interrupt a running program\n"
"  CTRL-D        -- on a blank line, do a soft reset of the board\n"
"  CTRL-E        -- on a blank line, enter paste mode\n"
"\n"
"For further help on a specific object, type help(obj)\n"
"For a list of available modules, type help('modules')\n"
;
