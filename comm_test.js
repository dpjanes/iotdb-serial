/*
 *  comm_test.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-29
 */

"use strict"

var comm = require("./comm")

var CommTest = function() {
    this.setup({
    })
}

CommTest.prototype = new comm.Comm;

CommTest.prototype.digital_write = function(pin, value) {
    this.send({
        command: "DigitalWeite",
        pin: pin,
        value: value,
        callback: callback
    })
}

CommTest.prototype.digital_read = function(pin, callback) {
    this.send({
        command: "DigitalRead",
        pin: pin,
        callback: callback
    })
}

CommTest.prototype.digital_listen = function(pin, callback) {
    this.send({
        command: "DigitalListen",
        pin: pin,
        callback: callback
    })
}

exports.CommTest = CommTest

if (1) {
    var comm = new CommTest()

    comm.digital_listen(1, function(pin, value) {
        console.log("result", pin, value)
    })
}

