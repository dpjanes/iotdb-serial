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
        av: [ "../server/comm_test.py", ]
    })
}

CommTest.prototype = new comm.Comm;

exports.CommTest = CommTest
