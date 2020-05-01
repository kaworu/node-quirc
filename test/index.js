'use strict';

/** @typedef {import('../index').QRCode} QRCode */

const fs   = require("fs");
const path = require("path");
const util = require("util");

const chai   = require("chai");
const expect = chai.expect;

const quirc = require("../index.js");

// QR-code versions.
const qr_versions = Array(40).fill(0).map((_, i) => i + 1);
// QR-code ECC levels.
const qr_ecc_levels = {
    ECC_LEVEL_M: "M",
    ECC_LEVEL_L: "L",
    ECC_LEVEL_H: "H",
    ECC_LEVEL_Q: "Q",
};
// QR-code encoding modes.
const qr_enc_modes = {
    MODE_NUMERIC: "NUMERIC",
    MODE_ALNUM:   "ALNUM",
    MODE_BYTE:    "BYTE",
    MODE_KANJI:   "KANJI",
};

const qr_eci = {
    ECI_ISO_8859_1:  "ISO_8859_1",
    ECI_IBM437:      "IBM437",
    ECI_ISO_8859_2:  "ISO_8859_2",
    ECI_ISO_8859_3:  "ISO_8859_3",
    ECI_ISO_8859_4:  "ISO_8859_4",
    ECI_ISO_8859_5:  "ISO_8859_5",
    ECI_ISO_8859_6:  "ISO_8859_6",
    ECI_ISO_8859_7:  "ISO_8859_7",
    ECI_ISO_8859_8:  "ISO_8859_8",
    ECI_ISO_8859_9:  "ISO_8859_9",
    ECI_WINDOWS_874: "WINDOWS_874",
    ECI_ISO_8859_13: "ISO_8859_13",
    ECI_ISO_8859_15: "ISO_8859_15",
    ECI_SHIFT_JIS:   "SHIFT_JIS",
    ECI_UTF_8:       "UTF_8",
};

const extensions = ["png", "jpeg"];

/**
 * helpers for test data files
 * @param {string} local_path
 * @returns {string}
 */
function test_data_path(local_path) {
    return path.join(__dirname, "data", local_path);
}

/**
 * @param {string} local_path
 * @returns {Buffer}
 */
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
        for (const [key, value] of Object.entries(qr_ecc_levels)) {
            it(`should set ${key} to ${value}`, function () {
                expect(quirc.constants[/** @type {keyof typeof qr_ecc_levels} */(key)]).to.exist.and.to.eql(value);
            });
        }
    });

    describe("QR-code encoding modes", function () {
        for (const [key, value] of Object.entries(qr_enc_modes)) {
            it(`should set ${key} to ${value}`, function () {
                expect(quirc.constants[/** @type {keyof typeof qr_enc_modes} */(key)]).to.exist.and.to.eql(value);
            });
        }
    });

    describe("QR-code ECI", function () {
        for (const [key, value] of Object.entries(qr_eci)) {
            it(`should set ${key} to ${value}`, function () {
                expect(quirc.constants[/** @type {keyof typeof qr_eci} */(key)]).to.exist.and.to.eql(value);
            });
        }
    });
});

