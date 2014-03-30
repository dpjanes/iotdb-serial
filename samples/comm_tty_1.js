/*
 *  comm_tty_1.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-30
 *
 *  Blink a digital light every second
 */

"use strict"

var CommTTY = require("../clients/comm_tty").CommTTY

var comm = new CommTTY()

var pin = 7
var on = 0

var timeoutId = timers.setInterval(function() {
    comm.digital_write(7, on)
    on = on ? 0 : 1;
}, 1000)
