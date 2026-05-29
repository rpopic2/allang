// al is "NOT" an oop language. but you can still do it.

#alias std.print

c :: @cat.new =[]
d :: @dog.new =[]

c make_it_sound=>   // meow!
d make_it_sound=>   // wuff!
// d actually put struct of animal and bark...

make_it_sound: (addr @animal a)
    [a.make_sound]=>

animal:
    struct { addr (@animal=>) make_sound, i32 _hunger }

dog:
    struct @animal, { i32 barked }

    @inline new: (=>dog)
        dog { dog.make_sound, _hunger: 0, barked: 0 }

    make_sound: (addr @self self)
        [self._hunger] + 20 =[]
        [self.barked] + 1 =[]
        "wuff!" print=>

cat:
    struct @animal

    @inline new: (=>cat)
        cat { cat.make_sound }

    make_sound: (addr @self self)
        self._hunger += 10
        "meow!" print=>

// implementing oop with vtable

// https://gobyexample.com/interfaces

// alternative way of impementing polymorphism by vtable
// this time, the methods are using registers for self!

#alias println: std.print.ln

geometry:
    struct {
        |addr vtable| vtable
    }

    struct vtable {
        addr (@geometry =>f64) area,
        addr (@geometry =>f64) perim
    }

rect:
    struct
        @geometry,
        { f64 width, height }

    @inline new: (@rect.rect self =>@self)
        geometry { _vtable }, self

    vtable: geometry.vtable | area, perim |

    area: (#self self =>f64)
        self.width * self.height

    perim: (#self self =>f64)
        2 * self.width, 2 * self.height
        +

circle:
    struct
        @geometry,
        { f64 radius }

    @inline new: (@circle.circle self =>@self)
        geometry { _vtable }, self

    vtable: geometry.vtable | area, perim |

    area: (#self self =>f64)
        #std.math.pi * self.radius
        * self.radius

    perim: (#self self =>f64)
        2 * #std.math.pi
        * self.radius

measure: (@geometry g) // it does not slice the derived classes. the arguments are still passed by registers.
    ::
    g print=>
    g.vtable.area=>
    g.vtable.perim=>

detect_circle: (@geometry g)
    g.vtable is circle.vtable -> // type check by vtable address
        c :: &g as circle // reinterpretes g as circle. nothing happens here.
        `circle with radius `c.radius print=>

r :: { width: 3, height: 4 } @rect.new
c :: { radius: 5 } @circle.new

r measure=>
c measure=>
r detect_circle=>
c detect_circle=>

