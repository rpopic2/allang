std.io

print: (s: addr c8)
    1, s, s =>std.string.strlen
<aarch64 macos>
    !mov x18, 0x61
    !svc 0
<>

scan: (s: addr c8 ->i32)
    acc: 0
    :: loop
        0, s, 1
<aarch64 macos>
        !mov x18, 0x60
        !svc 0
<>
        acc +:
        [s]
        :: ? ' ' le
            brk
        s +: szof c8
    acc
