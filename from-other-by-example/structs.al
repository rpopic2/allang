// https://gobyexample.com/structs

#alias std.str
#alias std.print

@struct person {
    str name,
    i32 age
}

new_person: (str name =>addr @person)
    // [p]= :: { name: name } // don't do this!
    // 42 =[p.age]

    p :: { name: name } // you could do this
    42 = p.age
    p =[p]

    [p]= :: { name, 42 } // but why not
    // p ret   // p is going to be a dangling ptr when returning!! it is also a compile err!
    // 1. you can allocate it on the heap, or 2. make this @inline so it gets created on the caller's stack

person { "Bob", 20 } print.ln.fmt=> // going to print 'Bob 20'
person { name: "Bob", age: 20 } print.ln.fmt=>
person { name: "Fred" } print.ln.fmt=> // WARNING! age is going to be some garbage value!
[]= :: person { name: "Ann", age: 40 }; print.ln.fmt=> // will print some usiz address
"Jon" new_person=> print.ln.fmt=>

[s]= :: person { name: "Sean", age: 50 }
[s.name] print.ln=>

sp :: s
[sp.age] print.ln.fmt=>

51 =[sp.age]
[sp.age] print.ln.fmt=>

dog :: {
    str name: "Rex"
    b8 isGood: true
}
dog print.ln.fmt=>

// https://doc.rust-lang.org/stable/rust-by-example/custom_types/structs.html

person:
    @struct {
        name str,
        age u8
    }

@struct unit { }
@struct pair { i32, f32 }
@struct point {
    x f32,
    y f32
}

@struct rectangle {
    top_left @point
    bottom_right @point
}

@struct rectangle top_left @point, bottom_right @point

name: "Peter"
age :: 27
peter :: person { name, age }
peter print.fmt=>

point :: point { x: 5.2, y: 0.4 }
another_point :: point { x: 10.3, y: 0.2 }

"point coordinates: (", point.x, ", ", point.y, ")" print=>

bottom_right :: point { x: 10.3, another_point.y }
// or
bottom_right :: another_point with { x: 10.3 }

left_edge, top_edge :: point

rectangle :: rectangle {
    top_left: point { x: left_edge, y: top_edge }
    bottom_right: bottom_right
}

unit :: unit { }
pair :: pair { 1, 0.1 }
integer, decimal :: pair
"pair contains ", integer, " and ", decimal print=>

// https://zig.guide/language-basics/structs

// we have guarentee about the in-memory layout. it is in declared order.

@struct Vec3 3 i32 { }

my_vector :: Vec3 {
    x: 0, y: 100, z: 50
}

// we don't have struct defaults

Vec4:
    @struct 4 i32{ }
    @inline new: (@self =>Vec4)
        { 0, 0, 0, 0 } with @self

test "struct defaults"
    my_vector :: { x: 25, y: -50 } @Vec4.new

Stuff:
    @struct {
        i32, i32
    }
    // derefernecing is not done automatically
    swap: (addr |@this selfp|)
        self :: [selfp]
        tmp :: self.x
        self.y =self.x
        tmp =self.y
        self =[selfp]

test "automatic derefernce"
    thing :: Stuff { 10, 20 } =[]
    thing swap=>

// http://odin-lang.org/docs/overview/#structs

struct Vector2 {
    f32 x, f32 y
}

v :: Vector2 { 1, 2 }
v.x : 4
v.x print=>

v :: Vector2 { 1, 2 } =[]
p :: v
1335 =[p.x]
v print=>

struct Vector3 {
    f32 x, y, z
}

v :: Vector3
v :: Vector3 ||
v :: Vector3 { 1, 4, 9 }

v :: Vector3 { z: 1, y: 2 }

.alignof 4
struct {

}

.packed
struct {

}

// http://odin-lang.org/docs/overview/#subtype-polymorphism

foo: (Entity entity)
    entity print=>

bar: (addr Entity entity)
    [entity] print=>

struct frog {
    f32 ribbit_volume,
    @Entity entity,
}

frog :: Frog {}
frog.x = 123
frog.entity foo=>

frog2 :: Frog {} =[]
123 =[frog.x]
frog2.entity foo=>


