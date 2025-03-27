#enum error {
    Ok, ExpecetedArgument
}

(args: slice str =>i32)

args.len < 2
    error.ExpectedArgument ret

args..arg, i @foreach.index
    "arg "i": "arg" print=>

f ::
    args[1] @.at
    f32.from=>

c ::
    f - 32, 5.0 / 9.0
    *
c .to.str.decimal.1 println=>

