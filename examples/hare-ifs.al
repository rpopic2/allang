
color: 
    #enum red, orange, yellow, green, blue, violet
    to_string: enum_to_string_lookup @


stock:
    struct {
        .Color u8 #color
        .Amount i32
    }

    print: Item @stock =>
        Item.
        ``([color.to_string * .Color])` paint\t`.Amount``(.Amount is 1 ? "" : "s") .print =>



Stock :: *@stock{
    {/red, 1}, {/blue, 6}, {/violet, 1}, {/orange, 4}
}
`Inventory:`Stock.0``Stock.1``Stock.2``Stock.3 .print n =>


