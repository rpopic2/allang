// https://gobyexample.com/closures

// no captures in allang...
// but it is easy to make it work like capturing

int_seq: (addr @int_seq self =>i32)
    tmp :: [self.i] + 1; =[]
    tmp ret

    @struct {
        i32 i
    }

    @inline new:
        int_seq { 0 }


int_seq.
[next_int ::]= @.new
next_int .=> print=>


int_seq: (=>addr (=>i32))
    i :: 0
    foo: (=>i32)
        i += 1
        i ret
    foo ret

next_int :: int_seq=>
next_int=> print=>

// https://doc.rust-lang.org/stable/rust-by-example/fn/closures.html

[outer_var]= :: 42

// function: (i32 i =>i32) // this is an err!
    // i + outer var

closure: (i32 i, @self =>i32)
    i + [self.capture]
    @struct {
        addr i32 capture
    }

closure_annotated :: closure { outer_var }
1 closure_annotated=> print=>   // prints 43

closure_annotated :: { outer_var }
    : (i32 i, @self =>i3)
        i + [self.capture]
    @struct {
        addr i32 capture
    }

1 closure_annotated=> print=>


one ::
    : ()
        1
one=>


color :: "green" @str.new

print ::
    : (@self)   // it's not a new syntax btw.
        "`color`: "self.color print=>
        @struct {
            str color
        }
    { color }

print=>

// we don't have that borrow checker. the destructor is set on this scope already by the @str.new inline macro

[count]= :: 0  // capturing registers do not work!

inc ::
    : (@self)
        [self.count] + 1 =[]
        "`count`: "count print=>
        @struct {
            addr i32 count
        }
    { count }, @this

inc.x inc.y=>

movable~ :: 3 @std.heap.alloc

consume ::
    %: (addr i32~)   // some unnamed label
        std.heap.free=>    // which is a routine
    ~>movable, %    // you move the movable here, making movable unaccecible, and taking address of it by %

~>consume.x consume.y=> // once you called it, consume.x is moved.. so you cannot use it!

consume @invoke // or use a macro!

haystack :: { 1, 2, 3 } @std.arr.new

contains ::
    %: (i32 needle, std.arr.i32 haystack)
        needle, haystack @std.arr.contains
    haystack, %

1, contains @invoke
4, contains @invoke

// input params?

#alias FnOnce T addr (T), T
#alias Fn T addr (T, i32 =>i32), T

apply: (addr (T) f) {
    f=>
}

apply_to_3: (addr (T, i32 =>i32) f =>i32) {
    3 f=>
}

greeting :: "hello"
farewell~ :: "goodbye" @std.str.new

@inline closure.start: (@lable.code code, ... captures)
    %: (@self)
        code
        @struct {
            captures
        }
    %, { captures }

diary ::
    % :
        "I said "self.greeting print=>
        "!!!", self.farewell.push_str=>
        "Then I screamed "self.farewell"." print=>
        "Now I can sleep. zzzzz" print=>
    %, { self.greeting, self.farewell } @closure.start

diary ::
    %: (@self)
        "I said "self.greeting print=>
        "!!!", self.farewell.push_str=>
        "Then I screamed "self.farewell"." print=>
        "Now I can sleep. zzzzz" print=>
        @struct {
            slice c8 greeting, str~ farewell
        }
    %, { greeting, ~>farewell }

~>diary apply=>
// type of diary: { addr (slice c8, str~), { slice c8, str~ } }

double ::
    %: (i32 x =>i32)
        2 * x
    %, { }

double apply_to_3=>
// type of double: { addr (i32=>i32) }


call_me: (addr () f)
    f=>

function: ()
    "I'm a function" print=>

closure ::
    %: ()
        "I'm a closure" print=>
    %

call_me(function)
call_me(closure)



@inline make_closure:

create_fn: ()
    text :: "Fn" @std.str.new

    move ::
        %: (@self)
            "This is a: "text print=>
            @struct {
                std.str
            }
        %, { 0~>text }

// hof?

@inline is_odd: u32 n =>bool
    n..2, 1 @modeq

acc :: 0
n :: 0
@loop
    ~ n += 1
    n * n
    ? upper >= <-
    @is_odd ? true->
        +=acc

u32 sum_of_squared_odd_numbers ::
    acc :: 0
    n :: 0..inf @range
        n * n
        ? upper >= <-
        @is_odd ? true
        +=acc

// diverging functions

foo: ()
    "this call never returns" print=> panic

->foo   // don't use =>(branch with link) if you do not want to return!

some_fn: ()
    { }

a :: some_fn=>
"This function returns and you can see this line." print=>

x :: "This call never returns." print=> panic
"You will never see this line!" print=>

sum_odd_numbers: (u32 up_to =>u32)
    acc :: 0
    i :: 0..up_to @range
        u32 addition ::
            i..2, 1 @modeq
            eq->
                i <-
            : ->continue
        +=addition
    acc
