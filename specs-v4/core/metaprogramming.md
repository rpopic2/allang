# inline macro

// inline macros are type checked

@inline add: (i32 i, i32 j)
    +

1, 2 @add   // this expands to 1 + 2

