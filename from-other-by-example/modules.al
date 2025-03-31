// https://doc.rust-lang.org/stable/rust-by-example/mod/visibility.html

// al is dead simple about moduls..
// a file is a module.. directory orginises it!
// all labels are global(public) by default, if you want to make it local(private, or not global), prefix name with _.
// and also you could just use nested labels to origanize, which we will show you at the bottom

// see my-mod.al and my-mod/nested.al

// we use somewhat stupid c-style paste it here, except pasted file will be indented once and labeled with the file name without .al extension.
@paste my-mod.al

// that global or local label applies only when you link this binary from other binary. you can access all @paste-ed method whether it is private or not. _ prefix just tells you know that you should not use them unless you know what you're doing.


function: ()
    println!("called `function`");

function=>
my-mod.function=>
my-mod.indirect_access=>
my-mod.nested.function=>

my-mod.nested._private_function=> // while possible, highly unrecommended
my-mod._private_function=>  // also generates warning

@paste my.al

open_box :: my.OpenBox { contents: "public information" }
"The open box contains"open_box.contents print=>

#warning disable
closed_box :: my.ClosedBox { _contents: "classified information" } // generates warning, but ignored!
#warning enable

closed_box :: "classified information" @my.ClosedBox.new

"The cloesd box contains"closed.box._contents print=> // warning!

#alias my-mod.deeply.nested.my_first_function

my_first_function=>

my-mod.deeply.nested.
#alias .my_first_function
#alias .my_second_function
#alias .AndATraitType

my_first_function=>

#alias other_function deeply.nested.function

other_function=>
_:
crate.deeply.nested. // use instead of alias directive
    // #alias crate.deeply.nested.function // we do not let you declare it as there already is a function! instead use
    .function=>


function=>

// https://doc.rust-lang.org/stable/rust-by-example/mod/super.html

function: ()
    "called function()" print=>

cool:
    function: ()
        "called cool.function()" print=>
// or just cool.function: (), which saves one indent

my:
    function: ()
        "called my.function()" print=>

    cool:
        function: ()
            "called `my.cool.function()`" print=>

    indirect_call: ()
        "called `my::indirect_call()`, that\n> " print=>
        ../function=>    // #self just replaces to one outer scope
        function=>  // same stuff as above

    #self.cool.function=> // how to impl this?
    ../..function=>

// https://odin-lang.org/docs/overview/#packages

// for std packages, you can just use it right away.

"hi" std.print=>

#alias std.print    // you can use it by the name print
#alias foo: std.print   // use it as name foo
