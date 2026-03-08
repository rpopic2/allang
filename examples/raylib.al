#import raylib5
#alias rl :: raylib

rl.init_window 1280, 720, "Testing" =>

Pos :: vector2{.X 640 .Y 320}

loop:
    rl.window_should_close => ?
        break loop->

    rl.
    .begin_drawing =>
    .clear_background {blue} =>
    .draw_rectangle_v Pos, {.Width 32, .Height 32}, {green} =>

    FrameTime :: rl.get_frame_time =>
    #literal speed :: 400
    Delta :: #speed * FrameTime

    rl.is_key_down.
    {left}  . => ? Pos.X :- Delta >>
    {right} . => ? Pox.X :+ Delta >>
    {up}    . => ? Pos.Y :- Delta >>
    {down}  . => ? Pos.Y :+ Delta >>
    <<

    rl.end_drawing =>

    loop->

rl.close_window =>

