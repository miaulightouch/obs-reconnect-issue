# OBS Reconnect Issue Reproduce Plugin

This plugin broadcasts an upside-down (may be a bit bugged sometimes) video from the current scene.

## Requirements

1. RTMP server listening on `localhost:1935`.
2. Install this plugin.

## Reproduce Steps

1. Launch OBS with this plugin.
2. Open the stream in a video player (e.g., mpv):
    - `rtmp://localhost/app/bad`
    - `rtmp://localhost/app/good`
3. Shutdown the RTMP server for 3 seconds to make OBS reconnect itself.
4. Reopen streams to check the result:
    - The `bad` stream will no longer be upside-down and will sync with the program view when you change scenes or edit sources.
    - The `good` stream will remain upside-down and won't sync any modifications with the program view unless you edit the sources.
