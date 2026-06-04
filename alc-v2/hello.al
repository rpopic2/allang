#declare printf: Format addr u8 => Num_Printed i32

printf "Hello World!\n" =>

print "Hello World"n .To &Stdout =>

"Hello World"n .print .To &Stdout =>


&Out :: 1024*u8!{undefined}
~ print Out .To &Stdout =>

write "Hello World"n .To &Out =>
write "Enjoy Buffered Output"n .To &Out =>



ret 0
