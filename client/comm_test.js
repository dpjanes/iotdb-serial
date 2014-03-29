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
        program: "python",
        av: [ "comm_test.py", ]
    })
}

CommTest.prototype = new comm.Comm;

exports.CommTest = CommTest

if (1) {
    var comm = new CommTest()

    comm.digital_listen(1, function(pin, value) {
        console.log("+ result", pin, value)
    })
}

