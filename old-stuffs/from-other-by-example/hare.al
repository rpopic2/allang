// https://harelang.org/tutorials/introduction/

// 'Getting started' section

"Hello world!"n .print =>

// 'Error handling in "Hello world!"
// plain print function don't report failure.
// use `print to` to detect errors.
// it give more fine-grained control such as setting buffer.

Stdout :: std.io{
    .To std.Out
    .Buffer *u8{undefined} =[]
}
"Hello world!"n .print to Stdout =>
    ! panic "print failed"n


// 'Functions & parameters'

&Buf :: 1024*u8\0{undefined}
User :: ask_name &init Buf =>
greet User =>

ask_name: *u8\0 &init Buffer => *u8
    "Hello! Please enter your name:"n .print =>
    ret Buffer .scan => ? panic "Buffer overflowed"

greet: *u8\0 =>
    "Hello, ". print =>

// 'Using const & let to define variables'

// everything is immutable except when you append & in front of an identifier
// this makes identifier modifiable in current scope
// in allang, const means compile-time. the equivalant is immutable in allang.

// unlike in hare's example, drainig file does changes stream pointer in the file
// also note that Source is prefixed with & to notate that .read_all is mutating the file.

&Source :: std.open "hare.al" =>
    ! "Failed to open file"n .print => eret
#defer . close =>
Source ::
    This := 4096*u8 \0{undefined}
    &Source .read_all to This =>
Source :: Source .split "\n" =>
first3 Source =>

// actually, there is a better way than shadowing variables
// no need to mark file mutable

Lines :: {
    std.open "hare.al" =>
        ! "Failed to open file"n .print => eret
    #defer . close =>
    .Buffer 4096*u8 \0{undefined}
    . read_all to Buffer =>
    .This . split "\n"
}
first3 Lines
    ! "there are less than three lines in this file!"n peret
=>

I, J :: 1337, 42
""I" + "J" = "(I + J)""n .print =>
J := I
""I" + "J" = "(I + J)""n .print =>

first3: Lines 3*str =>
    "The first three lines of hare.al are here:"n
    Lines.0""n
    Lines.1""n
    Lines.2""n
    .print =>

// 'More about types'

A, B, C, D :: i32{10}, u32{10}, u8{10}, f32{13.37}

// no inferring from a suffix.

A :: "hi"   // *u8\0. array of u8 terminated by 0
B :: {42, 24}   // 2*i32. there is no notion of tuples. they are arrays when these are the same type, structs when types are different. array, struct are just equivalant representation, only their sematics are different.
C :: {.X 10 .Y 20} // anonymous struct.
C2 :: {10, 20} . #std.coords  // type is 2*i32. now you can do C2.X C2.Y without making it struct. it is accepting std.coords for accessing members.
C3 :: {.X i32{10} .Y i64{20}} // now it makes struct with 32 bit padding between two fields.
D :: 4*i32{1, 2, 3, 4}  // arrays have comptime-known bounds
E :: slice i32<1, 2, 3, 4>   // slices have runtime-known bounds
// angled bracket is used to get pointer to the anonymous data in the text section. just like string literals. these can be addressed with arrays too.
E2 :: addr 4*i32<1, 2, 3, 4>


// 'Struct and tuple types'

coords:
    struct { 2*i32 } . #std.coord

Player1 :: {
    .X 10 .Y 20
}
Player2 :: coords{
    .Y 10 .X 20
}
Player3 :: {42, 24}

// the cool part of allang:

print_coords.
Player1 . =>
Player2 . =>
Player3 . =>

print_coords: coords@ // this macro will expand coords' field. so this function takes two i32's as parameter. on the call site, structs are destructured.
    "(".X", ".Y")"n .print =>


// 'Arrays and slices'
&X :: *i32 {1, 3, 3, 7}
std.expect X .len @ is 4 => use len macro to get the length.
std.expect X.3 is 7 => // .3 is a static indexing.
&X.3 := 8
std.expect X.3 is 8 =>

Y :: 1024*i32{. 1 . 3 . 3 . 7 .. 42}
print_vals Y.0..4
print_vals Y.2..8

print_vals: In slice i32 =>
    "input: "In.Len" integers"n .print =>   // slice contains .Len member
    loop:
        #once &I :: 0
        "In["I"]: "([In * I] ! loop.break->)""n
    // enjoy easy bound checking and breaking out of loop with ! syntax
    ""n .print =>



