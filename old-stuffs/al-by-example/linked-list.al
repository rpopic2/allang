// code from https://gobyexample.com/generics

#alias std.print
#alias std.heap

@generic list.(@type T)
    @struct {
        addr?~ element head,
        addr? element tail
    }

    @struct element {
        addr?~ element next,
        @T val
    }

    @inline new: (=> @self~)
        @self { }

    delete: (addr @self~ self)
        p :: [self.head?~>]

        @loop
            p isnt pointing -> ret
            np :: [p.next?~>]
            p~> heap.free=>
            (np~>)p

        self~> _


    push: (addr @self self, T v)
        [self.tail?] isnt pointing
            { nopoint, v } @element.new~
            ~> =[self.head]
            [self.head] =[self.tail]
            ret
        :
            tail :: [self.tail]
            { next: nopoint, val: v } @element.new $~
            $~> =[tail.next]
            [tail.next] =[self.tail]
        ret

    all_elements: (addr @self self =>std.list.T~)
        elems~ :: std.list.T.new=>
        e :: self.head; e isnt null; e([e.next]) @for
            [e.val], elems .add=>
        elems~> ret

    element:
        @inline new: (@self =>addr~ element)
            #sizeof element std.heap.alloc=> $~
            self =[$]
            $~> ret

        @inline delete: (addr~ @element self)
            ~> std.heap.free=>


[lst] :: @list.i32.new~>
~ lst~> .delete=>
lst, 10 .push=>
lst, 13 .push=>
lst, 23 .push=>
// generates compile err if you dont delete it. lst holds some owned address

elems :: lst .all_elements=> ~>
~ elems~> .free=>
elems print=>

@list.i32    // you can expand it here explicitly if you need to specify where to expand it. it would be a compile error if there is multiple place where it expands, as it is a redefinition!




// https://doc.rust-lang.org/stable/rust-by-example/custom_types/enum/testcase_linked_list.html

// this time, the list is stack allocated and will be destroyed when it goes out of scope!

List:
    @enum union {
        { u32 val, addr List next } Cons,
        Nil,
    }
    @inline new: (=>List)
        { Nil }

    @inline prepend: (u32 elem, @self self =>@self)
        ptr :: self @std.heap.alloc ~>
        ~ ptr ~> std.heap.free=> // now lifetime tied to the scope where it expands this
        { Cons { elem, ptr } }

    len: (addr @self =>u32)
        e, v :: [self]
        is Cons ->
            v.next .len=> + 1 ret
        : -> 0 ret

    stringify: (addr @self =>str~)
        e, v :: [self]
        is Cons ->
            head, tail :: v
            tail .stringify=> %
            head`, `%
        : ->
            "Nil"
        std.fmt=> ~>

List.
[list]= :: @List.new
1, [list] @.prepend=> =[list]
2, [list] @.prepend=> =[list]
3, [list] @.prepend=> =[list]
list .len=> %
`linked list has length: `% print=>
list .stringfy=> print=>


// zig's linked list https://ziglang.org/documentation/master/#struct

@generic linked_list: #type T => #type
    liked_list.T:
        struct {
            First addr? node
            Last slice? node
            Len usize
        }

        struct node {
            Prev slice? node
            Next addr? node
            Data T
        }

        push: Self addr self, Node addr~ node =>
        #leaf
            [Self.Last] ?
                [Node.Prev] = null
                [Node.Next] = null
                [Self.First] = Node~>
                [Self.Last] = Self.Last
                ret
            [Node.Next] = null
            [Self.Last] = Node.Next~>

        pop: Self addr self => addr? node
        #leaf
            Last :: [Self.Last] ? eret
            Prev :: Last.Prev
            Prev ->
                [Prev.Next] = null
            [Self.Last] = Prev

            ret Last

linked_list i32 @

#literal view? addr? :: .0 null

List :: linked_list.i32{
    .First null
    .Last null
    .Len 0
} =[]

Node :: linked_list.i32.node{
    .Prev null
    .Next null
    .Data 1234
} =[]

List2 :: linked_list.i32{
    .First ~>Node    // Node is invalidated because it is moved
    .Last This.First
    .Len 1
} =[]

Node2 :: linked_list.i32.node{
    .Prev [List2.Last]
    .Next null
}

1234 is [List2.First].Data -> eret

// addr cannot be copied. (can it be guaranteed to be not aliasing?)
