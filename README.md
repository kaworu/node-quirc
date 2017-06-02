[![NPM Version](https://img.shields.io/npm/v/node-quirc.svg)](https://npmjs.org/package/node-quirc)
[![Build Status](https://travis-ci.org/kAworu/node-quirc.svg?branch=master)](https://travis-ci.org/kAworu/node-quirc)

# node-quirc
A Node.js Addon of the quirc library (QR decoder library - https://github.com/dlbeer/quirc).

# installation
```
% npm install node-quirc
```

# documentation
node-quirc aim to be simple to use, the module exposes a `decode()` function
and a `constants` object.


## decode(img, callback)
`img` must be a `Buffer` of a PNG encoded image file. Currently only PNG is
supported, but JPEG support is planned (see [#2](./issues/2)).

`callback` is a "classic" Node.js callback function, taking an error as first
argument and the result as second argument. Because the provided image file may
contains several QR Code, the result is always an array on success.
```javascript
const fs    = require("fs");
const quirc = require("node-quirc");

quirc.decode(fs.readFileSync("Hello+World.png"), (err, codes) => {
    if (err) {
        console.error(`decode failed: ${err.message}`);
    } else {
        console.dir(codes);
    }
});

/* output:
[ { version: 1,
    ecc_level: 'L',
    mask: 0,
    mode: 'BYTE',
    data: Buffer [ 72, 101, 108, 108, 111 ] },
  { version: 1,
    ecc_level: 'L',
    mask: 7,
    mode: 'BYTE',
    data: Buffer [ 87, 111, 114, 108, 100 ] } ]
*/
```

## constants
see https://github.com/kAworu/node-quirc/blob/master/index.js#L9


# testing
Clone the repo and then simply
```
% npm install && npm test
```

# license
MIT, see [LICENSE](./LICENSE).
