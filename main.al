_main:
    file: addr
    filename, readmode =>_fopen =[file]
    [file] ? 0
    // :: [file] ? 0         // 
        // fileopen_err =>_printf

    filelen: i32
    , 0, 2 =>_fseek
    [file] =>_ftell =[filelen]
    [file], 0, 0 =>_fseek

    srcbuf: addr
    [filelen] =>_malloc =[srcbuf]

    , 1, [filelen], [file] =>_fread
    [file] =>_fclose

parse:
    i: i32
    0 ret

filename:
    "main.al"
readmode:
    "r"
fileopen_err:
    "could not open file"
