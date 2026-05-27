# should be make_struct handled by backend?

when handled by backend:
* we need to write make_struct for each architecture.

pros:
* we can handle if the architecture has different memory layout.. -> but it dosn't

i think it is better to be handled from frontend if possible.


# how should be branched returns handled? how should be return statement compiled to?
in traditional languages, return statemnet dosn't exactly match to ret instruction because of epilogues.
it could mean few things:
* ret instruction if there isn't epilogue.
* branch to epilogue
* has copied epilogue

we want to optimise this...

case 1:
```asm
cmp x0, x1
b.gt skip
mov w0, 0
b epilogue
skip:
; do sth..
```

can be turned into one branch
case 2:
```asm
cmp x0, x1
mov w0, 0
b.le epilogue
; do sth..
```

this optimization cannot be used if we should not destroy x0.

it may be better to have two rets if there is no epilogue:
case 3:
```asm
cmp x0, x1
b.le skip
mov w0 0
ret
skip:
; do sth
ret
```

i think that case 1 should be the default case.


# from where should we be traking registers? how to handle 16-bit register?
# how to deal with msvc x64 calling convention?
# how to deal with x86_64 having only half register than aarch64?
we cannot have named registers as much as we do in aarch64.
callee-saved registers in..
aarch64: 19~27 (8 regs)
x86_64: 7 regs in windows, 5 regs in linux. si, di is used as scratch in linux.. for function args.
-> let's have our own calling convention. use other only if is exported.

allow 7 named registers.

if you look from the frontend.. 16-bit registers should take up two registers even from the frontend.


backend should care about hardware-specific optimizations, while frontent should care about generic optimizations. if anything is generic and common for aarch64 and x86_64, and macos/linux/windows, it should be in the frontend.
