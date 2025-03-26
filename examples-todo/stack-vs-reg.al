// stack vars: three i32s. stack size: 0x20, fp : sp + 0x10
i32 foo // fp + 4
i32 bar // fp + 8
// reserve stack space for foo and bar

reg :: 3 // mov reg, 0x3
reg =foo  // str reg, [fp, -4]

// i32 foo: 3 // compile err!
foo := 3  // can be assigned only to result of str
// fp + 12
// mov x0, 3
// str x0, [fp, -12]

reg :: foo // add reg, sp, 12

[foo] =bar  // ldr x0, [fp, -12]
            // str x0, [fp, -8]
// i           // you cannot use before declared
// i ::         // compile err!
// i32 i        // compile err!
i :: 0      // mov w19, 0x0
i32 i :: 0  // mov w19, 0x0

0
:: i        // should we allow this?

i           // mov w0, w19


// on stack 

[] i ::= 0   // mov w8, 0;
            // str w8 [sp, 4]
[i32] i ::= 0

i           // add x0, sp, 4
[i]         // ldr x0, [sp, 4]


addr p :: malloc=> // bl malloc; mov w19, x0
p       // mov x0, x19
[p]     // ldr x19, x0
