/*
 *  comm_rpio.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-29
 */

"use strict"

var comm = require("./comm")

var CommRPIO = function() {
    this.setup({
        program: "sudo",
        av: [ "python3", "../server/comm_rpio.py", ]
    })
}

CommRPIO.prototype = new comm.Comm;

exports.CommRPIO = CommRPIO
