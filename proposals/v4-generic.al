use linked_list
linked_list: { T: type, head: addr node }
.node { T: type, next: addr node, T: value }

.new: (T: type, self: addr linked_list)
    node_t: T #linked_list
    head: sizeof node_t =>calloc
    =[self, linked_list.head]

.last: (self: addr linked_list => addr node)
    head: addr node [self.head]

    next: addr node
    :: loop
        next: [head.next]
        [next.next] ? 0 ->break
        head: next
    next


add: (T: #type, self: addr linked_list, value: T)
    type, self =>last
    node_t: T #node
    new_node: sizeof node_t =>calloc
    =[last.next]
    value =[new_node.value]

main:
    i32, $ls: #linked_list.new
    i32, ls, 3 =>add


main: (argc: i32, argv: addr addr char => i32)


// zig style generic

mod optional
optional: { T: type, has_value: bool value: T }

none: (T: type =>T #optional)
    false, 0

some: (T: type, value: T =>T #optional)
    true, value

builtin type: { offset: isize, size: isize }

main:
    result: =>test

test: (=>i32 #optional)
    i: i32
    :: i ? 10 lt
        i32 #optional.none
    ::
        i32, i #optional.some

end mod

// c# style generic

mod optional
optional<T>: { has_value: bool value: T }

none<T>: (=>optional T)
    false, 0

some<T>: (value: T =>optional T)
    true, value

builtin type: { offset: isize, size: isize }

main:
    result: =>test

test: (=> optional<i32>)
    i: i32
    :: i ? 10 lt
        #optional.none<i32>
    ::
        i #optional.some<i32>

end mod

// zig style generic with comptime operator

mod optional
optional: { #T: type, has_value: bool value: T }

none: (#T: type =>T #optional)
    false, 0

some: (#T: type, value: T =>T #optional)
    true, value

builtin type: { offset: isize, size: isize }

main:
    result: =>test

test: (=>i32 #optional)
    i: i32
    :: i ? 10 lt
        i32 #optional.none
    ::
        i32, i #optional.some

end mod


comptime_add: (a: i32, b: i32)
    `a + b`

main:
    3, 4 #comptime_add

    3, 4 #>comptime_add

    `3 + 4`


generic_adder: { `T: type`, a: T, b: T }

generic_add: (`T: type`, a: T, b: T => T)
    a + b + `sizeof T`

generic_add_i32: (a: i32, b: i32 => i32)
    a + b + sizeof i32

comptime_test: (T: type)
    generic_add_T: (a: T, b: T => T)
        a + b + sizeof T

i32, #comptime_test
    //
    generic_add_i32 (a: i32, b: i32 => i32)
        a + b + sizeof i32

##generic_add_i32

main:
    g: #generic_adder
    //
    g: { `T: type`, a: T, b: T }
    //

    i32, 3, 4 #generic_add
    //
    a + b + sizeof i32


