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

