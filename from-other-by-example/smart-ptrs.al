// trying to emulate std::unique_ptr

unique_ptr:
    @struct T {
        addr T~ ptr
        addr (addr) dtor
    }

    @inline from (@unique_ptr T self =>@self)
        self
        self.dtor ? null-> "dtor was null" print=> panic!
        ~ self.ptr self.dtor=>

    @inline make_unique (@type T =>@self)
        #sizeof T std.heap.alloc=> %
        ~ % std.heap.free=>
        %, std.heap.free

u :: i32 @make_unique
4 =[u.ptr]
[u.ptr] print=>

// it might need some synchronization but not now...
shared_ptr:
    @struct T {
        |addr| T ptr,
        |addr ctrlblk| _ctrl,
    }

    _ctrlblk:
        @struct ~ {
            |addr T~| ptr,
            i32 cnt,
        }
        rm_ref: (addr @self)
            [self.cnt] - 1
            ? 0 -> self std.heap.free=>
            : =[]


    @inline make_shared: (@type T =>shared_ptr T)
        ptr :: #sizeof T std.heap.alloc=>
        ctrlblk :: #sizeof _ctrlblk std.heap.alloc=>
        ctrlblk leak
        { ptr~, 1 } =[ctrlblk]
        ~ self._ctrl _ctrlblk.rm_ref=>
        { ptr, ctrlblk }

    @inline share: (@self T =>shared_ptr T)
        [self._ctrl].cnt + 1 =[]
        ~ self._ctrl _ctrlblk.rm_ref=>
        { self }

sp :: i32 @shared_ptr.make_shared
4 =[shared_ptr.ptr]

[sp._ctrlblk].cnt print=> // 1
:
    sp2 :: sp @shared_ptr.share
    [sp2] print=> // 4!
    [sp._ctrlblk].cnt print=> // 2

[sp._ctrlblk].cnt print=> // 1




