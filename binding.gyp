{
  "targets": [
    {
      "target_name": "node-quirc",
      "sources": [
        "src/decode.c",
        "src/identify.c",
        "src/node-quirc.cc",
        "src/quirc.c",
        "src/version_db.c"
      ],
      "libraries": ["-lpng"],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ],
}
