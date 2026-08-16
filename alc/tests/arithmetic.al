#compile_all expect.al

same_sign_widen =>
signed_dominant_add =>
sub_narrow_rhs =>
sub_narrow_lhs =>
signed_extend_negative =>
cmp_narrow_rhs =>
cmp_narrow_lhs =>

ret 0

same_sign_widen: =>
    A :: u32{5}
    B :: u64{10}
    Sum :: A + B
    expect Sum is 15 =>

signed_dominant_add: =>
    A :: i64{100}
    B :: u32{7}
    Sum :: A + B
    expect Sum is 107 =>

sub_narrow_rhs: =>
    A :: u64{50}
    B :: u32{20}
    Diff :: A - B
    expect Diff is 30 =>

sub_narrow_lhs: =>
    A :: u32{50}
    B :: u64{20}
    Diff :: A - B
    expect Diff is 30 =>

signed_extend_negative: =>
    A :: i64{100}
    Small :: i32{10}
    Big :: i32{30}
    B :: Small - Big
    Sum :: A + B
    expect Sum is 80 =>

cmp_narrow_rhs: =>
    A :: i64{100}
    B :: u32{50}
    A > B ->
        ret
    _Exit 81 =>

cmp_narrow_lhs: =>
    A :: u32{50}
    B :: i64{100}
    A < B ->
        ret
    _Exit 82 =>
