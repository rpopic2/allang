// https://gobyexample.com/interfaces

#alias print :: std.io.println

// also check out the alternative way using vtable in vtable.al
// sorry, but you will have to dynamic dispatch yourselves
// inheritance was done so easily compared to the cost of execution and human comprehension.
// although, it is not that hard to impl from scratch!

// this approach might be good if there is only one virtual function
// but since the size of each struct gets big, it might be better to use vtable

geometry:
    // @inline perim: (@type T =>@type) (T =>f64)// we'll show how to use this macro later.
    @struct {
        addr (@geometry =>f64) area,
        addr (@geometry =>f64) perim
    }

rect:
    @struct @geometry { // appends to the geometry struct.
        f64 width,
        f64 height,
    }

    @inline new: (@rect r)
        geometry { area, perim } r

    area: (@rect =>f64)
        [r.width] * [r.height] ret

    // perim: rect @geometry.perim // use the macro to change signatures all at once
    perim: (@rect =>f64)
        2 * [r.width], 2 * [r.height]
        + ret

circle:
    @struct @geometry { f64 radius }

    @inline new: (@circle r)
        geometry { area, perim } r

    area: circle @geometry.area
        [r.radius]
        std.math.pi * % * %

    perim: circle @geometry.perim
        2 * std.math.pi * [r.radius]

measure: (@geometry g)
    g print.fmt=>
    g g.area=> print=>
    g g.perim=> print=>

detect_circle: (@geometry g)
    g.area == circle.area ->    //type cheking is done by checking if address of two functions are same or not. you can also put a type variable in vtable.
        @circle c :: <g>
        "circle with radius", c.radius =>println


r :: { width: 3, height: 4 } @rect.new
c :: { radius: 5 } @circle.new

r measure=>
c measure=>

r detect_circle=>
c detect_circle=>


