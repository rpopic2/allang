my_variable :: 42
my_variable ::= 50

50 =my_variable

my_variable(50)

50(my_variable)

my_constant: 42

stack :: 30 =[]
80 =[stack]

[stack] + 10 =[]
(my_variable + 10)(my_variable)

(my_variable += 10)

(my_variable ++ 10)

20 +=my_variable

//

(my_variable)3

my_variable=3

3(my_variable)

3=my_variable

3`myvar

myvar= 3+20
myvar+= 10

[test, i32 3 +=]
[test+= i32 3]

myvar :: 42
myvar= 50
myconstant: 42

myvar :: 42
myvar :: 50
myconstant: 42

myvar :: 42 =[]
50 =[myvar]
[myconstant] =[myvar]

#myconst2: 42
#myconst2 =[myvar]

implicit_integer :: 70
implicit_double :: 70.0
explicit_double :: f64 70
float :: f32 4

label :: "The width is "
width :: 94
width_label :: "`label``width`" fmt

#apples: 3
#oranges: 5

apple_summary :: "I have `apples` apples"
fruit_summary :: "I have `apples + oranges` pieces of fruit"

quotation :: #heredoc EOF
	blah blah blah "" use whatever character before uppercase eof
	uppercase eof can be anything
	`apples` to include  in heredoc use \` for backtick
	EmOF

fruites :: str { "strawberries", "limes", "tangerines" } =[]
"grapes" =[fruites.1]

fruites :: str { "strawberries", "limes", "tangerines" } Std.Array.new=>
fruites, "blueberries" append=>

fruites :: ||
empty_array :: str ||


indivisual_scores: |75, 43, 103, 87, 94|
team_score :: 0

score..indivisual_scores @foreach
	score > 50 -> teamscore(+=3) >>
	: teamscore++
	<<

	score > 50 -> teamscore(+=3) : teamscore++
	
	teamscore(score > 50 -> +=3 : ++)

teamscore print=>

score_decoration ::
	teamscore > 10 -> "👍" : ""

`score: `teamscore` `score_decoration

optional_name :: str? "John Appleseed"
greeting :: "Hello!"

::
	optional_name is pointing, &
	greeting :: `Hello, `&

nickname :: str? not pointing
fullname :: str "John Appleseed"
informal_greeting ::
	nickname is pointing -> nickname : fullname %
	`Hi `%

nickname is pointing -> `Hey, `nickname print=>

vegitable :: "red pepper"
& @@is "celerry" ->>
	"add some raisins..."
& @@is "cucumber" ->>,
or & @@is "watercress" ->>
	"that would make a good..."
& @@ends_with "pepper" ->>
	`Is it a spicy `&
: "everything tastes good..."
<<

interesting_numbers = |
    Prime: |2, 3, 5, 7, 11, 13|,
    Fibonacci: |1, 1, 2, 3, 5, 8|,
    Square: |1, 4, 9, 16, 25|,
|

largest :: 0

numbers..interesting_numbers @foreach
	number..numbers @foreach
		number > largest -> number(largest)
largest print=>

n :: 2
n < 100 @while
	n(#*= 2)

m :: 2
@loop
	m(lsl= 2)
	m >= 100 <-

m < 100 @do_while
	m(lsl= 2)
m print=>

total :: 0
i, 0..4-1 @range
	total(+=i)
total print=>

greet: (@greet =>Str)
	`Hello `person`, today is `day`.`
	struct {
		person Str, day Str
	}

{ person: "Bob", day: "Tuesday" } greet=>

calculate_statistics: (scores slice i32 =>min i32, max i32, sum i32)
	min :: scores[0] @at
	max :: scores[0] @at
	sum :: 0

	score..scores @foreach
		score > max -> max(score) >>
		score < min -> min(score) >>
		<<
		sum(+=score)
	min, max, sum

calculate_statistics: subroutine (scores slice i32 =>min i32, max i32, sum i32)
	scores[0] @at
	=max
	=min
	0 =sum

	score..scores @foreach
		score > max -> max(score) >>
		score < min -> min(score) >>
		<<
		sum(+=score)
	min, max, sum

statistics :: |5, 3, 100, 3, 9| calculate_statistics=>
statistics.sum print=>
statistics.2 print=>

return_fifteen (=>i32)
	y :: 10

	add: subroutine ()
		y(+=5)
	add=>
	y

=>return_fifteen

make_incrementer: (=>addr (i32 =>i32))
	add_one: subrt (number i32 =>i32)
		++
	add_one

increment :: make_incrementer=>

7 increment=>


has_any_matches: (@has_any_matches =>bool)
	item..list @foreach
		item condition=-> true ret
	false
	struct { list slice i32, condition addr (i32 =>bool) }

less_than_ten: subrt (number i32 =>bool)
	<= 10

numbers :: |20, 19, 7, 12|
{ list: numbers, condition: less_than_ten } has_any_matches=>

_closure: subrt (number i32 =>i32)
	@* 3

numbers. _closure map=>

mapped_numbers ::
	_: subrt (number i32 =>i32)
		@*3
	numbers. _ map=>

sorted_numbers ::
    numbers. _ sorted=>
	

		



