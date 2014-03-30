/*
 *  comm_tty_2.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-30
 *
 *  Graduate the value of a Analog / PWM led
 */

"use strict"

var CommTTY = require("../clients/comm_tty").CommTTY

var comm = new CommTTY()

var pin = 11
var brightness = 0
var delta = 8

var timeoutId = timers.setInterval(function() {
    comm.analog_write(pin, brightness % 256)
    brightness += delta
}, 50)
