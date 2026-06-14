## profiles

it's like .bash_profile. it defines compiler options.
if there is no profile in the cwd, allang uses default settings (or creates one maybe?)

you can explicitly set profile with `#profile foo.al_profile`

## profile contains

it's like compiler switches (cmdline args)

default error handling behavior.
additions/subtraction panics when overflow by default.
division by zero panics by default.

you can choose it to propagate errors by returning, or make it wrap, or make it require to check using !? opererators.
