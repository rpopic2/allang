// union itself might not be too useful. see also enum-union.al

#union WebEvent {
    c8 KeyPress,
    str Paste,
    { x: f64, y: f64 } Click,
}

e :: WebEvent { 'c' }
e :: WebEvent { Paste: "hello" }
e :: WebEvent { { 2.3, 3.4 } }

'd' >e.KeyPress
{ 123.0, 345.2 } >e
"bye!" >e.Paste

e.Paste print=>

#union Result {
    i64 int,
    f64 float,
    bool bool,
};

test "simple union"
    result :: Result { int: 1234 }
    12.34 =result.float // this works in al
}
