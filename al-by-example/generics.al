// https://gobyexample.com/generics

#alias std.io.print

// generic is basically same stuff as @inline, but it generates the code only once.
@generic list.#T: (@type T) // #T is for literal text replacement for a type
    @struct ~ {
        addr element head~,
        addr element tail
    }

    delete.2: (@list.#T self)
        addr element ptr :: self.head
        @loop
            [ptr.next] ~>ptr
            ptr null<-
            ptr std.heap.free=>
        self.head~ std.heap.free=>

    element:
        @struct {
            addr element next~,
            @T val
        }
        @inline new: (@element self =>addr @element~)
            #sizeof element std.heap.alloc=> %
            self =[%]
            % ret
        @inline delete: (addr @element self)
            std.heap.free=> ret

    push: (addr @list.#T self, @T v)
        [self.tail] null->
            { null, v } @element.new
            ~>[self.head]
            [self.head] =[self.tail]
            ret
        ->
            { head: null, val: v } @element.new
            ~>[self.tail.next]
            [self.tail.next] =[self.tail]
        ret

    all_elements: (addr @list.#T self =>std.arr.#t~)
        std.arr.#T.
        elems :: .new=>
        e :: self.head; e != null; [e.next] = e @for
            [e.val] elems .append=>
        elems ret


list.i32.
list.i32 lst ::
~ lst .delete=>
10, lst .push=>
13, lst .push=>
23, lst .push=>
elems :: lst .all_elements=>
~ elems #.free=>
elems print.arr=>

i32 @list.#T    // you can expand it here explicitly if you need to specify where to expand it. it would be a compile error if there is multiple place where it expands, as it is a redefinition!

// https://doc.rust-lang.org/stable/rust-by-example/generics.html

@struct A

@struct Single @A // just appended struct

@generic @struct SingleGen.#T (@type T)

s :: Single { A }
SingleGen.c8 char :: SigleGen { 'a' }
t :: SingleGen { A }
i :: SingleGen { 6 }
c :: SingleGen { 'a' }

reg_fn :: (Single s)

gen_spec_t: (SingleGen.#A s)

gen_spec_i32: (SingleGen.i32 s)

@generic generic.(@type T)
    generic.#T: (SingleGen.#T)


S { A } reg_fn=>
Sgen { A } gen_spec_t=>
Sgen { 6 } gen_spec_i32=>

Sgen { 'a' } generic.c8=>
Sgen { 'a' } generic.#=>

Val:
    @struct {
        f64 val
    }

    value(addr @self =>addr f64)
        self.val

@generic GenVal.(@type T):
    @struct {
        T gen_val
    }

    value(addr @self =>addr T)
        self.val

x :: Val { val: 3.0 }
x :: GenVal.i32 { gen_val: 3'i32 }

