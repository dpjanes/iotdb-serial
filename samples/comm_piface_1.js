/*
 *  comm_piface.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-29
 *
 *  Change the lights on a PiFace
 *  board every time someone presses
 *  button 0
 */

"use strict"

var CommPiFace = require("../clients/comm_piface").CommPiFace

var comm = new CommPiFace()
var BUTTON = 0
var led = 0
var on = 1

comm.digital_listen(BUTTON, function(pin, value) {
    console.log("+ result", pin, value)
    if (!value) {
        return
    }

    comm.digital_write(led++, on)
    if (led == 8) {
        led = 0
        on = on ? 0 : 1
    }
})
