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
    struct { i32 }

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

jsmn_alloc_token: Parser addr jsmn_parser, Tokens slice jsmntok => addr! jsmntok
    Toknext :: [Parser.Toknext]

    Tok :: Tokens * Toknext ! eret
    ret Tok


