// port jsmn to al
// 
// MIT License
//
// Copyright (c) 2010 Serge Zaitsev
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 

JSMN_UNDEFINED: 0
JSMN_OBJECT: 1
JSMN_ARRAY: 2
JSMN_STRING: 4
JSMN_PRIMITIVE: 8

JSMN_ERROR_NOMEM: -1
JSMN_ERROR_INVAL: -2
JSMN_ERROR_PART: -3

ret 0

// TODO make enum, flag enum, allow lsl constant-folding
jsmntype:
    struct { Value i32 }

jsmn_parser:
    struct {
        Pos u32,
        Toknext u32,
        Toksuper i32,
    }

jsmntok:
    struct {
        Type i32,
        Start i32,
        End i32,
        Size i32,
    }

jsmn_alloc_token: Parser addr jsmn_parser, Tokens slice jsmntok => !addr jsmntok
    Toknext :: [Parser.Toknext]

    Tok :: Tokens * Toknext ! eret

    Toknext + 1 =[Parser.Toknext]

    -1 =[Tok.End] =[Tok.Start]
    0 =[Tok.Size]

    ret Tok

jsmn_fill_token: Token addr jsmntok, Type i32, Start i32, End i32 =>
    Type =[Token.Type]
    Start =[Token.Start]
    End =[Token.End]
    0 =[Token.Size]

tmp: I !u8 =>
    I ! ret
    ret

jsmn_parse_primitive: Parser addr jsmn_parser, Js slice !u8, Tokens slice jsmntok => i32
    Start :: [Parser.Pos]

    loop:
        Pos :: [^Parser.Pos]
        C :: [^Js * Pos ! loop.break->] ! loop.break->
        C is ':' found->
        C is '\t' found->
        C is '\r' found->
        C is '\n' found->
        C is ' ' found->
        C is ',' found->
        C is ']' found->
        C is '}' found->

        C < 32 ->
            ^^Start =[^^Parser.Pos]
            ret JSMN_ERROR_INVAL
        C >= 127 ->
            ^^Start =[^^Parser.Pos]
            ret JSMN_ERROR_INVAL
        Pos + 1 =[^Parser.Pos]
        loop->
    loop.break:

    found:
    Tokens.Length is 0 ->
        [^Parser.Pos] - 1 =[^Parser.Pos]
        ret 0

    // Token :: jsmn_alloc_token Parser, Tokens => ! ret JSMN_ERROR_NOMEM
    Token ::
        jsmn_alloc_token ^Parser, ^Tokens => =
        ^Token ! ret JSMN_ERROR_NOMEM
    End :: [Parser.Pos]
    jsmn_fill_token Token, JSMN_PRIMITIVE, i32{Start}, i32{End} =>

    End - 1 =[Parser.Pos]
    ret 0

jsmn_parse_string: Parser addr jsmn_parser, Js slice !u8, Tokens !slice jsmntok => i32
    [Start] :: [Parser.Pos] =[]

    // Skip starting quote
    [Start] + 1 =[Parser.Pos]

    loop:
    Pos :: [Parser.Pos]
    [C] :: [Js * Pos ! loop.break->] ! loop.break-> =[]
    [C] is '"' ->
        ^Tokens ! ret 0

        Token ::
            jsmn_alloc_token ^^Parser, ^^Tokens => =
            ^Token ! ret JSMN_ERROR_NOMEM

        Start2 :: [^Start] + 1
        jsmn_fill_token Token, JSMN_STRING, i32{Start2}, i32{^Pos} =>

        ret 0

    // Backslash: Quoted symbol expected
    backslash:
    [C] is '\\' ->
        [^Parser.Pos] + 1 =^Pos
        D :: [^Js * ^Pos ! backslash.break->]
        ^Pos =[^Parser.Pos]

        D is '"' loop.break->
        D is '/' loop.break->
        D is '\\' loop.break->
        D is 'b' loop.break->
        D is 'f' loop.break->
        D is 'r' loop.break->
        D is 'n' loop.break->
        D is 't' loop.break->

        // Allows escaped symbol \uXXXX
        D is 'u' ->
            ^^Pos + 1 =[^^Parser.Pos]

            [I] :: 0 =[]
            uloop:
            E :: [^^Js * ^^Pos ! uloop.break->] ! uloop.break->
            [I] >= 4 uloop.break->
            
            [I] + 1 =[I]
            ^^Pos + 1 =^^Pos =[^^Parser.Pos]
            uloop->
            uloop.break:

            loop.break->
        
        [^Start] =[^Parser.Pos]
        ret JSMN_ERROR_INVAL
        
    backslash.break:

    [Parser.Pos] + 1 =[Parser.Pos]
    loop->
    loop.break:

    [Start] =[Parser.Pos]
    ret JSMN_ERROR_PART

