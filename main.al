_main:
    file: addr
    filename, readmode =>_fopen =[file]
    :: ? 0
        fileopen_err =>_printf
        1 =>_exit

    filelen: i64
    , 0, 2 =>_fseek
    [file] =>_ftell =[filelen]
    [file], 0, 0 =>_fseek

    srcbuf: addr
    [filelen] =>_malloc =[srcbuf]
    :: ? 0
        malloc_err =>_printf
        1 =>_exit

    , 1, [filelen], [file] =>_fread
    [file] =>_fclose

    [srcbuf] =>_printf
    parse_start =>_printf

    $resultbuf: addr
    0x1000 =>_malloc =$resultbuf

    0x10 =[$resultbuf]

    outfile: addr
    filename_out, writemode =>_fopen =[outfile]
    $resultbuf, 1, 0x100, [outfile] =>_fwrite
    [outfile] =>_fclose
    0 ret

filename:
    "easy.al"
filename_out:
    "easy.o"
readmode:
    "r"
writemode:
    "w"
fileopen_err:
    "could not open file"
malloc_err:
    "malloc failed"
parse_start:
    ":: parse start
"
