#import test

store_struct: (Dst reg_t, Dtype addr dtype_t, Args addr dyn_agg_member) =>
once:
	Member.Count :: [Args.Cur] - [Args.Begin]
	Is Array ::
		isnt Array =
		Dtype .is_empty =>
		! Dtype .top =>.Tag is DK_ARRAY
		? is Array =

	&Index :: 0
	&Size :: usize{0}
	Rsize ::
		[Dtype.Size] =
		> 8 ? 8 : Self

loop:
	break if Index < Member.Count
	
	Member_Type ::
		Is Array ? Dtype =
		Isnt Array ? [[Dtype.Members.Begin] * Index .Type ! break] =
	Arg :: Args * Index ! panic

	if [Member_Type.Tag] is TK_STRUCT -> 
		if [Arg.Tag] is AGGREGATE ->
			store_struct
				Dst
				, Offset + i64{Size}
				, if Dtype .is_empty => ? Member_Type =[]
				else [Dtype] =[], Inner .pop =>
				, [Arg.Agg]
			=>
		else if [Arg.Tag] is VALUE ? [Arg.Value] is 0 ->
			emit_zerofill Dst, Offset + i64{Size}, Member_type =>

		&Size :+= Member_Type .get_size =>
		&Index :+= 1
		loop->

	Start_Index :: Index
	&Lo :: reg_t{.RegType SCRATCH .Offset 0 .Rsize Rsize .Dtype {.Base Dtype.Base}} =[]
 	Lo.Written :: emit_eightbyte_struct Lo, Dtype, Args, &Index &Size =>

	if Start_Index is Index ->
		compile_error !, "member size whatever..." =>
		break->

	Has_Hi ::
		! =
		[Dtype.Base.Size] - Size >= 8
		? Index < Member_Count
		? [Arg.Tag] isnt AGGREGATE
	&Hi :: Lo .dup @
		.Offset :+= 1
	&Hi.Written :: !

	if Has_Hi ?
		&Lo.Rsize := 8
		&Hi.Rsize := 8
		emit_eightbyte_struct Hi, Dtype, Args, &Index, &Size =>
		=Hi.Written
	emit_store_eightbytes Dst, Offset + i64{Size}, Lo, Lo.Written, Hi, Hi.Written, Has_Hi =>

end loop
struct_case:
	
