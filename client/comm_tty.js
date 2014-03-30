/*
 *  comm_tty.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-30
 *
 *  Talk serial protocol by opening a TTY.
 *  Handy for things such as Arduino
 *
 *  You'll want to upload the comm_arduino.ino
 *  sketch to work with this.
 */

"use strict"

var comm = require("./comm")
var timers = require('timers')

var CommTTY = function(paramd) {
    if (paramd === undefined) {
        paramd = {}
    }

    this.setup({
        tty: paramd.tty ? paramd.tty : "/dev/tty.usbmodem621",
        baudrate: paramd.baudrate ? paramd.baudrate : 9600
    })
}

CommTTY.prototype = new comm.Comm;

exports.CommTTY = CommTTY
