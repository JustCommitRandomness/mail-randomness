#define COMMA ,
#define N(...) X(__VA_ARGS__)
#define PARSEPART					\
  X(basename,,) X(base,,)				\
    X(exts, int count ; char *start ;			\
      , .exts.next = exts_n, .exts.start = NULL )	\
    N(protopart,,) N(verpart,,)				\
    N(netpart,,) N(prefixpart,,) N(addrpart,,)		\
    N(portpart,,) N(topart,,)				\
    N(invalid,, .invalid.next = exts_n )
#define X(part,...) part ## _n ,
    typedef enum { PARSEPART part_NULL } enum_parts;
#undef X
#define X(part, FIELDS,...) typedef struct					\
			{ const enum_parts next ; FIELDS } struct_ ## part ;
    PARSEPART ;
#undef X

#define X(part,...) struct_ ## part part ;
typedef struct {
  const char *part ;
  const char *name ;
  const enum_parts type;
  int len ;
  union {
    struct {
      enum_parts next;
    } access ;
    PARSEPART
  };
} struct_parts ;
#undef X
//array of const pointer to const char
#define MAKE_PART(parts, names, ...) (struct_parts) {	\
    .part = NULL , .name = names, .len = 0 ,		\
      .type = parts ## _n ,				\
      .access.next = parts ## _n + 1 ,			\
      __VA_ARGS__					\
} ,
#undef N
#define N(part,z,...) MAKE_PART(part, #part " (unimplemenrdf)", __VA_ARGS__ )
#define X(part,z,...) MAKE_PART(part, #part, __VA_ARGS__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winitializer-overrides"
struct_parts parts[] = { PARSEPART {0} };
#pragma clang diagnostic pop
#undef X
#undef N
#define N(part,...) | 1<< part ## _n
#define X(part,...)
    uint32_t unimplemented = 0 PARSEPART;
#undef X
#undef N
#define N(...) X(__VA_ARGS__)

#define PARSECOUNT X(l) X(ext) X(4dot) X(6colon) X(6coloncolon)
#define X(count) uint c_##count ;
    PARSECOUNT ;
#undef X

typedef enum { Separator, EndCmd, NetPart, RulesEND} enum_rules ;
const char *const *rules = NULL;

const char *const pending     = "mmmmaybeeee??" ;
const char *const none        = "nothing to see" ;
const char **newstyle	      = (const char *[]){ "_-.", "_",  ":", NULL  } ;
const char *const tosubpath[] = { "_-",  "_-", "", NULL   } ;
const char *const toip[]      = { "",    "",   "-_", NULL } ;

typedef enum {
  vnone,
  v6 = 1,
  v4 = 2,
  v4orv6 = 1|2,
  AddrType_MAX
} enum_addrtype ;
const char *const addrchars[AddrType_MAX] =
  { "",
    "0123456789:abcdefABCDEF",
    ".0123456789",
    "0123456789"
  };
const enum_parts netspec_n = protopart_n;
const enum_parts start_n = base_n;


