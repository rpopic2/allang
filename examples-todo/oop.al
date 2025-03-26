// another fucking example of polymorphism

[c]= :: @cat.new
[d]= :: @cat.new

@cat c :[] @cat.new

@cat c :: { cat.make_sound, 0 }

make_it_sound: (addr @animal a)
    a [a.make_sound]=>

c make_it_sound=>   // meow!
d make_it_sound=>   // wuff!

animal:
    #alias make_sound_t = (@animal)
    @struct { addr #make_sound_t make_sound, i32 _hunger }

    // you'll have to maintain your own vtable to implement interfaces/inheritance!
    // defining alias is not mandatory, but this way you can change signagure at once across vtable methods (or 'methods of derived classes' if you prefer, there is no such thing as 'class' in al)


    // `i32 _hunger` sorry, but no encapsulation!
    // but you can distinguish whether it is interface or not by leading underscore

// I don't know why you want to do inheretance, but you could still do it,
dog:
    @struct @animal { i32 barked }

    @macro new: ()
        animal { dog.make_sound, 0 } dog { 0 }

    make_sound: animal.make_sound_t
        "wuff!" =>std.io.print
        a._hunger += 20
        barked += 1
        ret

cat:
    @struct @animal

    @macro new: ()
        cat { cat.make_sound }

    make_sound: animal.make_sound_t
        "meow!" =>std.io.print
        a._hunger += 10
        ret


// another stupid example, parent as pointer and type checking

c: @circle.new =[]
~ [c] @circle.delete

addr shape s :: c
[s.get_area]=> std.str.from.#=> std.io.print=>

s.type == #typeof circle
    "circle!" std.io.print=>>
s.type == #typeof square
    "square!" std.io.print=>>
::

shape:
    #alias get_area_t = (addr @shape s => f32)
    @struct {
        addr #get_area_t get_area,
        #type type
    }

    @inline _new (@type T, type shape_type, #get_area_t get_area =>T)
        super :: sizeof shape std.heap.alloc=>
        ? null ->panic
        shape {
            get_area, shape_type
        }
        =[super]
        T { super }

circle:
    @struct {
        addr @shape super,
        f32 diameter
    }

    @inline new (f32 diameter => @circle~)
        self :: @circle, #typeof circle, get_area @shape._new
        self.diameter :: diameter;

    @inline delete (@circle self)
        self.super std.heap.free=>

    get_area: #shape.get_area_t
        s.diameter * 3f ret

square:
    @struct {
        addr @shape super,
        f32 side
    }

    @inline new (f32 side =>@square~) {
        self :: @square, #typeof square, get_area @shape._new
        self.side :: side;
    }

    @inline delete (@circle self)
        self.super std.heap.free=>

    get_area: #shape.get_area_t
        s.side * s.side ret


get_len: (@std.array arr => i32)
    arr.len ret
// it works because at the end all compiler cares about is...
// get_len: (addr, i32 => i32)


@alias std.str = str
s: "Hi Mom!" @@str.from =[]
=>get_len

// another way of implementing vtable

base:
    @struct {
        addr vtable
    }
    @inline new: ()
        { vtable }

    @struct vtable {
        addr () foo,
        addr () bar,
    }

    vtable: vtable { foo: foo, bar: bar }

    foo: ()
        "base foo" print=>
    bar: ()
        "base bar" print=>

derived:
    @struct @base { }

    @inline new: ()
        base { vtable }

    vtable: { foo: foo, bar: bar }

    foo: ()
        "derived foo" print=>
    bar: ()
        "derived bar" print=>

derived.2:
    @struct @base { }
    @inline new: ()
        { vtable }

    // you could do these kind of crazy stuffs
    vtable: { foo: derived.foo, bar: base.bar }

b :: @base.new
d :: @derived.new
d2 :: @dreived2.new

[b.vtable].bar]=> // 'base bar'

b make_it_foo=> // 'base foo'
d make_it_foo=> // 'derived foo'
d2 make_it_foo=>    // 'derived foo'
[d2.vtable].bar]=>  // 'base bar'

make_it_foo: (@base self)
    [self.vtable].foo]=>

