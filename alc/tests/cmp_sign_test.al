#declare printf: Format addr u8 => Num_Printed i32
#declare _Exit: Status i32

// Signedness distinction: high-bit-set unsigned values must compare as
// large positive numbers, not as negatives. These cases only pass if the
// unsigned condition codes (HI/HS/LO/LS) are selected for unsigned dtypes.
u32_highbit_gt =>
u32_highbit_ge =>
u32_highbit_lt =>
u32_highbit_le =>
u64_highbit_gt =>

// Signed negatives must use the signed condition codes (GT/GE/LT/LE).
i32_neg_gt =>
i32_neg_ge =>
i32_neg_le =>
i64_neg_lt =>

// Comparison coverage for the integer widths index_test.al skips.
cmp_i8 =>
cmp_i16 =>
cmp_i64 =>
cmp_u8 =>
cmp_u16 =>
cmp_u64 =>

// is / isnt across widths and with negatives.
is_i32_neg =>
isnt_i32_neg =>
is_u8 =>

printf "all cmp sign tests passed\n" =>
ret 0

u32_highbit_gt: =>
    A :: u32{0x80000000}
    A > 1 ->
        ret
    _Exit 60 =>

u32_highbit_ge: =>
    A :: u32{0x80000000}
    A >= 1 ->
        ret
    _Exit 61 =>

u32_highbit_lt: =>
    A :: u32{0x80000000}
    A < 1 -> _Exit 62 =>
    ret

u32_highbit_le: =>
    A :: u32{0x80000000}
    A <= 1 -> _Exit 63 =>
    ret

u64_highbit_gt: =>
    A :: u64{0x8000000000000000}
    A > 1 ->
        ret
    _Exit 64 =>

i32_neg_gt: =>
    A :: i32{-1}
    A > 1 -> _Exit 65 =>
    ret

i32_neg_ge: =>
    A :: i32{-1}
    A >= 0 -> _Exit 66 =>
    ret

i32_neg_le: =>
    A :: i32{-5}
    A <= 0 ->
        ret
    _Exit 67 =>

i64_neg_lt: =>
    A :: i64{-1}
    A < 0 ->
        ret
    _Exit 68 =>

cmp_i8: =>
    A :: i8{3}
    B :: i8{5}
    A < B ->
        ret
    _Exit 70 =>

cmp_i16: =>
    A :: i16{5}
    B :: i16{3}
    A > B ->
        ret
    _Exit 71 =>

cmp_i64: =>
    A :: i64{3}
    B :: i64{5}
    A <= B ->
        ret
    _Exit 72 =>

cmp_u8: =>
    A :: u8{200}
    B :: u8{100}
    A > B ->
        ret
    _Exit 73 =>

cmp_u16: =>
    A :: u16{100}
    B :: u16{200}
    A < B ->
        ret
    _Exit 74 =>

cmp_u64: =>
    A :: u64{5}
    B :: u64{3}
    A >= B ->
        ret
    _Exit 75 =>

is_i32_neg: =>
    A :: i32{-5}
    A is -5 ->
        ret
    _Exit 76 =>

isnt_i32_neg: =>
    A :: i32{-5}
    A isnt -4 ->
        ret
    _Exit 77 =>

is_u8: =>
    A :: u8{200}
    A is 200 ->
        ret
    _Exit 78 =>
