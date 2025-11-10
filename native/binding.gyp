{
  "targets": [{
    "target_name": "neutron_minimax",
    "include_dirs": [
      "include",
      "<!@(node -p \"require('node-addon-api').include\")"
    ],
    "dependencies": [
      "<!(node -p \"require('node-addon-api').gyp\")"
    ],
    "sources": [
      "src/Board.cpp",
      "src/cleaners.cpp",
      "src/FullMove.cpp",
      "src/gameutils.cpp",
      "src/minimax.cpp",
      "src/Move.cpp",
      "src/MinimaxAsyncWorker.cpp",
      "src/MinimaxAddon.cpp"
    ],
    "defines": [
      "NAPI_CPP_EXCEPTIONS"
    ],
    "cflags_cc": [
      "-Wall",
      "-std=c++20",
      "-O3",
      "-flto=auto",
      "-march=native",
      "-mtune=native",
      "-fexceptions"
    ]
  }]
}
