fn main:
    0 -> i
    loop:
        "%", i -> fmt -> print
        i, 1 -> + -> i
        < 10 ? loop
    ret

