#ifndef TOMORROW_MACROS_HH
#define TOMORROW_MACROS_HH

#include <v8.h>

using namespace v8;


// Convenience calling macros - a bit ugly, but using overloaded functions
// causes all manner of shenannigans to be required to handle scoping.
#define TRY_CALL_NOARG(RESULT, THIS, FUNC, ...) \
	Handle<Value> RESULT; \
	{ \
		TryCatch tryCatch; \
		RESULT = (FUNC)->Call((THIS), 0, NULL); \
		if (tryCatch.HasCaught()) { \
			tryCatch.ReThrow(); \
			RESULT = Undefined(); \
		} \
	}

#define TRY_CALL_ARGS(RESULT, THIS, FUNC, ...) \
	Handle<Value> RESULT; \
	{ \
		TryCatch tryCatch; \
		Handle<Value> params[] = { __VA_ARGS__ }; \
		RESULT = (FUNC)->Call((THIS), (EXPAND_COUNT( __VA_ARGS__ )), params); \
		if (tryCatch.HasCaught()) { \
			tryCatch.ReThrow(); \
			RESULT = Undefined(); \
		} \
	}



#define EXPAND_SUFFIXES ARGS, ARGS, ARGS, ARGS, ARGS, ARGS, ARGS, ARGS, ARGS, NOARG
#define EXPAND_NUMERICS 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define EXPAND_CHOOSE(PREFIX, _1, _2, _3, _4, _5, _6, _7, _8, _9, SUFFIX, ...) PREFIX##SUFFIX

#define EXPAND_OPTIONAL(PREFIX, ...) EXPAND(EXPAND_CHOOSE, PREFIX, __VA_ARGS__, EXPAND_SUFFIXES)
#define EXPAND_COUNT(...) EXPAND(EXPAND_CHOOSE, 0+, __VA_ARGS__, EXPAND_NUMERICS)

#define EXPAND_VC(MACRO, ARGS) MACRO ARGS
#define EXPAND(MACRO, ...) EXPAND_VC(MACRO, (__VA_ARGS__))

#define CALL(RESULT, THIS, ...) \
	EXPAND(EXPAND_OPTIONAL(TRY_CALL_, __VA_ARGS__), RESULT, THIS, __VA_ARGS__)


#endif // TOMORROW_MACROS_HH
