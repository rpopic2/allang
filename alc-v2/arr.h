#define ARR_GENERIC(T, SIZE) \
 \
typedef struct { \
    T data[SIZE]; \
    T *cur; \
    T *end; \
} arr_##T; \
 \
static inline void arr_##T##_init(arr_##T *arr) { \
    arr->cur = arr->data; \
    arr->end = arr->data + SIZE; \
} \
 \
static inline T *arr_##T##_push(arr_##T *arr, const T value) { \
    if (arr->cur == arr->end) { \
        fputs("arr was full", stderr); \
        abort(); \
    } \
    *arr->cur++ = value; \
    return arr->cur - 1; \
} \
 \
static inline void arr_##T##_pop(arr_##T *arr) { \
    if (arr->cur == arr->data) \
        return; \
    arr->cur--; \
} \
 \
static inline T *arr_##T##_top(const arr_##T *arr) { \
    if (arr->cur == arr->data) \
        return 0; \
    return arr->cur - 1; \
} \
 \
static inline bool arr_##T##_is_empty(const arr_##T *arr) { \
    return arr->cur == arr->data; \
} \



#define ARR_GENERICP(T, SIZE) \
 \
typedef struct { \
    T *data[SIZE]; \
    T **cur; \
    T **end; \
} arr_##T##p; \
 \
static inline void arr_##T##p_new(arr_##T##p *arr) { \
    arr->cur = arr->data; \
    arr->end = arr->data + SIZE; \
} \
 \
static inline void arr_##T##p_push(arr_##T##p *restrict arr, T *restrict value) { \
    if (arr->cur == arr->end) { \
        fputs("arr was full", stderr); \
        abort(); \
    } \
    *arr->cur++ = value; \
} \
 \
static inline void arr_##T##p_pop(arr_##T##p *arr) { \
    if (arr->cur == arr->data) \
        return; \
    arr->cur--; \
} \
 \
static inline T *arr_##T##p_top(const arr_##T##p *arr) { \
    if (arr->cur == arr->data) \
        return 0; \
    return arr->cur[-1]; \
} \

