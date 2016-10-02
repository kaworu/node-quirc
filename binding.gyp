{
  "targets": [
    {
      "target_name": "node-quirc",
      "sources": [
        "src/node-quirc.cc",
        "src/node_quirc_decode.c",
        "src/quirc/decode.c",
        "src/quirc/identify.c",
        "src/quirc/quirc.c",
        "src/quirc/version_db.c"
      ],
      "libraries": ["-lpng"],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ],
}
