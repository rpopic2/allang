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

