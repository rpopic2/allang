// comment at column 0, before any code
#declare printf: Format addr u8 => Num_Printed i32

    // comment indented under a top-level statement
    // (previously misread as the start of a new block)
printf "ok 1\n" =>

V ::
    3 + 2 =
      // comment indented 6 spaces inside a declaration block
    // comment indented 4 spaces
[Q] :: V =[]

  // comment with odd 2-space indentation between top-level statements
printf "ok 2\n" =>

        // deeply indented comment
   // odd-indent comment
// flush comment
printf "ok 3\n" =>

ret 0
