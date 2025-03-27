// print

main:
    "hello, world\n"
    ->print


// print2

main:
    "hello" \ print
    ", world" \ print
    "\n" \ print

// ctof

main:
    lower: 0
    upper: 300
    step: 20

    fahr: [lower]

loop:
    fahr upper
    > break
    celcius: 5 * (fahr - 32) / 9

    (fahr '\t' celcius 'n') \ tostr \ print

    "%\t%\n"
    fahr \ tostr \ print
    '\t' print
    celcius \ tostr \ print
    '\n' print

    fahr + step =>[fahr]

// ver 2
main: ->
    lower: 0
    upper: 300
    step: 20

    fahr: [lower]

.loop:
    cmp [fahr], [upper]
    :: le
        ->break

    celcius: 5 * ([fahr] - 32) / 9
    "%d\t%d\n", [fahr], [celcius] =>printf
    [fahr] + [step] = [fahr]
    ->loop
.break:
    ret

// ver 3
main: =>
    $lower: 0
    $upper: 300
    $step: 20

    $fahr: lower

.loop:
    cmp fahr, upper
    :: le ->break

    celcius: 5 * (fahr - 32) / 9
    "%d\t%d\n", fahr, celcius =>printf
    fahr + step >fahr
    ->loop
.break:

    ret

// 1.3 the for statement
main: =>
    fahr$ i32

    fahr = 0
.for:
    fahr, 300 gt ->break
    defer fahr += 20

    ["%3d %6.1f\n"], fahr, (5.0/9.0)*(fahr-32) ->printf
    ->for
.break:
    ret


// canonical for statement

for (int i = 0; i < len; ++i) {

}

main: =>

    i: 0
.for:
    i >= len ->break
    ~ ++i
    ~ ->for
.break:

    q: struct queue
.while:
    queue.empty, true eq ->break
.break:

main: =>
    i: 0
.for:
    i >= len ->.break
    ~ ++i
    j: 0
    ..for:
        j >= len ->..break
        ~ ++j
        ->..for
    ..break:
    ->.for
.break:

main: ->
    arr: 0x10 i32

    ptr$ arr
    end$ arr + arr.length

.loop: (ptr != end)
    [ptr]++ ->fromint ->print
    ->loop

    i: 0
.for: (i < arr.length)
    ~ ++i
    [ptr, i] ->fromint ->print
    ->for
::
    ret
