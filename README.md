[![NPM Version](https://img.shields.io/npm/v/node-quirc.svg)](https://npmjs.org/package/node-quirc)
![Test](https://github.com/kAworu/node-quirc/workflows/Test/badge.svg)

# node-quirc
A Node.js Addon of the quirc library (QR decoder library - https://github.com/dlbeer/quirc).

# installation
First, You need libpng and libjpeg (and their header files) installed. Then, simply
```
% npm install node-quirc
```

# documentation
node-quirc aim to be simple to use, the module exposes a `decode()` function
and a `constants` object.


## decode(img[, callback])
`img` must be either a `Buffer` of a PNG or JPEG encoded image file, or a
decoded image in [`ImageData`](https://developer.mozilla.org/en-US/docs/Web/API/ImageData)
format with 1 (grayscale), 3 (RGB) or 4 (RGBA) channels.

When `callback` is provided, it is expected to be a "classic" Node.js callback
function, taking an error as first argument and the result as second argument.
Because the provided image file may contains several QR Code, the result is
always an array on success.

When `decode` is called only with `img` as argument, a `Promise` is returned.

```javascript
const fs    = require("fs");
const quirc = require("node-quirc");

// load the image file
const img = fs.readFileSync("./test/data/Hello+World.png");

// callback version
quirc.decode(img, (err, codes) => {
    if (err) {
        // handle err.
        console.error(`decode failed: ${err.message}`);
    } else {
        // do something with codes.
        console.dir(codes);
        console.log(codes.map((code) => code.data.toString('utf8')));
    }
});

// Promise version
quirc.decode(img).then((codes) => {
    // do something with codes.
}).catch((err) => {
    // handle err.
});

// alternatively, use an already-loaded ImageData, e.g. from the `canvas` library
const context = canvas.getContext('2d');
const imageData = context.getImageData(0, 0, 800, 600);
quirc.decode(imageData).then((codes) => {
    // do something with codes.
    console.log(`codes read from ImageData (size=${
        imageData.data.length
    }, width=${imageData.width}, height=${
        imageData.height
    }):`, codes);
}).catch((err) => {
    // handle err.
});
```

output:

```
[
  {
    version: 1,
    ecc_level: 'H',
    mask: 1,
    mode: 'BYTE',
    eci: 'UTF_8',
    data: Buffer [Uint8Array] [ 72, 101, 108, 108, 111 ]
  },
  {
    version: 1,
    ecc_level: 'H',
    mask: 3,
    mode: 'BYTE',
    eci: 'UTF_8',
    data: Buffer [Uint8Array] [ 87, 111, 114, 108, 100 ]
  }
]
[ 'Hello', 'World' ]
```

## constants
see https://github.com/kAworu/node-quirc/blob/master/index.js#L26-L56


# testing
Clone the repo and then simply
```
% npm install && npm test
```

# license
MIT, see [LICENSE](./LICENSE).
