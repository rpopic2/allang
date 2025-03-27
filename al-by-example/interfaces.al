// https://gobyexample.com/interfaces

#alias print :: std.io.println

// SEE ALSO vtable.al

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
        @circle c :: &g
        "circle with radius", c.radius =>println


r :: { width: 3, height: 4 } @rect.new
c :: { radius: 5 } @circle.new

r measure=>
c measure=>

r detect_circle=>
c detect_circle=>

// https://doc.rust-lang.org/stable/rust-by-example/trait.html

Animal:
    @struct {
        addr (str =>any) new
        addr (any) name
        addr (any) noise
        addr (any) talk
    }

    fn talk(@self) {
        self.name=> %
        self.noise=> %2
        %"says "%2 print=>
    }

Sheep:
    @struct {
        b8 naked,
        str name,
    }

    @inline is_naked: (@self =>bool)
        self.naked

    shear: (addr @self)
        [self] @is_naked true->
            " is already naked..." % >>
        :
            true =[self.naked]
            " gets a haircut!" % >>
        << self.name=>, % print=>

Sheep: impl Animal
    @inline new: (str name =>Sheep)
        Sheep { name: name, naked: false }

    @inline name: (addr @self =>str)
        [self.name]

    noise: (addr @self =>str)
        [self] @is_naked
        "baaaaah?" : "baaaaah!"

    talk: (addr @self)
        self.noise=> %
        self.name" pauses briefly... "% print=>


dolly :: Sheep

// https://doc.rust-lang.org/book/ch10-02-traits.html

// not an actual al code, just trying to see if it fits in al!

Summary: Trait
    @struct {
        addr
    }
    addr (addr =>str) summarize

Summary:
    #alias summarize = (addr =>str)
    @interface summarize: (addr @self =>str)

NewsArticle:
    @struct {
        str headline, location, author, content
    }

NewsArticle: impl Summary
    summarize: (addr @self =>str)
        self :: [self]
        "{}, by {} ({})", self.headline, self.author, self.location fmt=>

Tweet:
    @struct {
        str username, content
    }
Tweet: impl Summary
    summarize: (addr @self =>str)
        self :: [self]
        "{}: {}", self.username, self.content fmt=>

Foo:
    @struct {
        i32 i
    }
Foo: impl Summary
    summarize: (addr @self =>str)
        "foo! "[self.i] fmt=>

whatever: (@Summary s) // whatever: (addr s)
    s summarize=> print=>

@inline whatever: (@Summary s) // whatever: (addr s)
    s summarize=> print=>

t []= :: Tweet { "alvin", "hello world!" }
t summarize=> print=>

f []= :: Foo { 2 }

t whatever=>
f whatever=>

// https://doc.rust-lang.org/stable/rust-by-example/generics/bounds.html

HasArea:
    area: (addr @self =>f64)

Rectangle:
    @struct {
        f64 length, height
    }
    area: (addr @Rectangle self =>f64)
        *

Triangle:
    @struct {
        f64 length, height
    }

area.(@type HasArea T)(addr T t =>f64)
    area=>

@inline area: (addr t =>f64)
    area=>

[r]= :: Rectangle { 3, 4 }
[t]= :: Triangle { 3, 4 }

r @area
t @area
r .area=>
t .area=>

slice T:
    @class {
        addr T data,
        usiz len,
    }
