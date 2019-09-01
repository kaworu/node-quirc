var fs   = require("fs");
var path = require("path");
var util = require("util");

var chai       = require("chai");
var underscore = require("underscore");
var expect     = chai.expect;

var quirc = require("../index.js");

// QR-code versions.
var qr_versions = underscore.range(1, 40);
// QR-code ECC levels.
var qr_ecc_levels = {
    ECC_LEVEL_M: "M",
    ECC_LEVEL_L: "L",
    ECC_LEVEL_H: "H",
    ECC_LEVEL_Q: "Q",
};
// QR-code encoding modes.
var qr_enc_modes = {
    MODE_NUMERIC: "NUMERIC",
    MODE_ALNUM:   "ALNUM",
    MODE_BYTE:    "BYTE",
    MODE_KANJI:   "KANJI",
};

/* helpers for test data files */
function test_data_path(local_path) {
    return path.join(__dirname, "data", local_path);
}
function read_test_data(local_path) {
    return fs.readFileSync(test_data_path(local_path));
}

describe("constants", function () {
    describe("QR-code versions", function () {
        it("should set VERSION_MIN to 1", function () {
            expect(quirc.constants.VERSION_MIN).to.exist.and.to.eql(1);
        });
        it("should set VERSION_MAX to 40", function () {
            expect(quirc.constants.VERSION_MAX).to.exist.and.to.eql(40);
        });
    });

    describe("QR-code ECC levels", function () {
        underscore.each(qr_ecc_levels, function (value, key) {
            it("should set " + key + " to " + value, function () {
                expect(quirc.constants[key]).to.exist.and.to.eql(value);
            });
        });
    });

    describe("QR-code encoding modes", function () {
        underscore.each(qr_enc_modes, function (value, key) {
            it("should set " + key + " to " + value, function () {
                expect(quirc.constants[key]).to.exist.and.to.eql(value);
            });
        });
    });
});

