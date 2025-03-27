s: i32

main:
    i: i32

    j:
        =>print

    s       // adrp x0, s@PAGE; add x0, x0, s@PAGEOFF
    [s]     // adrp x0, s@PAGE; ldr w0, [x0, s@PAGEOFF]

    main,   // adrp x0, main@PAGE, add x0, x0, main@PAGEOFF
    [main]  // adrp x0, main@PAGE, ldr x0, x0, s@PAGEOFF

    i,      // add x0, sp, 0x0
    [i]     // ldr w0, [sp, 0x0]

    j,      // adrp x0, main.j@PAGE, add x0, x0, main.j@PAGEOFF
    [j]     // adrp x0, main.j@PAGE, ldr w0, [x0, j@PAGEOFF]

    p: { x: i32, y: i32 }   // obj @ 0x4

    p       // add x0, sp, 0x4
    [p]     // ldr x0, [sp, 0x4]

    [p.x]   // ldr w0, [sp, 0x4]
    [p.y]   // ldr w0, [sp, 0x8]

    a: i32 10               // obj @ 0xc
    a       // add x0, sp, 0xc
    [a]     // ldr x0, [sp, 0xc]
    [a, 1]  // ldr x0, [sp, 0x10]

    [a, i]  // ldr x0, [sp]; add x0, x0, 0x10; ldr x0, [sp, x0];
    [a, [i]]

    :: for $i 0, <10, ++ // $i = w8
        [a, $i]  // add x0, w8, 0xc ldr x0, [sp, w8]

    0 >$i
    $i ? 10
    :: lt

