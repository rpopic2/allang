_main:
    file: addr
    filename, readmode =>_fopen =[file]

    , 0, 2 =>_fseek
    [file] =>_ftell
    ret
test:
    ret

filename:
    "main.al"
readmode:
    "r"
