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
