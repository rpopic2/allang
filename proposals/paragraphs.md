## paragraphs

it's like table [] in toml.


physical.color = "orange"
physical.shape = "round"

can be rewritten in

// this may be too fancy

:: physical ::
.color = "orange"
.shape = "round"

// may be unnoticable?

physical .
.color = "orange"
.shape = "round"

paragraph ends on blank lines

physical.color .
.is "orange" => . print =>
.is "red" => . print =>
