"use strict";

// Our C++ Addon
const addon = require('bindings')('node-quirc.node');

function decodeEncoded(img, callback) {
    return addon.decodeEncoded(img, callback);
}

function isImageDimension(number) {
    return (
        typeof number === 'number' &&
        number > 0 &&
        (number | 0) === number &&
        !isNaN(number)
    );
}

function decodeRaw(img, callback) {
    if (!isImageDimension(img.width)) {
        throw new Error(
            `unexpected width value for image: ${img.width}`
        )
    }
    if (!isImageDimension(img.height)) {
        throw new Error(
            `unexpected height value for image: ${img.height}`
        )
    }
    const channels = img.data.length / img.width / img.height;
    if (channels !== 1 && channels !== 3 && channels !== 4) {
        throw new Error(
            `unsupported ${channels}-channel image, expected 1, 3, or 4`
        );
    }
    return addon.decodeRaw(img.data, img.width, img.height, callback);
}

function maybePromisify(fn) {
    return (...args) => {
        if (args.length < fn.length) {
            return new Promise((resolve, reject) => {
                fn(...args, (err, results) => {
                    if (err) {
                        return reject(err);
                    } else {
                        return resolve(results);
                    }
                });
            });
        } else {
            return fn(...args);
        }
    };
}

// public API
module.exports = {
    decode: maybePromisify((img, callback) => {
        if (Buffer.isBuffer(img)) {
            return decodeEncoded(img, callback);
        } else if (img && typeof img === "object") {
            return decodeRaw(img, callback);
        } else {
            throw new TypeError("img must be a Buffer or ImageData");
        }
    }),
    constants: {
        // QR-code versions.
        VERSION_MIN:  1,
        VERSION_MAX: 40,
        // QR-code ECC levels.
        ECC_LEVEL_M: "M",
        ECC_LEVEL_L: "L",
        ECC_LEVEL_H: "H",
        ECC_LEVEL_Q: "Q",
        // QR-code encoding modes.
        MODE_NUMERIC: "NUMERIC",
        MODE_ALNUM:   "ALNUM",
        MODE_BYTE:    "BYTE",
        MODE_KANJI:   "KANJI",
        // Common character encodings
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
    },
};
