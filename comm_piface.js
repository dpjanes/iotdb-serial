/*
 *  comm_piface.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-29
 */

"use strict"

var comm = require("./comm")

var CommPiFace = function() {
    this.setup({
        program: "python3",
        av: [ "comm_piface.py", ]
    })
}

CommPiFace.prototype = new comm.Comm;

exports.CommPiFace = CommPiFace

var led = 0
var on = 1

if (1) {
    var comm = new CommPiFace()

    // comm.digital_write(1, 1)
    // comm.digital_write(2, 1)
    comm.digital_listen(0, function(pin, value) {
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
}
