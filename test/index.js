var quirc = require('../index.js');
var chai   = require("chai");
var expect = chai.expect;

describe("decode", function () {
    it.skip('should work', function (done) {
        quirc.decode(function (err, result) {
            expect(err).to.not.exist;
            expect(result).to.exist.and.to.eql("Hello");
            return done();
        });
    });
});
