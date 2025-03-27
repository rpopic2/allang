// https://doc.rust-lang.org/stable/rust-by-example/scope/borrow/mut.html
// # mutability

Book:
    @struct {
        span c8 author
        span c8 title
        u32 year
    }

borrow_book: (|addr Book| book)
    "I immutably borrowed "book.title " - "book.year" edition" print=>

new_edition: (addr Book book)
    2014 =[book.year]
    "I mutably borrowed "book.title " - "book.year" edition" print=>

|immutabook| :: Book {
        author: "Douglas Hofstadter",
        title: "Gödel, Escher, Bach",
        year: 1979,
    } =[]

mutabook :: immutabook

immutabook borrow_book=>
mutabook borrow_book=>

immutabook new_edition=> // cannot call it because it is not const!
mutabook new_edition=>
