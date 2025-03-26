// https://gobyexample.com/interfaces

// alternative way of impementing polymorphism by vtable
// this time, the methods are using registers for self!

#alias println: std.print.ln

geometry:
    @struct {
        |addr vtable| vtable
    }

    @struct vtable {
        addr (@geometry =>f64) area,
        addr (@geometry =>f64) perim
    }

rect:
    @struct @geometry {
        f64 width, height
    }

    @inline new: (@rect.rect r =>@rect)
        geometry { vtable } r

    vtable: geometry.vtable { area, perim }

    area: (@rect self =>f64)
        self.width * self.height ret

    perim: (@rect self =>f64)
        2 * self.width, 2 * self.height
        + ret

circle:
    @struct @geometry {
        f64 radius
    }

    @inline new: (@rect self)
        self
        vtable >self.vtable

    vtable: geometry.vtable { area, perim }

    area: (@circle self =>f64)
        std.math.pi * self.radius * self.radius

    perim: (@circle self =>f64)
        2 * std.math.pi * self.radius

measure: (@geometry g) // it does not slice the derived classes. the arguments are still passed by registers.
    g println.va
    vt :: [g.vtable]
    args :: va_args
    va_args [vt.area]=>
    va_args [vt.perim]=>

detect_circle: (@geometry g)
    g.vtable == circle.vtable -> // type check by vtable address
        c :: g as @circle // it just treats as if it is a circle. no obj created
        "circle with radius" , c.radius println.va=>

r :: { width: 3, height: 4 } @rect.new
c :: { radius: 5 } @circle.new

r measure
c measure
r detect_circle
c detect_circle