// 'Stack allocation & pass by reference'
// there is no address-of operator.

&I :: 10
    . print =>
    increment &I =>
    . print =>

Hash ::
    sha256.new @ =[]    // this is on the stack.

    File :: std.open "hare.al" =>
        ! peret "faile to open file"n
    #defer File .close =>

    io.copy This, File          // using name Hash takes address of Hash
        ! peret "faile to create hash"n

Sum ::
    #sha256.size*u8{.. 0}
    Hash .sum Sum =>

hex.encode Sum .print =>
""n .print =>

increment: & addr i32 =>
    [.] + 1 =[.] // load, add, store


// 'Dynamic memory allocation & defer'

(Argv **u8\0)
X :: 42 =[std.heap.alloc =>]
" X: "X .print =>
"*X: "[X] .print =>
std.heap.free X~ =>
// you cannot access X after freeing. ~ notation denotes that

&File ::
    std:open
        [Argv * 1]
            ! peret "file name expected"n
    =>
        ! peret "failed to open file"n
#defer File close =>

&Buffer :: 65535*u8{.. 0} =[std.heap.alloc =>]
#defer std.heap.free Buffer =>

N ::
    &File. std.read to &Buffer ? "buffer overflowed"n =>
    usize{.}
Buffer * 0..N .print =>


// 'Static allocation'
// global variables are much verbose, as they can have undesired side effects.
global:
    &Items: 4*i32{1, 3, 3, 7}

(Argv slice slice u8 \0)
print_items =>
[&global.Items.3] := 1
[&global.Items.0] := 7
print_items =>

3 .times @ increment => .print =>

&File ::
    [Argv * 1] ! peret "file name expected"n
    std.open . => ! peret "failed to open file"n
#defer File .close =>

&Buffer: 65535*u8\0{.. 0}

Bytes_Read :: File .read to &Buffer
    ! peret "failed to read"n
Buffer * 0..Bytes_read ! peret "out of bounds access"
.print =>

print_items: =>
    #unroll for I in 0..4 excl
        [global.Items.#I] .print =>

increment: => i32
    x: 41
    ret [x] + 1 =[]



// 'Thinking in terms of ownership'
// there is no notion of ownership, however allang helps you deal with lifetimes of dynamic objects.
&File :: std.open "hare.al" => ! peret "unable to open file"
#defer File .close =>

Buffer ::
    hare.io.drain &File =>
#defer hare.free Buffer =>

String :: hare.strings.from_utf8 Buffer =>
. print =>



// 'Handling errors'

(Argv **u8 \0)

&File ::
    Path :: [Argv * 1] ! "path required on arg 1" .print => eret
    OFlags :: flag{wronly} or flag{trunc}

    os:create Path, 0o644, OFlags =>
        was {noaccess} ? `Error opening `Path`: Access denied` .print => eret
        was {fs.error} ? `Error opening `Path`: `err .print => eret
#defer . close =>
        was {ok, File} File =This

Buf :: "Hello world!"n // not actually a buffer here, but a slice to a static string

Buf .print to &File =>
    was {io.error} ? `Error writing to file: `error .print => eret
    was {ok, z} ?
        z is Buf.Len ! "Unexpected short write" panic

// 'Propagating erros' section

(Argv **u8 \0)

Path :: [Argv * 1] ! "path required on arg 1" .print => eret

writehello Path =>
    was {fs.error} ? `Error writing `Path`: `fs.error .print => eret
    was {io.error} ? `Error writing `Path`: `io.error .print => eret

writehello: Path *u8 \0 => !fs.error!io.error
    &File ::
        OFlags :: flag{wronly} or flag{trunc}
        os.create Path, 0o644, OFlags => ! eret
    #defer . close =>
    Buf :: "Hello world!"n
    Buf .print to &File => ! eret

// 'Defining new error types' section

prompt =>
    ? .print => eret

invalid: struct { !strconv.invalid !strconv.overflow }
unexpected_eof: struct { ! }
error: struct { @io.error @invalid @unexpected_eof }

str_error: Err error => slice u8
#leaf
    !strconv.invalid
        ret "Expected a positive number"
    !unexpected_eof
        ret "Unexpected end of file"
    !io.error
        ret io.str_error Err =>

prompt: !error =>
    "Please enter a positive number:"n .print =>
    Num :: get_number => ! eret
    ``Num` + 2 is `(Num + 2) .print =>

get_number: !error =>
    &Buf :: 1024*u8 \0 undefined
    std.scan.i32 to &Buf => !eof eret unexpected_eof
    ret strconv.stou Buf =>

// 'Testing your code' section
// notice how simpler it becomes with load multiple syntax ([0..1]) and bound checking with !

bubble_sort: Items *i32 =>
    #once
        It :: iter i32{Items}

    is &Sorted :: {Sorted}
    loop:
        #defer It++
        Cur, Next :: [It * 0..1] ! break;
        Cur > Next ?
            [It * 0..1] := {Next, Cur}
            &Sorted := {isnt Sorted}
        loop->
    is Sorted ? ret
    bubble_sort->

Items :: *{5, 4, 3, 2, 1}
bubble_sort Items =>
loop:
    #once It :: iter{Items}
    Cur, Next :: It[0..1]++ ! break
    std.expect Cur <= Next ! "list is unsorted"n =>

// 'if & switch statements' section

color:
    #enum red, orange, yellow, green, blue, violet
    to_string: #color .to_string_lookup @ .capitalize @
    // no need to write colorstr by hand. let compiler do it for you.
    // this generates color lookup table in the code section.

stock:
    struct {
        .Color u8 #color
        .Amount i32
    }

    print: Item stock@ =>
        Item.
        ``[color.to_string * .Color]` paint\t`
        .Amount` litter`(.Amount is 1 ? "" : "s")``n .print =>


Stock :: *stock@{
    {red}, 1}, {blue}, 6}, {violet}, 1}, {orange}, 4}
}
`Inventory:`Stock.0``Stock.1``Stock.2``Stock.3 .print n =>


