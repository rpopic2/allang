#include <stdbool.h>

typedef struct {
	bool some;
	long value;
} opt_long;

inline static opt_long opt_long_some(long value) {
	return (opt_long){ .some = true, .value = value };
}

static const opt_long opt_long_none = { .some = false, .value = 0 };

inline static bool opt_long_try(const opt_long self, long *out) {
	*out = self.value;
	return self.some;
}

#define if_opt_long(varname, expr) \
	opt_long _##varname = (expr); \
	long varname = _##varname.value; \
	if (_##varname.some)

#define ifnone_opt_long(varname, expr) \
	opt_long _##varname = (expr); \
	long varname = _##varname.value; \
	if (!_##varname.some)

#define try_opt_long(varname, expr, ret) \
	opt_long _##varname = (expr); \
	long varname = _##varname.value; \
	if (!_##varname.some) \
		return ret;

