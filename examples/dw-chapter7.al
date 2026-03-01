// soft_renderer2d.al
:
    FovAngle: f32{60.0}

    Player.
        Position: vector2{0.0, 0.0}
        Color: linear_color{Gray}

    Target.
        Position: vector2{0.0, 0.0}
        Color: linear_color{Blue}

#(!:Player:Position !:Player:Color !:Target:Position !:Target:Color)
:update_2d: InDeltaSeconds f32 =>
    MoveSpeed: f32{100.0}
    RandomPosX: math:uniform_distribution{.StartInclusive -300.0 .EndExclusive 300}
    RandomPosY: math:uniform_distribution{.StartInclusive -200.0 .EndExclusive 200}
    Duration: f32{3.0}
    !ElapsedTime: f32{0.0}
    TargetStart: [:Target:Position]
    TargetDestination: vector2{.X RandomPosX .random => .Y RandomPosY .random =>}
    #alias TargetDst :: TargetDestination

    HalfFovCos: #comptime ([:FovAngle] * 0.5). deg2rad @). cos

    G :: get_2d_game_engine =>
    Input :: G .get_input_manger =>

    $ElapsedTime :: ([ElapsedTime] + InDeltaSeconds). clamp 0.0, [Duration] @ =:[ElapsedTime]
    . equals [Duration] ?
        [TargetDestination]
            =:[TargetStart]
            =:[!:Target:Position]

        [TargetDestination]= vector2{.X RandomPosX .random => .Y RandomPosY .random =>}
        [ElapsedTime]= 0.0
    :
        Ratio :: [ElapsedTime] / Duration
        [:Target:Position]= vector2{
            .X math:lerp [TargetStart.X], [TargetDst.X], Ratio =>
            .Y math:lerp [TargetStart.Y], [TargetDst.Y], Ratio =>
        }

    InputVector ::
        vector2 {
            .X Input. get_axis {XAxis}
            .Y Input .get_axis {YAxis}
        }
        . get_normalize @
    DeltaPosition ::
        $MoveSpeed :: [MoveSpeed]
        InputVecor. * MoveSpeed.* InDeltaSeconds

    F :: vector2{UnitY}
    V ::
        [:Target:Position] .- [:Player:Position]
        . get_normalize @

    Test :: V. dot F @ >= [HalfFovCos]
        [!:Player:Color]= Test ? {Red} : {Gray}
        [!:Target:Color]= Test ? {Red} : {Blue}

    [:Player:Position] + DeltaPosition =[!:Player:Position]

:render_2d: =>
    Radius: f32{5.0}
    Sphere: dyn_array.vector2{0}
    SightLength: f32{300.0}

    R :: get_renderer =>
    G :: get_2d_game_engine =>

    Sphere :: [Self:Sphere]
    Sphere. is_empty @ ?
        Radius :: [Self:Radius]
        RR :: Radius * Radius
        inc X in (- Radius)..Radius incl @
            inc Y in (- Radius)..Radius incl @
                Target :: vector2{.X X .Y Y}
                Target .size_squared =>
                . < RR ?
                    Sphere .push Target =>

    [HalfFovSin], [HalfFovCos] ::
        math:get_sin_cos init HalfFovSin, init HalfFovCos, [:FovAngle] * 0.5 =>

    PlayerPosition :: [::Player.Position]
    SightLength :: [:SightLength]
    R. draw_line
        PlayerPosition,
        PlayerPosition .+ vector2{SightLength * HalfFovSin, SightLehgth * HalfFovCos} @,
        PlayerColor
    =>
    R. draw_line
        PlayerPosition,
        PlayerPosition .+ vector2{(- SightLength) * HalfFovSin, SightLehgth * HalfFovCos} @,
        PlayerColor
    =>
    R. draw_line
        PlayerPosition,
        (PlayerPosition .+ vector2{unitY} @) .* SightLength @) .* 0.2 @,
        PlayerColor
    =>

    foreach V in Sphere @
        R .draw_point
            [:Player:Position]
            .Point V + .,
            .Color [:Player:Color]
        =>

    foreach V in Sphere @
        R .draw_point
            [:Target:Position]
            .Point V + .,
            .Color [:Target:Color]
        =>
