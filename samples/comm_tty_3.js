/*
 *  comm_tty_3.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-30
 *
 *  When a button is pressed, light a LED.
 *  This monitors the LED for updates
 */

"use strict"

var CommTTY = require("../client/comm_tty").CommTTY

var comm = new CommTTY()

var pin_input = 8
var pin_output = 7

comm.digital_listen(pin_input, function(pin, value) {
    console.log("+ pin changed", pin, value)
    comm.digital_write(pin_output, value)
})
