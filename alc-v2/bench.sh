
clang -DNDEBUG -O2 -std=c11 main.c diagnostics.c emit-aarch64-bin.c emit-macho-exe.c -o alc-bin && \
clang -DNDEBUG -O2 -std=c11 main.c diagnostics.c emit-aarch64.c emit-macos.c -o alc-clang && \
clang -DDEBUG_TIMER -O2 -std=c11 main.c diagnostics.c emit-aarch64-bin.c emit-macho-exe.c -o alc-bin-timer && \
clang -DDEBUG_TIMER -O2 -std=c11 main.c diagnostics.c emit-aarch64.c emit-macos.c -o alc-clang-timer && \
echo "all built"

N=200

bench() {
  local t
  t=$( { time (for i in $(seq $N); do eval "$2" 2>/dev/null; done); } 2>&1 | grep real | awk '{print $2}' )
  local ms
  ms=$(echo "$t" | awk -F'[ms]' '{printf "%.1f", ($1*60+$2)*1000/'$N'}')
  echo "  $1: ${ms}ms/run  (${t} total, ${N} runs)"
}

for src in hello arithmetic test large; do
  echo "=== tests/${src}.al ==="
  bench "bin (exe)   " "./alc-bin   tests/${src}.al"
  bench "clang (.s)  " "./alc-clang tests/${src}.al"
  bench "clang (exe) " "./alc-clang tests/${src}.al && clang tests/${src}.s -o tests/${src}"
  echo ""
done

rm alc-bin
rm alc-clang
rm alc-bin-timer
rm alc-clang-timer
