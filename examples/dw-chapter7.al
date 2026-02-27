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
    G :: get_2d_game_engine =>
    Input :: G .get_input_manger =>

    MoveSpeed: f32{100.0}
    RandomPosX: math:uniform_distribution{.Start -300.0 .EndExclusive 300}
    RandomPosY: math:uniform_distribution{.Start -200.0 .EndExclusive 200}
    Duration: f32{3.0}
    !ElapsedTime: f32{0.0}
    TargetStart: [:Target:Position]
    TargetDestination: vector2{.X RandomPosX .random => .Y RandomPosY .random =>}
    #alias TargetDst :: TargetDestination

    HalfFovCos: cos (math:deg2rad :FovAngle * 0.5 @)

    $ElapsedTime :: ([ElapsedTime] + InDeltaSeconds) .clamp 0.0, [Duration] @ =:[ElapsedTime]
    . equals [Duration] ?
        [TargetDestination]
            =:[TargetStart]=
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
        InputVecor .* MoveSpeed .* InDeltaSeconds

    F :: vector2{UnitY}
    V ::
        [:Target:Position] .- [:Player:Position]
        . get_normalize @

    Test :: V. dot F @ >= [HalfFovCos]
        [!:Player:Color]= Test ? {Red} : {Gray}
        [!:Target:Color]= Test ? {Red} : {Blue}

    [:Player:Position] + DeltaPosition =[!:Player:Position]

:render_2d: =>
    R :: get_renderer =>
    G :: get_2d_game_engine =>

    Radius: f32{5.0}
    Sphere: (dyn_array vector2):new => =[]
    SightLength: f32{300.0}

    $Sphere. is_empty @ ?
        $Radius :: [Radius]
        RR :: Radius * Radius
        X :: - $Radius ?<= Radius
        loop:
            Y :: - $Radius ?<= Radius
            loop:
                Target :: vector2{.X X .Y Y}
                SizeSquared :: Target. size_squared =>
                SizeSquared < RR ?
                    $Sphere. push Target =>
            Y += 1 ? loop->
        X += 1 ? loop->

    [HalfFovSin], [HalfFovCos] ::
        math:get_sin_cos HalfFovSin, init HalfFovCos, [:FovAngle] * 0.5 =>

    PlayerPosition :: [:Player:Position]
    R. draw_line
        PlayerPosition,
        PlayerPosition .+ vector2{$SightLength * HalfFovSin, SightLehgth * HalfFovCos},
        PlayerColor
    =>
    R. draw_line
        PlayerPosition,
        PlayerPosition .+ vector2{(- $SightLength) * HalfFovSin, SightLehgth * HalfFovCos},
        PlayerColor
    =>
    R. draw_line
        PlayerPosition,
        PlayerPosition .+ vector2{UnitY} .* $SightLength .* 0.2,
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
