// multiple inheritance?
// i know this example is stupid but...
// from "Multiple Inheritance for C++ (1989)" by Bjarne Stroustrup

// wip.

clickable:
    struct {
        addr vtable vtable
    }

    struct vtable {
        addr () click
    }

makes_sound:
    struct {
        addr vtable vtable
    }

    struct vtable {
        vtable_entry make_sound
    }

    struct vtable_entry {
        addr () routine, i32 delta
    }

    play_default_sound: ()
        "default_click.wav" audio.play=>

button:
    @struct _self { str audio.name }
    @struct @clickable @makes_sound @_self

    vtable: clickable.vtable { click }
    vtable2: make_sound.vtable {
        make_sound: { make_sound, #offsetof button.button }
    }

    @inline new (@_self =>@button)
        { vtable, vtable2 }

    click: ()
        "Hello World!" printf=>

    make_sound: (@button self)
        [self.audio.name] audio.play=>

b :: @button.new    // { clickable.vtable: , makes_sound.vtable: }
[c]= :: @button.new    // { clickable.vtable: , makes_sound.vtable: }

b click=>
// { b.makes_sound, b.button } sound=>

c click=>
c + #offsetof button.makes_sound sound=>

click: (@clickable c)
    [c.vtable].click]=>

sound: (addr @makes_sound m)
    entry :: [m.vtable].make_sound]
    m + entry.delta [entry.routine]=>

