// https://www.haskellforall.com/2015/10/basic-haskell-examples.html?m=1

put_todo: (int n, str todo)
	``n`: `todo print=>

prompt: (list.str todos)
    =[]
	`\ncurrent todo list:` print=>
	i, t..[todos] @foreach.range; i, t put_todo=>
	command :: scan=>
	command, [todos] interpret=>

interpret: (str s, list.str todos)
	s. @starts_with.rest@@ "+ ", todo -> todos. todo add=> todos prompt->
	s. @starts_with.rest@@ "- ", num ->
		num :: num. to.i32=>
		is Ok -> todos. @delete_at@ num is Ok ->
			todos prompt->
		`no todo entries match..` print=>
		prompt->
	s is 'q' -> 0 std.exit->
	command :: &s
	`invalid command '`command`'` print=>
	todos ->prompt

delete: (i32 n, list.str as =>opt.list.str)
	n is 0 -> as. @delete_at@ 0 => as ret
	as. @delete_at@ n - 1
	as ret

`Commands:`
`+ <str> - Add...`
`- <int> - Del...`
`q - Quit...` print=>
List.i32.new=> ->prompt


@inline@ List.T.delete_at(@self, int index =>self)
    ()
	newp :: @self. alloc=>
	self. 0..index @slice
	@copyto@ newp
	self. index+1..self.len @slice
	@copyto@ newp
	=self.data
	self ret

	
// example #2

tab_to_comma: char =>char
	is '\t' -> ',' ret
	is &c -> c ret

@loop
	std.scan.char=> tab_to_comma=> std.print=>

// example #3

#enum DayOfWeek
	Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday

DayOfWeek.str: #enum_to_str DayOfWeek

#enum Month
	January, February, March, April, May, June,
	July, August, September, October, November, December,

Month.str: #enum_to_str Month

next: DayOfWeek => DayOfWeek
	is #last DayOfWeek -> #first DayOfWeek ret
	: ++ 1 ret

pad: int day => str~
	str.from.T~>
	.len is 1 -> $~ $.~> ' ' prepend~> ret
	: ret


month: Month m, DayOfWeek start_day, int max_day =>str~
	week: "Su Mo Tu We Th Fr Sa\n"

	days: DayOfWeek, int n =>str
		is Sunday -> n > max_day -> "\n" ret
		is Saturday ->
			Sunday, n++ days=> %
			n pad~=> %% ~>
			%%`\n`% fmt=> ret
		is &day ->
			day next=>, n++ days=> %
			pad=> %% ~>
			%%` `% fmt=> ret

	spaces: DayOfWeek curr_day
		is start_day -> start_day, 1 days=>
		: next=> spaces=> %
		`    `%
		
	[Month.str, addr m] %
	``%` 2015\n`week``spaces

year ::
	s :: ""
	@inline month (Month, DayOfWeek, max_day)
		month=> % ~> s. % append
	

	January, Thursday, 31 month @month
	Febuary, Sunday, 28 month @month
	s

year print=>

# 4 rna decode

#enum RNA A, U, C, G

#enum AminoAcid
    Ala, Cys, Asp, Glu, Phe, Gly, His, Ile, Lys, Leu
    ,Met, Asn, Pro, Gln, Arg, Ser, Thr, Val, Trp, Tyr
    ,Stop

decode: RNA, RNA, RNA =>AminoAcid
	is U, U, U -> Phe ret
	is U, U, C -> Phe ret
	// ...

decode_all: slice.RNA =>slice.AminoAcid
	is { &a, &b, &c, ..&ds } ->
		a, b, c decode=> %
		ds decode_all => . % append=>
	: empty ret
	


