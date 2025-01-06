_main:
    filename, readmode =>_fopen
    =>_fclose

    // .file: addr
    // filename, readmode =>_fopen =[file]
    // , 0, 2 =>_fseek =>_ftell
    ret

filename:
    "main.al"
readmode:
    "r"
