/*
 *  comm.js
 *
 *  David Janes
 *  IOTDB.org
 *  2014-03-29
 *
 *  Manage GPIO-type device communications over the stdin / stdout.
 *
 *  Happy Arduino Day 2014
 */

"use strict"

var carrier = require('carrier')
var spawn = require('child_process').spawn

var Comm = function() {
}

Comm.prototype.setup = function(initd) {
    var self = this;

    self.program = spawn(initd.program, initd.av)
    self.sequence = 0
    self.callbackd = {}

    // console.log("HERE:A", initd)

    carrier.carry(self.program.stdout, function(line) {
        console.log('- stdout: ' + line);
        var parts = line.split(",")
        if (parts.length < 2) {
            return
        }

        var paramd = self.callbackd[parts[0]]
        if (!paramd) {
            return
        }

        if (parts[1] == "-1") {
            delete self.callbackd[parts[0]]
        } else {
            paramd.callback(paramd.pin, parseInt(parts[1]))
        }
    });
    carrier.carry(self.program.stderr, function(line) {
        console.log('- stderr: ' + line);
    });
}

Comm.prototype.digital_write = function(pin, value) {
    this.send({
        command: "DigitalWrite",
        pin: pin,
        value: value
    })
}

Comm.prototype.digital_read = function(pin, callback) {
    this.send({
        command: "DigitalRead",
        pin: pin,
        callback: callback
    })
}

Comm.prototype.digital_listen = function(pin, callback) {
    this.send({
        command: "DigitalListen",
        pin: pin,
        callback: callback
    })
}


Comm.prototype.send = function(paramd) {
    var self = this;

    var s = "" + ( self.sequence++ );
    if (paramd.callback) {
        self.callbackd[s] = paramd
    }

    var line = s;
    if (paramd.command !== undefined) {
        line += ","
        line += paramd.command;
    }
    if (paramd.pin !== undefined) {
        line += ","
        line += paramd.pin;
    }
    if (paramd.value !== undefined) {
        line += ","
        line += paramd.value;
    }
    line += "\n"

    console.log("- send", "seq=" + s)
    self.program.stdin.write(line)
}

exports.Comm = Comm
