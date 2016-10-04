{
    "target_defaults": {
        "defines": [
            "QUIRC_MAX_REGIONS=65534"
        ]
    },

    "targets": [
        {
            "target_name": "quirc",
            "product_prefix": "lib",
            "type": "static_library",
            "sources": [
                "decode.c",
                "identify.c",
                "quirc.c",
                "version_db.c"
            ],
            "direct_dependent_settings": {
                "include_dirs": [
                    "."
                ],
            }
        }
    ]
}
