ret 0

ret_addr: => addr i32
    [I] :: 1 =[]
    ret I

ret_addr_nullable: => addr! i32
    [I] :: 1 =[]
    ret I

eret_test2: => !addr i32
    eret