describe("decode()", function () {
    describe("arguments", function () {
        it("should throw an Error when no arguments are given", function () {
            expect(function () {
                quirc.decode();
            }).to.throw(Error, "img must be a Buffer");
        });
        it("should return a Promise when only one argument is given", function () {
            const p = quirc.decode(Buffer.from("data"));
            expect(p).to.be.a("Promise");
            p.catch((e) => { /* ignored */ });
        });
        it("should throw when img is not a Buffer", function () {
            expect(function () {
                quirc.decode("a string", function dummy() { });
            }).to.throw(TypeError, "img must be a Buffer");
        });
        it("should throw when callback is not a function", function () {
            expect(function () {
                quirc.decode(new Buffer(""), "not a function");
            }).to.throw(TypeError, "callback must be a function");
        });
    });

    context("when the buffer data is empty", function () {
        it("should yield an Error", function (done) {
            quirc.decode(new Buffer(""), function (err, codes) {
                expect(err).to.exist.and.to.be.an("error");
                expect(err.message).to.eql("failed to load image");
                return done();
            });
        });
    });

    context("when the buffer data is not an image", function () {
        it("should yield an Error", function (done) {
            quirc.decode(new Buffer("Hello World"), function (err, codes) {
                expect(err).to.exist.and.to.be.an("error");
                expect(err.message).to.eql("failed to load image");
                return done();
            });
        });
    });

    context("when the image file has no QR Code", function () {
        var empty_image;
        before(function () {
            empty_image = read_test_data("1x1.png");
        });

        it("should not yield an Error", function (done) {
            quirc.decode(empty_image, function (err, codes) {
                expect(err).to.not.exist;
                return done();
            });
        });
        it("should not yield a result", function (done) {
            quirc.decode(empty_image, function (err, codes) {
                expect(codes).to.be.an('array').and.to.have.length(0);
                return done();
            });
        });
    });

    context("when the image file has multiple QR Code", function () {
        var hello_plus_world;
        before(function () {
            hello_plus_world = read_test_data("Hello+World.png");
        });

        it("should not yield an Error", function (done) {
            quirc.decode(hello_plus_world, function (err, codes) {
                expect(err).to.not.exist;
                return done();
            });
        });
        it("should yield two results", function (done) {
            quirc.decode(hello_plus_world, function (err, codes) {
                expect(codes).to.be.an('array').and.to.have.length(2);
                return done();
            });
        });
        it("should yield the first QR Code", function (done) {
            quirc.decode(hello_plus_world, function (err, codes) {
                expect(codes[0].err).to.not.exist;
                expect(codes[0].version).to.eql(1);
                expect(codes[0].ecc_level).to.eql("L");
                expect(codes[0].mask).to.eql(0);
                expect(codes[0].mode).to.eql("BYTE");
                expect(codes[0].data).to.be.an.instanceof(Buffer);
                expect(codes[0].data.toString()).to.eql("Hello");
                return done();
            });
        });
        it("should yield the second QR Code", function (done) {
            quirc.decode(hello_plus_world, function (err, codes) {
                expect(codes[1].err).to.not.exist;
                expect(codes[1].version).to.eql(1);
                expect(codes[1].ecc_level).to.eql("L");
                expect(codes[1].mask).to.eql(7);
                expect(codes[1].mode).to.eql("BYTE");
                expect(codes[1].data).to.be.an.instanceof(Buffer);
                expect(codes[1].data.toString()).to.eql("World");
                return done();
            });
        });
    });

    /*
     * This context() ensure that quirc was compiled with QUIRC_MAX_REGIONS > 256.
     *
     * see https://github.com/dlbeer/quirc/issues/2 for the test image file and
     * https://github.com/dlbeer/quirc/pull/9 for the rational.
     */
    context("when the image file is big", function () {
        var big_image_with_two_qrcodes;
        before(function () {
            big_image_with_two_qrcodes = read_test_data("big_image_with_two_qrcodes.png");
        });

        it("should not yield an Error", function (done) {
            quirc.decode(big_image_with_two_qrcodes, function (err, codes) {
                expect(err).to.not.exist;
                return done();
            });
        });
        it("should yield two results", function (done) {
            quirc.decode(big_image_with_two_qrcodes, function (err, codes) {
                expect(codes).to.be.an('array').and.to.have.length(2);
                return done();
            });
        });
        it("should yield the first QR Code", function (done) {
            quirc.decode(big_image_with_two_qrcodes, function (err, codes) {
                expect(codes[0].err).to.not.exist;
                expect(codes[0].version).to.eql(4);
                expect(codes[0].ecc_level).to.eql("M");
                expect(codes[0].mask).to.eql(2);
                expect(codes[0].mode).to.eql("BYTE");
                expect(codes[0].data).to.be.an.instanceof(Buffer);
                expect(codes[0].data.toString()).to.eql("from javascript");
                return done();
            });
        });
        it("should yield the second QR Code", function (done) {
            quirc.decode(big_image_with_two_qrcodes, function (err, codes) {
                expect(codes[1].err).to.not.exist;
                expect(codes[1].version).to.eql(4);
                expect(codes[1].ecc_level).to.eql("M");
                expect(codes[1].mask).to.eql(2);
                expect(codes[1].mode).to.eql("BYTE");
                expect(codes[1].data).to.be.an.instanceof(Buffer);
                expect(codes[1].data.toString()).to.eql("here comes qr!");
                return done();
            });
        });
    });

    context("using generated file", function () {
        var mode_to_data = {
            NUMERIC: "42",
            ALNUM:   "AC-42",
            BYTE:    "aA1234",
            KANJI:   [0x93, 0x5f, 0xe4, 0xaa], // 点茗 in Shift-JIS
        };

        function test_filename(version, ecc_level, mode) {
            var fmt = "version=%s,level=%s,mode=%s.png";
            // pad version with a leading 0 if needed to "simulate" printf's
            // %02d format.
            var version_str = ("0" + version).slice(-2);
            return util.format(fmt, version_str, ecc_level, mode);
        }

        underscore.each(qr_versions, function (version) {
            underscore.each(qr_ecc_levels, function (ecc_level) {
                underscore.each(qr_enc_modes, function (mode) {
                    var fname = test_filename(version, ecc_level, mode);
                    context(fname, function () {
                        // relative path for test_data_path() and
                        // read_test_data()
                        var rpath = "generated/" + fname;
                        // use accessSync(), because async it() calls
                        // won't register as expected.
                        var found = false;
                        try {
                            fs.accessSync(test_data_path(rpath), fs.R_OK);
                            found = true;
                        } catch (e) {
                            found = false;
                        }
                        if (!found) {
                            it.skip(rpath + " not found, skipped");
                        } else {
                            it("should yield the QR Code", function (done) {
                                var image = read_test_data(rpath);
                                quirc.decode(image, function (err, codes) {
                                    expect(err).to.not.exist;
                                    expect(codes).to.be.an('array').and.to.have.length(1);
                                    expect(codes[0].version).to.eql(version);
                                    expect(codes[0].ecc_level).to.eql(ecc_level);
                                    expect(codes[0].mode).to.eql(mode);
                                    expect(codes[0].data).to.be.an.instanceof(Buffer);
                                    expect(codes[0].data).to.eql(new Buffer(mode_to_data[mode]));
                                    return done();
                                });
                            });
                        }
                    });
                });
            });
        });
    });
});
