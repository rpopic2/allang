// https://gobyexample.com/enums

#alias println std.print.ln

ServerState:
    #enum i32 {
        StateIdle, ServerState,
        StateConnected,
        StateError,
        StateRetrying
    }
    #ServerState @std.enum.to.str

transition: (#ServerState s)
    ? #StateIdle -> #StateConnected ret
    ? #StateConnected or #StateRetrying -> #StateIdle ret
    ? #StateError -> #StateError ret
    ?
        s ServerState.to.str=>
        "unknown state", % std.print.err=> panic

// enums just gets replaced with literal numbers on compile time.
ns :: #StateIdle transition=>
ns println=>

ns2 :: ns transition=>
ns2 println=>

// @inline std.enum.to.str(@enum e)
//     e.to_str

// https://doc.rust-lang.org/stable/rust-by-example/custom_types/enum.html
// also see enum-union.al

#enum Stage {
    Beginner,
    Advanced
}

#enum Role {
    Student,
    Teacher
}

// this will allow you to use with the short name...
Stage.
Role.
stage :: .Beginner
role :: .Student

// stage :: .Advanced // until there is a empty line! now this is a compile error.

// you can also keep it alive across a label!
Stage.
_: {
    #.Beginner: "Beginners are starting their learning journey!",
    #.Advanced: "Advanced learners are mastering their subjects..."
}[, stage] print=>

Role.
_: {
    #.Student: "Students are acquiring knowledge!",
    #.Teacher: "Teachers are spreading knowledge!"
}[, role] print=>

#enum Number {
    Zero, One, Two,
}

#enum Color {
    Red: 0xff0000, Green: 0x00ff00, Blue: 0x0000ff
}

"zero is ", #Number.Zero as i32 print=>
"one is ", #Number.One as i32 print=>

#Color.Red &i32, "06", to.hex=> %
"roses are ", % print=>
#Color.Red &i32, "06", to.hex=> %
"violets are ", % print=>

// https://zig.guide/language-basics/enums

#enum Direction {
    north, south, east, west
}

#enum u8 Value {
    zero, one, two
}

expect Value.zero as 0 is 0


#enum u32 Value2 {
    hundred: 100,
    thousand: 1000,
    million: 1000000,
    next,
}

Suit:
    #enum {
        clubs, spades, diamonds, hearts
    }

    isClubs: (@self =>bool)
        is clubs

test "enum method"
    expect Suit.spades isClubs=> is Suit.clubs

Mode:
    #enum {
        on, off
    }
    count :: 0'u32

test "hmm"
    Mode.count += 1
    expect Mode.count is 1
