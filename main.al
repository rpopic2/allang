_main:
    // filename =>_printf
    // readmode =>_printf
    file: addr
    filename, readmode =>_fopen =[file]

    filelen: i32
    , 0, 2 =>_fseek
    [file] =>_ftell =[filelen]
    [file], 0, 0 =>_fseek

    0 ret

filename:
    "main.al"
readmode:
    "r"
