proc scan.i32: i32
    acc: i32
    0 ->acc

.loop:
    tmp: u8
    [uart] ->tmp
    tmp +>acc
    cmp tmp, ' '
    :: !=
        ->.loop
    acc
    ret

// mixed c style?
proc scan() {
    acc: i32 = 0;

    .loop {
        tmp: u8 = [uart];
        acc += tmp;
        compare (tmp, ' ') {
            (!=) => goto .loop
        }
    }
    ret acc
}

// c style?
i32 scan() {
    i32 acc = 0;

    while (1) {
        u8 tmp = *uart;
        acc += tmp;
        if (tmp == ' ')
            break
    }
}

// rust style?
fn scan() -> i32 {
    let mut acc: i32 = 0;

    loop {
        let tmp: u8 = *uart;
        acc += tmp;
        if tmp == ' '
            break;
    }
}

// jai style

scan :: () ->i32 {
    acc := 0;

    loop {
        tmp := *uart;
        acc += tmp;
        if tmp == ' '
            break;
    }
}

// again al style, use reg

scan: ->i32
    0 :> acc

.loop:
    [uart] :> tmp
    tmp > acc
    tmp, ' '
    != goto .loop

// al , very sugary

scan: ->i32
    acc: 0

.loop:
    c: [uart]
    + acc, c
    c, ' '
    != goto .loop

    acc
