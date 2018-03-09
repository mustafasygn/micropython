/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014, 2015 Damien P. George
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
#ifndef MICROPY_INCLUDED_STMHAL_USB_H
#define MICROPY_INCLUDED_STMHAL_USB_H

#include "usbd_cdc_msc_hid0.h"

#define PYB_USB_FLAG_USB_MODE_CALLED    (0x0002)

// Windows needs a different PID to distinguish different device configurations
#define USBD_VID         (0xf055)
#define USBD_PID_CDC_MSC (0x9800)
#define USBD_PID_CDC_HID (0x9801)
#define USBD_PID_CDC     (0x9802)

typedef enum {
    PYB_USB_STORAGE_MEDIUM_NONE = 0,
    PYB_USB_STORAGE_MEDIUM_FLASH,
    PYB_USB_STORAGE_MEDIUM_SDCARD,
} robot_usb_storage_medium_t;

typedef enum {
    USB_PHY_FS_ID = 0,
    USB_PHY_HS_ID = 1,
} USB_PHY_ID;

extern mp_uint_t robot_usb_flags;
extern robot_usb_storage_medium_t robot_usb_storage_medium;
extern const struct _mp_obj_tuple_t robot_usb_hid_mouse_obj;
extern const struct _mp_obj_tuple_t robot_usb_hid_keyboard_obj;
extern const mp_obj_type_t robot_usb_vcp_type;
extern const mp_obj_type_t robot_usb_hid_type;
MP_DECLARE_CONST_FUN_OBJ_KW(robot_usb_mode_obj);
MP_DECLARE_CONST_FUN_OBJ_0(robot_have_cdc_obj); // deprecated
MP_DECLARE_CONST_FUN_OBJ_1(robot_hid_send_report_obj); // deprecated

void robot_usb_init0(void);
bool robot_usb_dev_init(uint16_t vid, uint16_t pid, usb_device_mode_t mode, USBD_HID_ModeInfoTypeDef *hid_info);
void robot_usb_dev_deinit(void);
bool usb_vcp_is_enabled(void);
int usb_vcp_recv_byte(uint8_t *c); // if a byte is available, return 1 and put the byte in *c, else return 0
void usb_vcp_send_strn(const char* str, int len);

void robot_usb_host_init(void);
void robot_usb_host_process(void);
uint robot_usb_host_get_keyboard(void);

#endif // MICROPY_INCLUDED_STMHAL_USB_H