// .. you need NOT to write this by hand!!
// but to show you what switch statement looks like in allang
colorstr: C color => *u8
    ret C is.
        {red} ? "Red"
        {orange} ? "Orange"
    // ...

// better way is to have it on array, not switch statement/expressions.
colorstr: * #color *u8 {
    "Red", "Orange", "Yellow" // ...
}

// now access them like..
[colorstr * red]    // now this will point to "Red"



// section 'Using yield'

(Argv **u8 \0)

File ::
    Name :: [Argv * 1] ! "First argument required"n .print to std.Err => eret
    os.open Name =>
        ! "Unable to open "Name =>

io.copy std.Out, File =>
    ! "copy error"n .print =>

// section 'For loops'

Items :: *{"Hello", "world!", "allang", "is", "cool!"}

I :: loop:
    #once This := 0
    #defer I :+ 1

    [Items * I]
        ! This := !-1
        break loop
    . equals "allang" ? break loop

""EOF "allang" is at index EOF""I .print =>

// section 'For-each loops'

Itmes :: {
    . *{"apple", "banana", "carrot"}
    .Counter usize{0}
}

// just use iterators instead of this iterator function!

// for-each value
loop:
    #once It :: iter{Items}
    Item :: It[]++ ! loop.break->
    .print =>

// for-each address (originally reference)
loop:
    #once It :: iter{Items}
    Item_Addr :: It++ ! loop.break->
    Item_Addr" = "[Item_Addr] .print =>

// for-each macro

foreach .. Items @
    . print =>


// unrolled loops
#unroll foreach I .. Items @ . print =>
// after unrolling, this will become three print statements


// section 'Flow control'

Items :: *{"Hello", "world!", "allang", "is", "cool!"}

foreach .. Items
    . equals "allang" ? loop->
    . print =>

// section 'The `never` type
// we'll never have never types

color:
    #enum red, orange, yellow, green, blue, violet

Color :: #color blue

// probably don't do this
Name ::
    Color is .
    {red}   ? "Red" >>
    {orange} ? "Orange" >>
    {yellow} ? "Yellow" >>
    {green} ? panic "green is a creative color"n
    {blue}  ? "Blue" >>
    {violet} ? "Violet" >>
    <<
"your color is "Name .print =>

// rather do this
Color :: #color blue
switch: * #color{
    .red "Red"
    .orange "Orange"
    .yellow "Yellow"
    .green !
    .blue "Blue"
    .violet "Violet"
}

Color is {green} ? panic "green is a creative color"n
[switch.Color]
    ! panic "choose a creative color"n
. print =>



// 'Castign & type assertions'

f32{13.37} .floor_to.i32 @ .print =>