describe("decode()", function () {
    describe("arguments", function () {
        it("should throw an Error when no arguments are given", function () {
            expect(function () {
                // @ts-ignore, but replace with @ts-expect-error with TS 3.9
                quirc.decode();
            }).to.throw(Error, "img must be a Buffer");
        });
        it("should return a Promise when only one argument is given", function () {
            const p = quirc.decode(Buffer.from("data"));
            expect(p).to.be.a("Promise");
            p.catch(() => { /* ignored */ });
        });
        it("should throw when img is not a Buffer", function () {
            expect(function () {
                // @ts-ignore, but replace with @ts-expect-error with TS 3.9
                quirc.decode("a string", function dummy() { });
            }).to.throw(TypeError, "img must be a Buffer");
        });
        it("should throw when callback is not a function", function () {
            expect(function () {
                // @ts-ignore, but replace with @ts-expect-error with TS 3.9
                quirc.decode(Buffer.from(""), "not a function");
            }).to.throw(TypeError, "callback must be a function");
        });
    });

    context("when the buffer data is empty", function () {
        it("should yield an Error", function (done) {
            quirc.decode(Buffer.from(""), function (err, _codes) {
                expect(err).to.exist.and.to.be.an("error");
                expect(/** @type {Error} */(err).message).to.eql("failed to load image");
                return done();
            });
        });
    });

    context("when the buffer data is not an image", function () {
        it("should yield an Error", function (done) {
            quirc.decode(Buffer.from("Hello World"), function (err, _codes) {
                expect(err).to.exist.and.to.be.an("error");
                expect(/** @type {Error} */(err).message).to.eql("failed to load image");
                return done();
            });
        });
    });

    extensions.forEach(function (ext) {
        context(`${ext}`, function () {
            context("when the image file has no QR Code", function () {
                /** @type {Buffer} */
                let empty_image;
                before(function () {
                    empty_image = read_test_data(`1x1.${ext}`);
                });

                it("should not yield an Error", function (done) {
                    quirc.decode(empty_image, function (err, _codes) {
                        expect(err).to.not.exist;
                        return done();
                    });
                });
                it("should not yield a result", function (done) {
                    quirc.decode(empty_image, function (_err, codes) {
                        expect(codes).to.be.an('array').and.to.have.length(0);
                        return done();
                    });
                });
            });

            context("when the image file has multiple QR Code", function () {
                /** @type {Buffer} */
                let hello_plus_world;
                before(function () {
                    hello_plus_world = read_test_data(`Hello+World.${ext}`);
                });

                it("should not yield an Error", function (done) {
                    quirc.decode(hello_plus_world, function (err, _codes) {
                        expect(err).to.not.exist;
                        return done();
                    });
                });
                it("should yield two results", function (done) {
                    quirc.decode(hello_plus_world, function (_err, codes) {
                        expect(codes).to.be.an('array').and.to.have.length(2);
                        return done();
                    });
                });
                it("should yield the first QR Code", function (done) {
                    quirc.decode(hello_plus_world, function (_err, codes) {
                        expect(codes).to.exist;
                        const [code] = /** @type {QRCode[]} */(codes)
                        expect('err' in code).to.be.false;
                        expect(code.version).to.eql(1);
                        expect(code.ecc_level).to.eql("H");
                        expect(code.mask).to.eql(1);
                        expect(code.mode).to.eql("BYTE");
                        expect(code.eci).to.eql("UTF_8");
                        expect(code.data).to.be.an.instanceof(Buffer);
                        expect(code.data.toString()).to.eql("Hello");
                        return done();
                    });
                });
                it("should yield the second QR Code", function (done) {
                    quirc.decode(hello_plus_world, function (_err, codes) {
                        expect(codes).to.exist;
                        const [, code] = /** @type {QRCode[]} */(codes)
                        expect('err' in code).to.be.false;
                        expect(code.version).to.eql(1);
                        expect(code.ecc_level).to.eql("H");
                        expect(code.mask).to.eql(3);
                        expect(code.mode).to.eql("BYTE");
                        expect(code.eci).to.eql("UTF_8");
                        expect(code.data).to.be.an.instanceof(Buffer);
                        expect(code.data.toString()).to.eql("World");
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
                /** @type {Buffer} */
                let big_image_with_two_qrcodes;
                before(function () {
                    big_image_with_two_qrcodes = read_test_data(`big_image_with_two_qrcodes.${ext}`);
                });

                it("should not yield an Error", function (done) {
                    quirc.decode(big_image_with_two_qrcodes, function (err, _codes) {
                        expect(err).to.not.exist;
                        return done();
                    });
                });
                it("should yield two results", function (done) {
                    quirc.decode(big_image_with_two_qrcodes, function (_err, codes) {
                        expect(codes).to.be.an('array').and.to.have.length(2);
                        return done();
                    });
                });
                it("should yield the first QR Code", function (done) {
                    quirc.decode(big_image_with_two_qrcodes, function (_err, codes) {
                        expect(codes).to.exist;
                        const [code] = /** @type {QRCode[]} */(codes)
                        expect('err' in code).to.be.false;
                        expect(code.version).to.eql(4);
                        expect(code.ecc_level).to.eql("M");
                        expect(code.mask).to.eql(2);
                        expect(code.mode).to.eql("BYTE");
                        expect(code.data).to.be.an.instanceof(Buffer);
                        expect(code.data.toString()).to.eql("from javascript");
                        return done();
                    });
                });
                it("should yield the second QR Code", function (done) {
                    quirc.decode(big_image_with_two_qrcodes, function (_err, codes) {
                        expect(codes).to.exist;
                        const [, code] = /** @type {QRCode[]} */(codes)
                        expect('err' in code).to.be.false;
                        expect(code.version).to.eql(4);
                        expect(code.ecc_level).to.eql("M");
                        expect(code.mask).to.eql(2);
                        expect(code.mode).to.eql("BYTE");
                        expect(code.data).to.be.an.instanceof(Buffer);
                        expect(code.data.toString()).to.eql("here comes qr!");
                        return done();
                    });
                });
            });
        });
    });

    context("using generated file", function () {
        const mode_to_data = {
            NUMERIC: "42",
            ALNUM:   "AC-42",
            BYTE:    "aA1234",
            KANJI:   [0x93, 0x5f, 0xe4, 0xaa], // 点茗 in Shift-JIS
        };

        /**
         * @param {QRCode['version']} version 
         * @param {QRCode['ecc_level']} ecc_level 
         * @param {QRCode['mode']} mode 
         */
        function test_filename(version, ecc_level, mode) {
            const fmt = "version=%s,level=%s,mode=%s.png";
            // pad version with a leading 0 if needed to "simulate" printf's
            // %02d format.
            const version_str = ("0" + version).slice(-2);
            return util.format(fmt, version_str, ecc_level, mode);
        }

        for (const version of qr_versions) {
            for (const [, ecc_level] of Object.entries(qr_ecc_levels)) {
                for (const [, mode] of Object.entries(qr_enc_modes)) {
                    const fname = test_filename(
                        version,
                        /** @type {QRCode['ecc_level']} */(ecc_level),
                        /** @type {QRCode['mode']} */(mode)
                    );
                    context(fname, function () {
                        // relative path for test_data_path() and
                        // read_test_data()
                        const rpath = "generated/" + fname;
                        // use accessSync(), because async it() calls
                        // won't register as expected.
                        let found = false;
                        try {
                            // XXX: TOCTOU but meh
                            fs.accessSync(test_data_path(rpath), fs.constants.R_OK);
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
                                    const [code] = /** @type {QRCode[]} */(codes);
                                    expect(code.version).to.eql(version);
                                    expect(code.ecc_level).to.eql(ecc_level);
                                    expect(code.mode).to.eql(mode);
                                    expect(code.data).to.be.an.instanceof(Buffer);
                                    expect(code.data).to.eql(Buffer.from(mode_to_data[code.mode]));
                                    return done();
                                });
                            });
                        }
                    });
                }
            }
        }
    });
});
