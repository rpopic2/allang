// https://doc.rust-lang.org/stable/rust-by-example/scope/raii.html

// it's quite different from rust
create_box: ()
    // first, the box should be suffexed with ~, indicating ownership. since std.heap.alloc waives ownership, you must suffix with ~
    // it comes from destructor(~) symbol, so it means that it has to be destructed!
    box1~ :: 3'i32 @std.heap.alloc
    
    // box1 is not destoyed at the end of the scope, you have to free it yourself, however..!
    // if you do not free it or grant ownership to others, it'a compile error!!
    
    // you give ownership to the routine argument, as std.heap.free requires the ownership to be given to it.
    // ~> notation MOVES ownership
    box1~> std.heap.free=>
    
    // after the move, you cannot access box1!! it throws compile err. even you don't catch the error, the box is pointing null at that point already.
    
#alias std.heap 
    
box2~ :: 5'i32 @heap.alloc

:
    box3~ :: 4'i32 @heap.alloc
    // while box3 does not freed when it goes out of scope, but you can tie the lietime of it by:
    ~ box3~> heap.free=>
    // note that ~ is a defer keword, which defers the execution of following to the exit of the scope
    
    box4~ :: 4 heap.alloc=>
    box4~> leak // you can also leak it
    
i :: 0..1'000 @range
    create_box=>
    
// destructor

// there is no destructor!
// but you can tie it's lifetime to the current stack by using @inline macro

ToDrop:
    @struct {
        addr~ ptr
    }
    
    @inline new: (=>@self)
          p :: 4 @heap.alloc
          ~ p~> heap.free=>
          { p }
          ~ 

@ToDrop.new
// this will just paste the code here, which ties lifetime of it to this scope 

// ownership and moves

destroy_box: (addr~ i32 c)
    heap.free=>

x :: 5'u32 =[]  // load 5 and store it(=[]) on to the stack
                // now x is the address pointing to the object on the stack!

y :: [x]    // obviously copies it. loads and stores to y. it is 

"x is "[x]", and y is "y print=>

a~ :: 5'i32 @heap.alloc  // integer 5, heap allocated. an owning ptr.

"a conatins: "[a]

b :: a  // this just copies it!

b~ :: a~>   // now it moves it, and b owns the ptr!

// "a contains: "[a] // compile err! you cannot use a anymore

destroy_box(b~>)    // now b is moved into the fn!

// mutability

|immutable_box|~ :: 5'u32 @heap.alloc

"immutable_box contains "[immutable_box] print=>
// 4 =[immutable_box] compile err!!

mutable_box~ :: immutable_box~> // move the ptr!

"mutable_box continas "[mutable_box] print=>

4 =[mutable_box]

"mutable_box now continas "[mutable_box] print=>

#alias std.str

Person:
    @struct {
        str~ name
        addr~ u8 age
    }


name~ :: "Alice" @str.new
age~ :: 20 @heap.alloc
person :: Person {
    name~>, age~>
}

// you simply cannot just reference age like rust example did it in allang!
Person { name~, age~ } :: person~>

// to do partial move..

Person { name~, age } :: person.name~>, person.age

"The person's age is "[age] print=>

"The person's name is "name print=>

// "The person struct is "person print=> // err! person.age is moved!

// `person` cannot be used but `person.age` can be used as it is not moved
println!("The person's age from person struct is {}", person.age);

// borrowing

eat_box_i32: (addr~ i32 boxed_i32)
    "Destroying the box that conatins "[boxed_i32] print=>
    boxed_i32 @heap.free

borrow_i32: (addr i32 borrowed_i32)
    "This int is "borrowed_i32 print->

boxed_i32~ :: 5'i32 @heap.alloc
stacked_i32 :: 6'i32 =[]

boxed_i32 borrow_i32=>
stacked_i32 borrow_i32=>

:
    copy_to_i32 :: boxed_i32 // just copies the ptr.
    alias_to_i32 :: &boxed_i32 // this just creates alias. you just call it in other name, nothiong hapens to the machine!

    boxed_i32~> eat_box_i32=>
    // now ref_to_i32 becomes a dangling ptr..!
    // this is where allang differs from rust. you can move ownership to it.
    // but it also invalidates the copies! so..

    // you cannot use copy_to_i32 because it is a dangling ptr.
    // copy_to_i32 borrow_i32=>

// boxed_i32~> eat_box_i32=> // this simply will not work.
// stacked_i32 eat_box_i32=>   // does not work! it requires an owned ptr!

// #aliasing

// unlike rust, it does not check for mut/immut borrows

@struct Point { i32 3 }

point :: Point { x: 0, y: 0, z: 0 } =[]

|borrowed_point| :: point
|another_point| :: point

"Point has coordinates: "borrowed_point.x", "another_borrow.y", "point.z print=>

"borrowed_point: "borrowed_point

// this works in allang.
mutable_borrow :: point

"Point has coordinates: "borrowed_point.x", "another_borrow.y", "point.z print=>

mutable_borrow :: point

{ 5, 2, 1 } =[mutable_borrow]

// this is also allowed
y :: point.y

"Point has coordinates: "borrowed_point.x", "another_borrow.y", "point.z print=>

@struct Point { i32 2 }

c :: 'Q' =[]
ref_c1 :: c // we don't have alternative syntax. it's just copying ptrs
ref_c2 :: c

point :: Point { 0, 0 } =[]

copy_of_x ::
    // x, _ :: point // this doesn't work!
    // i know this is not the point but just do..
    [point.x]

mutable_point :: point

:
    mut_ref_to_y :: mutable_point.y
    1 =[mut_ref_to_y]

5'u32 @heap.alloc %~
mutable_tuple :: %~>, 3'u32

:
    last :: mutable_tuple.y
    2'u32 =[last]

"tuple is "mutable_tuple =>

mutable_tuple.x~> heap.free=>

// https://zig.guide/language-basics/defer

test defer:
    |i16 x| :: 5
    :
        ~ 2 +=x
        x is 5 @expect
    x is 7 @expect

test multi_defer:
    |f32 x| :: 5
    :
        ~ 2 +=x // done in reverse order
        ~ 2 @/=x
    x is 4.5 @expect

// https://zig.guide/language-basics/labelled-blocks

count ::
    |sum| :: 0'u32
    |i| :: 0'u32
    i < 10, i += 1 @zig.while
        i +=sum
    sum

    tmp :: i
    i += 1
    tmp

// https://odin-lang.org/docs/overview/#defer-statement

x :: 123
~ x print=>

:
    ~ x := 4
    x := 2

x print=>

x := 234

:
    ~ foo=>; bar=>
    ~ is cond -> bar=>

// wierd stuff with defer and loop

owning_ptr~ :: i32 4 std.heap.alloc=>
~ owning_ptr~> std.heap.free=>

foo=> is > 5
    0 ret   // is this a leak?

err?, f~ :: "my_file.txt" os.open=>
err? isnt os.ERROR_NONE ->
    // handel err
~ f~> os.close