jsmn_parse: Parser addr jsmn_parser, Js slice !u8, Tokens !slice jsmntok => i32
    [Count] :: [Parser.Toknext] =[]

    loop:
    [C] ::
        Pos :: [^Parser.Pos]
        [^Js * Pos ! loop.break->] ! loop.break-> =[]
    [C] is '{' brackets->
    [C] is '[' brackets->
    brackets:
        [^Count] + 1 =[^Count]
        ^Tokens ! switch.break->
        Token :: jsmn_alloc_token ^Parser, ^Tokens =>
        Token ! ret JSMN_ERROR_NOMEM

        Toksuper :: [^Parser.Toksuper]
        Toksuper isnt -1 ->
            T :: ^^Tokens * ^Toksuper unchecked
            [T.Size] + 1 =[T.Size]

        [^C] is '{' ->
            JSMN_OBJECT =[^Token.Type]
        [^C] isnt '{' ->
            JSMN_ARRAY =[^Token.Type]

        [^Parser.Pos] =[Token.Start]
        [^Parser.Toknext] - 1 =[^Parser.Toksuper]
        switch.break->

    [C] is '}' bracket_close->
    [C] is ']' bracket_close->
        ^Tokens.Length is 0 switch.break->
        Type ::
            JSMN_ARRAY =This
            [^^C] is '}' ->
                JSMN_OBJECT =^^Type
            0
        0

        loop2:
        I :: [^Parser.Toknext] - 1
        I < 0 loop2.break->

        Token :: ^Tokens * I unchecked
        [Token.Start] is -1 ->
            [^Token.End] is -1 ->
                [^^Token.Type] is ^^Type ->
                    ret JSMN_ERROR_INVAL
                -1 =[^^^Parser.Toksuper]
                [^^^Parser.Pos] + 1 =[^^Token.End]
                switch.break->
            0
        0
        I is -1 ->
            ret JSMN_ERROR_INVAL

        loop3:
        I < 0 loop3.break->

        I - 1 =I
        loop3->
        loop3.break:
            ^^Tokens * ^I unchecked =^Token
        I - 1 =I
        loop2->
        loop2.break:

        0
        switch.break->

    [C] is '"' ->
        R :: jsmn_parse_string ^Parser, ^Js, ^Tokens =>
        R < 0 ->
            ret ^R
        [^Count] + 1 =[^Count]
        Toksuper :: [^Parser.Toksuper]
        Toksuper isnt -1 ->
            ^^Tokens.Length isnt 0 ->
                Tmp :: ^^^Tokens * ^^Toksuper unchecked
                [Tmp.Size] + 1 =[Tmp.Size]
            0
        switch.break->
    0

    [C] is '\t' switch.break->
    [C] is '\r' switch.break->
    [C] is '\n' switch.break->
    [C] is ' ' switch.break->

    [C] is ':' ->
        [^Parser.Toknext] - 1 =[^Parser.Toksuper]
        switch.break->

    [C] is ',' ->
        ^Tokens.Length is 0 ->
            Toksuper :: [^^Parser.Toksuper]
            Toksuper isnt -1 ->
                Tmp :: ^^^Tokens * ^Toksuper unchecked
                [Tmp.Type] isnt JSMN_ARRAY ->
                    [^Tmp.Type] isnt JSMN_OBJECT dummy_loop->
                0
            0
        0
    dummy_loop:

    dummy_loop.break:

    switch.break:

    [Parser.Pos] + 1 =[Parser.Pos]
    loop->
    loop.break:

    ret 0

jsmn_init: Parser addr jsmn_parser =>
    0 =[Parser.Pos]
    0 =[Parser.Toknext]
    -1 =[Parser.Toksuper]
