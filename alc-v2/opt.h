#define OPT_GENERIC(T) \
\
typedef struct { \
	bool some; \
	T value; \
} opt_##T; \
\
inline static opt_##T opt_##T##_some(T value) { \
	return (opt_##T){ .some = true, .value = value }; \
} \
 \
static const opt_##T opt_long_none = { .some = false, .value = 0 }; \

#define if_opt(T, varname, expr) \
	opt_##T _##varname expr; \
	T varname = _##varname.value; \
	if (_##varname.some)

#define ifnone_opt(T, varname, expr) \
	opt_##T _##varname = (expr); \
	T varname = _##varname.value; \
	if (!_##varname.some)

#define try_opt(T, varname, expr, ret) \
	opt_##T _##varname = (expr); \
	T varname = _##varname.value; \
	if (!_##varname.some) \
		return ret;

