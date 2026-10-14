#define COMMA ,
#define I(...) X(__VA_ARGS__)
#define N(...) X(__VA_ARGS__)
#define PARSEPART 	I(null,,)			\
    X(log, char const *name;	\
      , .log.name = "log")				\
    X(dir,,) X(basename,,) X(base,,)			\
    X(exts, int count ; char *start ;			\
      , .exts.next = exts_n, .exts.start = NULL )	\
    N(protopart,,) N(verpart,,)				\
    N(netpart,,) N(prefixpart,,) N(addrpart,,)		\
    N(portpart,,) N(topart,,)				\
    I(invalid,, .invalid.next = exts_n )		\
    I(parts_END, enum_parts from ;,)
#define X(part,...) part ## _n ,
typedef enum { PARSEPART part_MAX } enum_parts;
#undef X
#define X(part, FIELDS,...) typedef struct					\
			{ enum_parts const next ; FIELDS } struct_ ## part ;
    PARSEPART ;
#undef X

#define X(part,...) struct_ ## part part ;
typedef struct {
  char const *part ;
  char const *name ;
  enum_parts const type;
  int len ;
  union {
    struct {
      enum_parts next;
    } access ;
    PARSEPART
  };
} struct_parts ;
#undef X

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

#define X(part,...) part ## _n ,
#define N(...)
#undef I
#define I(...)
enum_parts const public_parts[] = { PARSEPART 0 } ;
#undef X
#undef N
#undef I

struct parsed_field {
  enum_parts part;
  char const *name;
  short len;
  char *value;
};

struct parsed { char const *const join; struct parsed_field fields[]; };
typedef enum { Separator, EndCmd, NetPart, RulesMAX} enum_rules ;
typedef char const *Rules[RulesMAX];
Rules const* rules = NULL;
Rules newstyle	      = { "_-.", "_",  ":"  } ;
Rules const tosubpath = { "_-",  "_-", ""   } ;
Rules const toip      = { "",    "",   "-_" } ;

char const *const pending     = "maybe or maybe not"  ;
char const *const none        = &pending[9] ;

typedef enum {
  vnone,
  v6 = 1,
  v4 = 2,
  v4orv6 = 1|2,
  AddrType_MAX
} enum_addrtype ;
char const *const addrchars[AddrType_MAX] =
  { "",
    "0123456789:abcdefABCDEF",
    ".0123456789",
    "0123456789"
  };
enum_parts const netspec_n = protopart_n;
enum_parts const start_n = base_n;


