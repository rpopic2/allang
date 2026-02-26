windows_rsi:draw_line: (InStartPos $vector2, InEndPos $vector2, InColor vector2 =>)
    StartPosition, EndPosition ::
        ClippedStart :: InStartPos =[]
        ClippedEnd :: InStartEnd =[]
        cohen_sutherland_line_clip (!ClippedStart !ClippedEnd)
            ScreenExtend :: vector2{[global:ScreenSize.X], [global:ScreenSize.Y]} .* 0.5
            .MinScreen      - ScreenExtend
            .MaxScreen      ScreenExtend
            .ClippedStart   ClippedStart
            .ClippedEnd     ClippedEnd
        =>
            ? ret

        StartPosition = screen_point:to_screen_coordinate global:ScreenSize, [ClippedStart] =>
        EndPosition = screen_point:to_screen_coordinate global:ScreenSize, [ClippedEnd] =>

    Width, Height ::
        EndPosition.X - StartPosition.X,
        EndPosition.Y - StartPosition.Y

    IsGradualSlope :: (abs Width, abs Height) >=

    Dx, Dy ::
        Dx = Width >= 0 ? 1 : -1,
        Dy = Height >= 0 ? 1 : -1
        IsGradualSlope ? Dx swap Dy @

    Fw, Fh ::
        Dx * Width,
        Dy * Height

    F, F1, F2 :: (Fh Fw)
        IsGradualSlope ?
            F = (Fh * 2) - Fw,
            F1 = Fh * 2,
            F2 = (Fh - Fw) * 2
        :
            F = Fw - Fh,
            F1 = Fw * 2,
            F2 = (Fw - Fh) * 2

    X, Y ::
        X = StartPosition.X,
        Y = StartPosition.Y
        IsGradualSlope ? X swap Y @

    TargetPosition :: IsGradualSlope ? EndPosition.X : EndPosition.Y

    while X isnt TargetPosition @ (!X !Y !F)
        screen_point X, Y => .set_pixel InColor =>
        F < 0 ?
            F += F1
        :
            F += F2
            Y += Dy

        X += Dx

