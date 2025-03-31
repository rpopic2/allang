_private_function: ()
     println!("called `my_mod.private_function()`");

function: ()
    "called `my_mod::function()`"

indirect_access: ()
    "called `my_mod::indirect_access()`, that\n> " print=>
    _private_function=>

// will be pasted inside "nested" label, since the folder name matches current file name
@paste my_mod/nested.al

call_public_function_in_my_mod
    "called `my_mod::call_public_function_in_my_mod()`, that\n> " print.nl=>
    nested.public_function_in_my_mod();
    "> " print.nl=>
    nested.public_function_in_super_mod();
