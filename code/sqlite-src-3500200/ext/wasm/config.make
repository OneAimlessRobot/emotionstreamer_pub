# Gets filtered by the configure script
bin.bash = /usr/bin/bash
bin.emcc = /mnt/FASTstorage/FASTprogs/emotionstreamer/code/sqlite-src-3500200/tool/emcc.sh
bin.wasm-strip = 
bin.wasm-opt = /mnt/FASTstorage/FASTprogs/emscr/emsdk/upstream/bin/wasm-opt

SHELL := $(bin.bash)

# The following overrides can be uncommented to test various
# validation and if/else branches the makefile code:
#
#bin.bash :=
#bin.emcc :=
#bin.wasm-strip :=
#bin.wasm-opt :=
