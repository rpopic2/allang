geometry:
    // @inline perim: (@type T =>@type) (T =>f64)
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

@inline measure: (@geometry g)
    g print=>
    g g.area=> print=>
    g g.perim=> print=>

detect_circle: (@geometry g)
    g.area == circle.area ->    //type cheking is done by checking if address of two functions are same or not. you can also put a type variable in vtable.
        @circle c :: &g
        "circle with radius", c.radius =>println


r :: { width: 3, height: 4 } @rect.new
c :: { radius: 5 } @circle.new

r measure=>
c measure=>

r detect_circle=>
c detect_circle=>