x: enum union {
    i32
    u32
}
X :: x{i32{42}} =[]
std.expect [x] is {i32, _} =>
[x.i32] ! "x wasnt i32"
.print =>

// Y :: ?addr X    // you cannot have duplicate addr's, but raw_pointer can.
Y :: ?raw_pointer x{X}
std.expect Y =>
""Y" "Y x * unchecked 10""n .print => // addr type cannot pointer arithmetic, but raw_pointers can do. unchecked is required as it's unsafe to do so.

// unsafe things are made verbose
Z :: #reinterpret_cast Y i32 * 1
std.expect [Z] is 42 =>

// 'User-defined types'
index: struct { usize }
offs: struct { usize }
Z :: enum union{.index .offs 1337}
std.expect Z is {offs, _}

// 'Pointer types in depth'
// addr type cannot be null by default. also, addr does allow 0 to be addressed. however, the ! notation makes it to treat the type's zero value as error.
// this can be used for indexing error that returns -1. e.g. usize!-1. addr! is shorthand for addr!0
// note that this is distinc from enum union (tagged union)
coords: struct { 2*i32 } . #std.coords

Pos :: coords{.X 10 .Y 20} =[]
print_coords ! =>
print_coords Pos =>

print_coords: Pos addr! coords =>
    Pos ! "(null)"n
    : "("Pos.X", "Pos.Y")"n
    print =>



// 'Struct sub-typing'
// allang is NOT an oop language.

limit_stream: struct {
    .Stream hare.io.stream@
    .Sink hare.io.handle
    .Limit usize
}

global:
    limit_vtable: hare.io.vtable{
            .Writer addressof limit_write
            //...
        }

limit_writer: Sink hare.io.handle, Limit usize => &init addr limit_stream {
    ret {
        .Stream.Vtable global.limit_vtable
        .Stream.Fd 0
        .Sink Sink
        .Limit Limit
    }
}

limit_write: &St addr hare.io.stream, Buf slice u8 => usize!
    &St :: #reinterpret_cast limitstream{St}
    Buf ::
        Limit :: [St.Limit]
        This := Buf * 0..Limit ! Buf
    [&St.Limit] :- Buf.Len
    ret St.Sink .write Buf =>

Limit :: limit_writer std.Out, 5 =>
hare.fmt.fprintln limit, "Hello World!"n => ! panic

// 'Tagged unions in depth'

signed: struct enum union {
    .i8 .i16 .i32 .i64
}

unsigned: struct enum union {
    .u8 .u16 .u32 .u64
}

integer: struct enum union {
    unsigned@
    signed@
}

floating: struct enum union {
    .f32 .f64
}

numeric: struct enum union {
    integer@
    floating@
}

numeric_repr: struct {
    .Id u32
    union {
        .i8
        .i16
        .i32
        .i64
    }
}

Buf :: hare.strings.toutf8 "int" => hare.memio.fixed =>
Sc :: hare.bufio.newcanner Buf =>
Lexer :: hare.lexer.init Sc, "<string>" =>
Type :: hare.parse.type Lexer => ! panic
#defer hare.ast.type_finish Type =>
Store :: hare.types.store #hare.types.x86_64, !, ! =>
#defer hare.types.store_free Store =>
IType :: hare.types.lookup Store, Type => #reinterpret_cast . addr hare.types.Type
Obj :: numeric{1337} =[]
Ptr :: #reinterpret_cast raw_pointer numeric_repr{Obj}
std.expect [Ptr.Id] is IType.Id =>
std.expect [Ptr.i32] is 1337 =>

// 'Groawable slices'
"Enter a list of strings, then press <Ctrl+D>:"n .print =>
Lines ::
    std.dyn.new *u8\0 @

    Stdin :: std.io {
        .From std.In
        .Buffer 4096*u8{undefined}
    }

    loop:
    Line ::
        Stdin .read_line =>
        !v panic "buffer overflow"
        ! loop.break->
    Lines .push Line @
    loop->

#defer Lines .delete =>

"test line" =[Lines .alloc] % ! eret
Lines .insert at 0, % => ! eret

loop:
    #once It :: iter{Lines}
    It[] .equals "foobar" ?
        Lines .free It
        Lines .delete It
        It--
    It++

std.sort Lines std.str.cmp =>

"Your strings, sorted:"n .print =>
foreach Line .. Lines @
    Line""n .print =>

