// from "이득우의 게임 수학" (Lee Deukwoo's game math), 6th chapter 아핀(affine)
// file name should be windows_rsi

// listing 6-1

#enum Coords: X, Y, Z

screen_point:
    struct {
        2*i32 #Coords
    }

    #leaf @inline
    to_screen_coordinate: InScreenSize screen_point, InPos vector2 => screen_point
        ret screen_point{
            .X InPos.X + (InScreenSize.X * 0.5)
            .Y InPos.Y + (InScreenSize.Y * 0.5f)
        }

    #leaf @inline
    to_cartesian_coordinate: InScreenSize screen_point => vector2
        ret vector2{
            .X (X - InScreenSize.X) * 0.5) + 0.5
            .Y - (Y + 0.5f) + (InScreenSize.Y * 0.5)
        }

// listing 6-2
:draw_line: (InStartPos $vector2, InEndPos $vector2, InColor vector2 =>)
    StartPosition, EndPosition ::
        ClippedStart :: InStartPos =[]
        ClippedEnd :: InStartEnd =[]
        :cohen_sutherland_line_clip (!ClippedStart !ClippedEnd)
            ScreenExtend :: vector2{[global:ScreenSize.X], [global:ScreenSize.Y]} .* 0.5
            .MinScreen      - ScreenExtend
            .MaxScreen      ScreenExtend
            .ClippedStart   !ClippedStart
            .ClippedEnd     !ClippedEnd
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

    F, F1, F2 ::
        Fw, Fh ::
            Dx * Width,
            Dy * Height

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

    while X isnt TargetPosition @ (!X !Y !F)
        screen_point X, Y => .set_pixel InColor =>
        F < 0 ?
            F += F1
        :
            F += F2
            Y += Dy

        X += Dx

// listing 6-3

#leaf
:test_region: (InVectorPos vector2, InMinPos vector2, InMaxPos vector2 => i32 $Result)
    Result = 0

    InVectorPos.X
    . < InMinPos.X ?
        Result orr= 0b0001
        >>
    . > InMaxPos.X ?
        Result orr= 0b0010
    <<

    InVectorPos.Y
    . < InMinPos.Y ?
        Result orr= 0b0100
        >>
    . > InMaxPos.Y ?
        Result orr= 0b1000

:cohen_sutherland_line_clip:
    (!InOutStartPos addr vector2, !InOutEndPos addr vector2, InMinPos vector2, InMaxPos vector2 => ?)
    &StartTest :: :test_region [InOutStartPos], InMinPos, InMaxPos =>
    &EndTest :: :test_region [InOutEndPos], InMinPos, InMaxPos =>

    Width :: InOutEndPos.X - InOutStartPos.X
    Height :: InOutEndPos.Y - InOutStartPos.Y

    loop:
    StartTest is 0 ? EndTest is 0 ?
        okret
    StartTest and EndTest isnt zero ?
        eret
    IsStartTest :: StartTest isnt zero
    #leaf ClippedPosition ::
        CurrentTest :: IsStartTest ? StartTest : EndTest
        CurrentTest < 0b0100 ?
            {
                .X CurrentTest and 0b0001 ? InMinPos.X : InMaxPos.X
                .Y Height. math:equals_in_tolerance 0.0 @
                    ? InOutStartPos.Y
                    : InOutStartPos.Y + ((Height * (ClippedPosition.X - InOutStartPos.X)) / Width)
            }
        :
            {
                .Y CurrentTest and 0b0100 ? InMinPos.Y : InMaxPos.Y
                .X Width. math:equals_in_tolerance 0.0 @
                    ? InOutStartPos.X
                    : (ClippedPosition.Y - InOutStartPos.Y) * Width / Height + InOutStartPos.X
            }

    IsStartTest ?
        [InOutStartPos]= ClippedPosition
        StartTest = ClippedPosition. test_region InMinPos, InMaxPos =>
    :
        [InOutEndPos]= ClippedPosition
        EndTest = ClippedPosition. test_region InMinPos, InMaxPos =>
    loop->

