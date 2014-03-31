/*
 *  comm_tty_4.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-31
 *
 *  Listen to multipe analog inputs (such
 *  as from a JoyStick)
 */

"use strict"

var CommTTY = require("../client/comm_tty").CommTTY

var comm = new CommTTY()

var pin_input_x = 0
var pin_input_y = 1

var on_change = function(pin, value) {
    console.log("+ pin changed", pin, value)
}

comm.analog_listen(pin_input_x, on_change)
comm.analog_listen(pin_input_y, on_change)
