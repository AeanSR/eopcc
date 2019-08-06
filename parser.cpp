#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <tuple>
#include <utility>
#include <set>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <functional>
#include <variant>
#include <bitset>

using namespace std::string_literals;

inline constexpr uint64_t knuth(uint64_t x) {
  return x * 6364136223846793005ULL + 1442695040888963407ULL;
}
inline constexpr uint64_t knuth(const char * x) {
  return *x ? knuth(knuth(knuth(*x) ^ knuth(__LINE__)) ^ knuth(x + 1)) :
         knuth(__LINE__);
}

int verbose_level = 0;
#define verbose(level, ...) (verbose_level >= level && [&](){ __VA_ARGS__; return true; }())

std::set<std::string> keywords = {
  "if", "else", "for", "do", "while", "continue", "break", "return", "print",
  "sizeof", "typeof",
  "def", "async", "await", "except",
  "conv", "pool", "mm", "act", "trans", "cycleadd",
  "int", "float", "vector", "extern", "intern", "const",
  "EOPConvolution", "EOPFullyConnected", "EOPPooling", "EOPConcat", "EOPSplit",
  "EOPGroupConv", "EOPDepthwiseConv"
};

std::set<std::string> punctuators = {
  "[", "]", "(", ")", "{", "}", ".", "->", "++", "--", "&", "*", "+",
  "-", "~", "!", "/", "%", "<<", ">>", "<", ">", "<=", ">=", "==", "!=",
  "^", "|", "&&", "||", "?", ":", ";", "...", "=", "*=", "/=", "%=", "+=",
  "-=", "<<=", ">>=", "&=", "^=", "|=", ",", "#", "##", "<:", ":>", "<%",
  "%>", "%:", "%:%:",
};
std::set<char> simple_escapes = {
  '\'', '"', '?', '\\', 'a', 'b', 'f', 'n', 'r', 't', 'v',
};
std::map<char, char> simple_escapes_tr = {
  { '\'', '\''}, { '"', '"' }, { '?', '?' }, { '\\', '\\' }, { 'a', '\a' }, { 'b', '\b' },
  { 'f', '\f' }, { 'n', '\n' }, { 'r', '\r' }, { 't', '\t' }, { 'v', '\v' },
};

#define in +in_recieve_set_t()*
#define contains *in_recieve_set_t()+
#define cc(...) std::set<decltype(__VA_ARGS__)>({__VA_ARGS__})

template<class T>
struct in_recieve_key_t {
  T set;
  in_recieve_key_t(T set) : set(set) { }
};

struct in_recieve_set_t {
  
};

template<class T>
in_recieve_key_t<T> operator*(in_recieve_set_t lhs, T rhs) {
  return in_recieve_key_t<T>(rhs);
}
template<class T>
in_recieve_key_t<T> operator*(T lhs, in_recieve_set_t rhs) {
  return in_recieve_key_t<T>(lhs);
}
template<class T1, class T2>
bool operator+(in_recieve_key_t<T1> lhs, T2 rhs) {
  return lhs.set.find(rhs) != lhs.set.end();
}
template<class T1, class T2>
bool operator+(T2 lhs, in_recieve_key_t<T1> rhs) {
  return rhs.set.find(lhs) != rhs.set.end();
}

#define is %is_recieve_ptr_t()%
struct is_recieve_ptr_t {
  std::type_index type1;
  std::type_index type2;
  operator bool() const { return type1 == type2; }
  is_recieve_ptr_t() : type1(typeid(void)), type2(typeid(void)) {  };
  template<class T> is_recieve_ptr_t operator%(std::shared_ptr<T> rhs)
    { is_recieve_ptr_t ret(*this); return ret.type2 = std::type_index(typeid(*rhs)), ret; }
  is_recieve_ptr_t operator%(const std::type_info& rhs)
    { is_recieve_ptr_t ret(*this); return ret.type2 = std::type_index(rhs), ret; }
};
template<class T> is_recieve_ptr_t operator%(std::shared_ptr<T> lhs, is_recieve_ptr_t rhs)
  { is_recieve_ptr_t ret(rhs); return ret.type1 = std::type_index(typeid(*lhs)), ret; }
is_recieve_ptr_t operator%(const std::type_info& lhs, is_recieve_ptr_t rhs)
  { is_recieve_ptr_t ret(rhs); return ret.type1 = std::type_index(lhs), ret; }

inline bool is_digit(char c) {
  return c >= '0' && c <= '9';
}
inline bool is_octaldigit(char c) {
  return c >= '0' && c <= '7';
}
inline bool is_hexaldigit(char c) {
  return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
inline int char_to_digit(char c) {
  if (is_digit(c)) return c - '0';
  else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}
inline bool is_lowercase(char c) {
  return c >= 'a' && c <= 'z';
}
inline bool is_uppercase(char c) {
  return c >= 'A' && c <= 'Z';
}
inline bool is_alphabet(char c) {
  return is_lowercase(c) || is_uppercase(c);
}
inline bool is_identifier(char c) {
  return is_alphabet(c) || c == '_';
}

std::vector<std::string> raw;
int errors_occurred = 0;
int warnings_occurred = 0;

struct final_report_t {
  ~final_report_t() {
    std::cout << errors_occurred << " error(s), " << warnings_occurred << " warning(s)" << std::endl;
  }
};

final_report_t& final_report() {
  static final_report_t fr;
  return fr;
}

void rewind_function_instantiation_stack();
struct cursor_t {
  int lineno;
  int charno;
  cursor_t() : lineno(0), charno(0) { }
  cursor_t(int l, int c) : lineno(l), charno(c) { }
  std::ostream& error() {
    return errors_occurred++, rewind_function_instantiation_stack(), std::cout << lineno + 1 << ":" << charno + 1 << ":" << "error: ";
  }
  std::ostream& warn() {
    return warnings_occurred++, rewind_function_instantiation_stack(), std::cout << lineno + 1 << ":" << charno + 1 << ":" << "warning: ";
  }
  std::ostream& note() {
    return std::cout << lineno + 1 << ":" << charno + 1 << ":" << "note: ";
  }
  std::string eol() {
    std::stringstream ss;
    ss << std::endl << "\t" << raw[lineno] << (raw[lineno].back() == '\n' ? "\t" : "\n\t") << std::string(charno, ' ') << "^" << std::endl << std::endl;
    return ss.str();
  }
};

struct char_reader_t : public cursor_t {
  std::list<std::tuple<int, int>> stack;
  int cla() const {
    if (lineno >= raw.size() - 1 || (lineno == raw.size() - 2 && charno >= raw[lineno].size())) return EOF;
    else return raw[lineno][charno];
  }
  int cla(int d) {
    if (!d) return cla();
    else {
      mark();
      int c = cgf(d);
      unmark();
      return c;
    }
  }
  int cgf(int d = 1) {
    while (d--) {
      if (cla() == EOF) return EOF;
      if (cla() == '\n') {
        lineno++; charno=0;
      } else {
        charno++;
      }
    }
    return cla();
  }
  void mark() {
    stack.emplace_back(lineno, charno);
  }
  void unmark() {
    std::tie(lineno, charno) = stack.back();
    stack.pop_back();
  }
  int operator*() const {
    return cla();
  }
  char_reader_t operator++(int i) {
    char_reader_t copy(*this);
    return cgf(), copy;
  }
  int operator[](int d) {
    return cla(d);
  }
  char_reader_t& operator+=(int d) {
    return cgf(d), *this;
  }
};

struct token_t : public cursor_t {
  int type;
  std::string identify;
  int64_t integral;
  double floating;
  enum {
    NIT,
    PUNC,
    ID,
    KEY,
    INT,
    FLOAT,
    STR,
  };
  token_t(cursor_t* c, int type, std::string str) : cursor_t(*c), type(type), identify(str) { }
  token_t(cursor_t* c, int64_t integral) : cursor_t(*c), type(INT), integral(integral) { }
  token_t(cursor_t* c, double floating) : cursor_t(*c), type(FLOAT), floating(floating) { }
  std::string desc() const {
    std::vector<std::string> token_type_lut = {
      "end of file",
      "punctuator ",
      "identifier ",
      "keyword ",
      "number literal ",
      "number literal ",
      "string literal ",
    };
    std::stringstream ss;
    ss << token_type_lut[type];
    if (type == NIT) { }
    else if (type == KEY) { ss << identify; }
    else if (type == INT) { ss << integral; }
    else if (type == FLOAT) { ss << floating; }
    else { ss << "\"" << identify << "\""; }
    return ss.str();
  }
};



void read_source(const char* fn) {
  std::ifstream sf(fn);
  char c;
  std::string line;
  while((c=sf.get()) != EOF) {
    line.push_back(c);
    if (c == '\n') {
      raw.push_back(line);
      line.clear();
    }
  }
  if (!line.empty()) raw.push_back(line);
  raw.push_back("[EOF]");
}

std::vector<token_t> tokens;

void lexer() {
    enum {
      S_BREAK, S_COMMENT_SLASH, S_COMMENT_STAR,
      S_NUMLIT, S_ZNUMLIT, S_OCTLIT, S_HEXLIT, S_FLOATLIT, S_FLOATLIT_EXP,
      S_STRLIT, S_STRESCAPE,
      S_CHRLIT, S_CHRESCAPE,
      S_IDENTIFY
    };
    int state = S_BREAK;
    std::string memory;
    int64_t number;
    char_reader_t p;
    char_reader_t token_start;
    while(*p != EOF) {
      if (*p == '\\' && (p[1] == 0x0d || p[1] == 0x0a)) {
        if (p[1] == 0x0d && p[2] == 0x0a) p += 3;
        else p += 2;
      } else switch(state) {
        case S_BREAK:
          memory.clear();
          if (*p in cc(' ', '\t', '\r', '\n'))
            { p++; }
          else if (*p == '/' && p[1] == '/')
            { state = S_COMMENT_SLASH; p += 2; }
          else if (*p == '/' && p[1] == '*')
            { state = S_COMMENT_STAR; p += 2; }
          else if (*p == '0')
            { state = S_ZNUMLIT; token_start = p++; }
          else if (is_digit(*p))
            { state = S_NUMLIT; token_start = p; memory.push_back(*p++); }
          else if (is_identifier(*p))
            { state = S_IDENTIFY; token_start = p; memory.push_back(*p++); }
          else if (*p == '"')
            { state = S_STRLIT; token_start = p++; }
          else if (*p == '\'')
            { state = S_CHRLIT; token_start = p++; }
          else {
            int is_punc = 0;
            for (int len = 3; len > 0; len--) {
              token_start = p;
              memory.clear();
              for (int i = 0; i < len; i++) memory.push_back(*p++);
              if (memory in punctuators) {
                tokens.emplace_back(&token_start, token_t::PUNC, memory);
                is_punc = 1;
                break;
              }
              p = token_start;
            }
            if (!is_punc) {
              p.error() << "unexpected character '" << (char)*p << "'(" << (unsigned)*p << ")" << p.eol();
              p++;
            }
          } break;
        case S_IDENTIFY:
          if (is_identifier(*p) || is_digit(*p))
            { memory.push_back(*p++); }
          else {
            state = S_BREAK;
            if (memory in keywords) {
              tokens.emplace_back(&token_start, token_t::KEY, memory);
            } else {
              tokens.emplace_back(&token_start, token_t::ID, memory);
            }
          } break;
        case S_NUMLIT:
          if (is_digit(*p))
            { memory.push_back(*p++); }
          else if (*p == '.')
            { state = S_FLOATLIT; memory.push_back(*p++); }
          else if (*p in cc('E', 'e'))
            { state = S_FLOATLIT_EXP; memory.push_back(*p++); }
          else {
            if (*p in cc('U', 'u')) p++;
            if (*p in cc('L', 'l')) p++;
            if (*p in cc('L', 'l')) p++;
            state = S_BREAK;
            tokens.emplace_back(&token_start, (int64_t)atoll(memory.c_str()));
          } break;
        case S_ZNUMLIT:
          if (is_octaldigit(*p))
            { state = S_OCTLIT; number = char_to_digit(*p++); }
          else if (*p == '.')
            { state = S_FLOATLIT; memory.push_back(*p++); }
          else if (*p in cc('E', 'e'))
            { state = S_FLOATLIT_EXP; memory.push_back(*p++); }
          else if (*p in cc('X', 'x'))
            { state = S_HEXLIT; number = 0; p++; }
          else {
            if (*p in cc('U', 'u')) p++;
            if (*p in cc('L', 'l')) p++;
            if (*p in cc('L', 'l')) p++;
            state = S_BREAK;
            tokens.emplace_back(&token_start, (int64_t)0);
          } break;
        case S_OCTLIT:
          if (is_octaldigit(*p))
            { number *= 8; number += char_to_digit(*p++); }
          else {
            if (*p in cc('U', 'u')) p++;
            if (*p in cc('L', 'l')) p++;
            if (*p in cc('L', 'l')) p++;
            state = S_BREAK;
            tokens.emplace_back(&token_start, number);
          } break;
        case S_FLOATLIT:
          if (is_digit(*p))
            { memory.push_back(*p++); }
          else if (*p in cc('E', 'e'))
            { state = S_FLOATLIT_EXP; memory.push_back(*p++); }
          else {
            if (*p in cc('F', 'f')) p++;
            if (*p in cc('L', 'l')) p++;
            state = S_BREAK;
            tokens.emplace_back(&token_start, atof(memory.c_str()));
          } break;
        case S_FLOATLIT_EXP:
          if (is_digit(*p))
            { state = S_FLOATLIT; memory.push_back(*p++); }
          else if (*p in cc('+', '-'))
            { state = S_FLOATLIT; memory.push_back(*p++); }
          else {
            p.error() << "expect sign or digits after exponential symbol." << p.eol();
            state = S_BREAK;
            memory.push_back('1');
            tokens.emplace_back(&token_start, atof(memory.c_str()));
          } break;
        case S_HEXLIT:
          if (is_hexaldigit(*p))
            { number *= 16; number += char_to_digit(*p++); }
          else {
            if (*p in cc('U', 'u')) p++;
            if (*p in cc('L', 'l')) p++;
            if (*p in cc('L', 'l')) p++;
            state = S_BREAK;
            tokens.emplace_back(&token_start, number);
          } break;
        case S_STRLIT:
          if (*p == '"')
            { state = S_BREAK; p++; tokens.emplace_back(&token_start, token_t::STR, memory); }
          else if (*p == '\\')
            { state = S_STRESCAPE; p++; }
          else if (*p == '\n')
            { p.error() << "missing terminating quotation mark." << p.eol();
              state = S_BREAK; tokens.emplace_back(&token_start, token_t::STR, memory); }
          else {
            memory.push_back(*p++);
          } break;
        case S_STRESCAPE:
          if (*p in simple_escapes)
            { state = S_STRLIT; memory.push_back(simple_escapes_tr[*p++]); }
          else if (is_octaldigit(*p)) {
            number = 0;
            for (int i = 0; i < 3; i++) {
              if (is_octaldigit(*p))
                number = number * 8 + char_to_digit(*p++);
              else {
                p.warn() << "expected 3 octal digits for an escape, got " << i << "." << p.eol();
                break;
              }
            }
            memory.push_back((char)number);
            state = S_STRLIT;
          } else if (*p == 'x') {
            number = 0;
            p++;
            for (int i = 0; i < 2; i++) {
              if (is_hexaldigit(*p))
                number = number * 16 + char_to_digit(*p++);
              else {
                p.warn() << "expected 2 hex digits for an escape, got " << i << "." << p.eol();
                break;
              }
            }
            memory.push_back((char)number);
            state = S_STRLIT;
          } else {
            p.warn() << "unkown escape sequence." << std::endl;
            memory.push_back(*p++);
            state = S_STRLIT;
          } break;
        case S_CHRLIT:
          if (*p == '\'')
            { state = S_BREAK; p++; tokens.emplace_back(&token_start, token_t::STR, memory); }
          else if (*p == '\\')
            { state = S_CHRESCAPE; p++; }
          else if (*p == '\n')
            { p.error() << "missing terminating quotation mark." << p.eol();
              state = S_BREAK; tokens.emplace_back(&token_start, token_t::STR, memory); }
          else {
            memory.push_back(*p++);
          } break;
        case S_CHRESCAPE:
          if (*p in simple_escapes)
            { state = S_CHRLIT; memory.push_back(simple_escapes_tr[*p++]); }
          else if (is_octaldigit(*p)) {
            number = 0;
            for (int i = 0; i < 3; i++) {
              if (is_octaldigit(*p))
                number = number * 8 + char_to_digit(*p++);
              else {
                p.warn() << "expected 3 octal digits for an escape, got " << i << "." << p.eol();
                break;
              }
            }
            memory.push_back((char)number);
            state = S_CHRLIT;
          } else if (*p == 'x') {
            number = 0;
            p++;
            for (int i = 0; i < 2; i++) {
              if (is_hexaldigit(*p))
                number = number * 16 + char_to_digit(*p++);
              else {
                p.warn() << "expected 2 hex digits for an escape, got " << i << "." << p.eol();
                break;
              }
            }
            memory.push_back((char)number);
            state = S_CHRLIT;
          } else {
            p.warn() << "unkown escape sequence." << std::endl;
            memory.push_back(*p++);
            state = S_CHRLIT;
          } break;
        case S_COMMENT_SLASH:
          if (*p++ == '\n') state = S_BREAK; break;
        case S_COMMENT_STAR:
          if (*p == '*' && p[1] == '/')
            { state = S_BREAK; p += 2; }
          else p++; break;
        default:
          p.error() << "internal error: unexpected state of lexer." << p.eol();
          state = S_BREAK;
      }
    }
    tokens.emplace_back(&p, token_t::NIT, memory);
}

int syn_cursor = 0;
int syn_error_cur = 0;
std::string syn_error_message;

bool upd_error(std::string err_info) {
  if (syn_cursor >= syn_error_cur) {
    syn_error_cur = syn_cursor;
    syn_error_message = err_info;
  }
  return false;
}

bool expect_key(std::string name) {
  if (tokens[syn_cursor].type == token_t::KEY && tokens[syn_cursor].identify == name) return syn_cursor++, true;
  return upd_error("expect keyword " + name + ", got " + tokens[syn_cursor].desc());
}

bool expect_punc(std::string name) {
  if (tokens[syn_cursor].type == token_t::PUNC && tokens[syn_cursor].identify == name) return syn_cursor++, true;
  return upd_error("expect punctuator \"" + name + "\", got " + tokens[syn_cursor].desc());
}

bool expect_id(std::string& get) {
  if (tokens[syn_cursor].type == token_t::ID) return get = tokens[syn_cursor++].identify, true;
  return upd_error("expect identifier, got " + tokens[syn_cursor].desc());
}

bool expect_int(int64_t& get) {
  if (tokens[syn_cursor].type == token_t::INT) return get = tokens[syn_cursor++].integral, true;
  return upd_error("expect integer literal, got " + tokens[syn_cursor].desc());
}

bool expect_float(double& get) {
  if (tokens[syn_cursor].type == token_t::FLOAT) return get = tokens[syn_cursor++].floating, true;
  return upd_error("expect number literal, got " + tokens[syn_cursor].desc());
}

bool expect_str(std::string& get) {
  if (tokens[syn_cursor].type == token_t::STR) return get = tokens[syn_cursor++].identify, true;
  return upd_error("expect string literal, got " + tokens[syn_cursor].desc());
}

template<class T>
bool expect(std::shared_ptr<T>& get) {
  auto _get = std::make_shared<T>();
  if (_get->parse()) {
    get = _get;
    return true;
  }
  return false;
}

#define guard(...) [&](){ auto _guard_b = syn_cursor; bool _guard_r = (__VA_ARGS__); if (_guard_r) return _guard_r; else syn_cursor = _guard_b; return _guard_r; }()

#define select_punc(type, enum_start, ...) do { int i = enum_start; for (auto&& p : std::vector<std::string>({__VA_ARGS__})) if (expect_punc(p)) { type = i; break; } else i++; } while(0)
#define select_key(type, enum_start, ...) do { int i = enum_start; for (auto&& p : std::vector<std::string>({__VA_ARGS__})) if (expect_key(p)) { type = i; break; } else i++; } while(0)
#define expect_multipuncs(type, enum_start, ...) [&](){ type = -1; select_punc(type, enum_start, __VA_ARGS__); return type >= 0; }()
#define expect_multikeys(type, enum_start, ...) [&](){ type = -1; select_key(type, enum_start, __VA_ARGS__); return type >= 0; }()
#define left_aggregate(major_type, minor_type, punc) [&](){                         \
    decltype(rhs) first;                                                            \
    if (guard(expect(first) && punc && expect(rhs))) {                              \
      lhs = std::make_shared<typename decltype(lhs)::element_type>();               \
      lhs->set_cursor();                                                            \
      lhs->minor_type = first;                                                      \
      auto type_ = type;                                                            \
      while (guard(punc && expect(first))) {                                        \
        auto trailing = std::make_shared<typename decltype(lhs->major_type)::element_type>(*this); \
        trailing->set_cursor();                                                     \
        lhs = std::make_shared<typename decltype(lhs)::element_type>();             \
        lhs->set_cursor();                                                          \
        lhs->major_type = trailing;                                                 \
        rhs = first;                                                                \
      }                                                                             \
      type = type_;                                                                 \
      return true;                                                                  \
    } else return false;                                                            \
  }()

struct ast_node_t;

std::map<std::pair<int, std::type_index>, std::pair<int, std::shared_ptr<ast_node_t>>> syntr;

struct ast_node_t : public cursor_t, std::enable_shared_from_this<ast_node_t> {
  using ptr = std::shared_ptr<ast_node_t>;
  void set_cursor() { lineno = tokens[syn_cursor].lineno; charno = tokens[syn_cursor].charno;  }
  virtual bool _parse() = 0;
  bool parse() {
    set_cursor();
    auto _b = syn_cursor;
    verbose( 3, note() << "parse " << typeid(*this).name() << " from here" << eol() );
    auto tr = syntr.find({syn_cursor, std::type_index(typeid(*this))});
    if (tr != syntr.end()) {
      /*
          transposition disabled due to bug :(
          this should dramatically boost the parser.
      */
      /*if (tr->second.second)
        *this = *tr->second.second;
        syn_cursor = tr->second.first;
      verbose( 3, note() << "found transpositioned parse result " << tr->second.second << eol() );
      return tr->second.second != nullptr;*/
      if (tr->second.second == nullptr) return false; // only negative records are probed from transpositions to avoid the undiscovered dangling-pointer bug.
    }                                                 // but this should also boost the parser in order of magnitudes :D
    bool r = _parse();
    verbose( 3, note() << "parse result of " << typeid(*this).name() << ":" << r << eol() );
    syntr[decltype(syntr)::key_type({_b, std::type_index(typeid(*this))})] = r ? decltype(syntr)::mapped_type({ syn_cursor, shared_from_this() }) : decltype(syntr)::mapped_type({ 0, nullptr });
    return r;
  }
};

struct ident_t;
struct expr_t;
struct assignexpr_t;
struct arglist_t;
//struct rangeexpr_t;
struct assignment_t;
struct condexpr_t;
//struct range_t;
struct castexpr_t;
struct cast_t;
struct lorexpr_t;
struct cond_t;
struct landexpr_t;
struct lor_t;
struct orexpr_t;
struct land_t;
struct xorexpr_t;
struct or_t;
struct andexpr_t;
struct xor_t;
struct equalityexpr_t;
struct and_t;
struct relationalexpr_t;
struct equality_t;
struct shiftexpr_t;
struct relational_t;
struct addexpr_t;
struct shift_t;
struct mulexpr_t;
struct add_t;
struct unaryexpr_t;
struct mul_t;
struct unary_t;
struct postfixexpr_t;
struct descexpr_t;
struct callexpr_t;
struct postfix_t;
struct pexpr_t;
struct vexpr_t;
struct iexpr_t;
struct fexpr_t;
struct stmt_t;
struct compstmt_t;
struct ctrlstmt_t;
struct forstmt_t;
struct ifstmt_t;
struct retstmt_t;
struct contstmt_t;
struct brkstmt_;
struct barrstmt_t;
struct type_t;
struct declstmt_t;
struct vardecl_t;
struct funcdecl_t;
struct exprstmt_t;

// expressions ===============================================================

struct ident_t : public ast_node_t {
  std::string name;
  virtual bool _parse() {
    return expect_id(name);
  }
};

ast_node_t::ptr null_ast() {
  static auto n = std::make_shared<ident_t>();
  return n;
}

struct sexpr_t : public ast_node_t {
  std::string lit;
  virtual bool _parse() {
    return expect_str(lit);
  }
};

struct vexpr_t : public ast_node_t {
  std::shared_ptr<ident_t> var;
  virtual bool _parse() {
    return expect(var);
  }
};

struct fake_assign_t : public ast_node_t {
  std::shared_ptr<ident_t> lhs;
  virtual bool _parse() {
    return expect(lhs);
  }
};

struct iexpr_t : public ast_node_t {
  int64_t value;
  virtual bool _parse() {
    return expect_int(value);
  }
};

struct fexpr_t : public ast_node_t {
  double value;
  virtual bool _parse() {
    return expect_float(value);
  }
};

struct pexpr_t : public ast_node_t {
  std::shared_ptr<expr_t> e;
  std::shared_ptr<sexpr_t> s;
  std::shared_ptr<vexpr_t> v;
  std::shared_ptr<iexpr_t> i;
  std::shared_ptr<fexpr_t> f;
  virtual bool _parse() {
    return expect(s) || expect(v) || expect(i) || expect(f) ||
           guard(expect_punc("(") && expect(e) && expect_punc(")"));
  }
};

struct callexpr_t : public ast_node_t {
  bool async;
  std::shared_ptr<vexpr_t> lhs;
  std::shared_ptr<expr_t> args;

  virtual bool _parse() {
    return guard([&](){
      async = expect_key("async");
      return expect(lhs) && expect_punc("(") && (expect(args), true) && expect_punc(")");
    }());
  }
};

struct postfixexpr_t : public ast_node_t {
  std::shared_ptr<callexpr_t> call;
  std::shared_ptr<descexpr_t> desc;
  std::shared_ptr<postfix_t> postfix;
  std::shared_ptr<pexpr_t> primary;

  virtual bool _parse();
};

struct descexpr_t : public ast_node_t {
  std::shared_ptr<postfixexpr_t> lhs;
  std::shared_ptr<expr_t> desc;

  virtual bool _parse() {
    if (!lhs) { error() << "internal error: postfix should not be parsed directly by recursive descent." << eol(); return false; }
    return guard(expect_punc("[") && expect(desc) && expect_punc("]"));
  }
};

struct postfix_t : public ast_node_t {
  std::shared_ptr<postfixexpr_t> lhs;
  int type;
  enum { INC, DEC };

  virtual bool _parse() {
    if (!lhs) { error() << "internal error: postfix should not be parsed directly by recursive descent." << eol(); return false; }
    return expect_multipuncs(type, INC, "++", "--");
  }
};

  bool postfixexpr_t::_parse() {
    if (expect(call)) return true;
    if (guard(expect(primary))) {
      while(1) {
        auto leading = std::make_shared<postfixexpr_t>(*this);
        auto try_desc = std::make_shared<descexpr_t>();
        auto try_post = std::make_shared<postfix_t>();
        try_desc->lhs = leading;
        try_post->lhs = leading;
        if (try_desc->parse()) {
          desc = try_desc;
          primary = nullptr;
        } else if (try_post->parse()) {
          postfix = try_post;
          primary = nullptr;
        } else return true;
      }
    } return false;
  }

struct unaryexpr_t : public ast_node_t {
  std::shared_ptr<postfixexpr_t> postfix;
  std::shared_ptr<unary_t> unary;

  virtual bool _parse() {
    return expect(unary) || expect(postfix);
  }
};

struct unary_t : public ast_node_t {
  std::shared_ptr<unaryexpr_t> lhs;
  int type;
  enum { INC, DEC, PROD, ALL, POS, NEG, NOT, REV, ANY, SIZEOF, TYPEOF };

  virtual bool _parse() {
    return guard([&](){
      if (expect_multipuncs(type, INC, "++", "--", "*", "&", "+", "-", "!", "~", "|")) {
        return expect(lhs);
      } else if (expect_key("sizeof")) {
        type = SIZEOF;
        return expect(lhs);
      } else if (expect_key("typeof")) {
        type = TYPEOF;
        return expect(lhs);
      } else return false;
    }());
  }
};

struct castexpr_t : public ast_node_t {
  std::shared_ptr<unaryexpr_t> unary;
  std::shared_ptr<cast_t> cast;

  virtual bool _parse() {
    return expect(cast) || expect(unary);
  }
};

struct cast_t : public ast_node_t {
  std::shared_ptr<castexpr_t> lhs;
  std::shared_ptr<type_t> type;

  virtual bool _parse() {
    return guard(expect_punc("(") && expect(type) && expect_punc(")") && expect(lhs));
  }
};

struct mulexpr_t : public ast_node_t {
  std::shared_ptr<castexpr_t> cast;
  std::shared_ptr<mul_t> mul;

  virtual bool _parse() {
    return expect(mul) || expect(cast);
  }
};

struct mul_t : public ast_node_t {
  std::shared_ptr<mulexpr_t> lhs;
  std::shared_ptr<castexpr_t> rhs;
  int type;
  enum { MUL, DIV, MOD };

  virtual bool _parse() {
    return left_aggregate(mul, cast, expect_multipuncs(type, MUL, "*", "/", "%"));
  }
};

struct addexpr_t : public ast_node_t {
  std::shared_ptr<mulexpr_t> mul;
  std::shared_ptr<add_t> add;

  virtual bool _parse() {
    return expect(add) || expect(mul);
  }
};

struct add_t : public ast_node_t {
  std::shared_ptr<addexpr_t> lhs;
  std::shared_ptr<mulexpr_t> rhs;
  int type;
  enum { ADD, SUB };

  virtual bool _parse() {
    return left_aggregate(add, mul, expect_multipuncs(type, ADD, "+", "-"));
  }
};

struct shiftexpr_t : public ast_node_t {
  std::shared_ptr<addexpr_t> add;
  std::shared_ptr<shift_t> shift;

  virtual bool _parse() {
    return expect(shift) || expect(add);
  }
};

struct shift_t : public ast_node_t {
  std::shared_ptr<shiftexpr_t> lhs;
  std::shared_ptr<addexpr_t> rhs;
  int type;
  enum { SHL, SHR };

  virtual bool _parse() {
    return left_aggregate(shift, add, expect_multipuncs(type, SHL, "<<", ">>"));
  }
};

struct relationalexpr_t : public ast_node_t {
  std::shared_ptr<shiftexpr_t> shift;
  std::shared_ptr<relational_t> relational;

  virtual bool _parse() {
    return expect(relational) || expect(shift);
  }
};

struct relational_t : public ast_node_t {
  std::shared_ptr<relationalexpr_t> lhs;
  std::shared_ptr<shiftexpr_t> rhs;
  int type;
  enum { LT, GT, LEQ, GEQ };

  virtual bool _parse() {
    return left_aggregate(relational, shift, expect_multipuncs(type, LT, "<", ">", "<=", ">="));
  }
};

struct equalityexpr_t : public ast_node_t {
  std::shared_ptr<relationalexpr_t> relational;
  std::shared_ptr<equality_t> equality;

  virtual bool _parse() {
    return expect(equality) || expect(relational);
  }
};

struct equality_t : public ast_node_t {
  std::shared_ptr<equalityexpr_t> lhs;
  std::shared_ptr<relationalexpr_t> rhs;
  int type;
  enum { EQ, NEQ };

  virtual bool _parse() {
    return left_aggregate(equality, relational, expect_multipuncs(type, EQ, "==", "!="));
  }
};

struct andexpr_t : public ast_node_t {
  std::shared_ptr<equalityexpr_t> equality;
  std::shared_ptr<and_t> and_;

  virtual bool _parse() {
    return expect(and_) || expect(equality);
  }
};

struct and_t : public ast_node_t {
  std::shared_ptr<andexpr_t> lhs;
  std::shared_ptr<equalityexpr_t> rhs;
  int type;

  virtual bool _parse() {
    return left_aggregate(and_, equality, expect_punc("&"));
  }
};

struct xorexpr_t : public ast_node_t {
  std::shared_ptr<andexpr_t> and_;
  std::shared_ptr<xor_t> xor_;
  int type;

  virtual bool _parse() {
    return expect(xor_) || expect(and_);
  }
};

struct xor_t : public ast_node_t {
  std::shared_ptr<xorexpr_t> lhs;
  std::shared_ptr<andexpr_t> rhs;
  int type;

  virtual bool _parse() {
    return left_aggregate(xor_, and_, expect_punc("^"));
  }
};

struct orexpr_t : public ast_node_t {
  std::shared_ptr<xorexpr_t> xor_;
  std::shared_ptr<or_t> or_;
  int type;

  virtual bool _parse() {
    return expect(or_) || expect(xor_);
  }
};

struct or_t : public ast_node_t {
  std::shared_ptr<orexpr_t> lhs;
  std::shared_ptr<xorexpr_t> rhs;
  int type;

  virtual bool _parse() {
    return left_aggregate(or_, xor_, expect_punc("|"));
  }
};

struct landexpr_t : public ast_node_t {
  std::shared_ptr<orexpr_t> or_;
  std::shared_ptr<land_t> land;

  virtual bool _parse() {
    return expect(land) || expect(or_);
  }
};

struct land_t : public ast_node_t {
  std::shared_ptr<landexpr_t> lhs;
  std::shared_ptr<orexpr_t> rhs;
  int type;

  virtual bool _parse() {
    return left_aggregate(land, or_, expect_punc("&&"));
  }
};

struct lorexpr_t : public ast_node_t {
  std::shared_ptr<landexpr_t> land;
  std::shared_ptr<lor_t> lor;

  virtual bool _parse() {
    return expect(lor) || expect(land);
  }
};

struct lor_t : public ast_node_t {
  std::shared_ptr<lorexpr_t> lhs;
  std::shared_ptr<landexpr_t> rhs;
  int type;

  virtual bool _parse() {
    return left_aggregate(lor, land, expect_punc("||"));
  }
};

struct cond_t : public ast_node_t {
  std::shared_ptr<lorexpr_t> cond;
  std::shared_ptr<expr_t> lhs;
  std::shared_ptr<condexpr_t> rhs;

  virtual bool _parse() {
    return guard(expect(cond) && expect_punc("?") && expect(lhs) && expect_punc(":") && expect(rhs));
  }
};

struct condexpr_t : public ast_node_t {
  std::shared_ptr<lorexpr_t> lor;
  std::shared_ptr<cond_t> cond;

  virtual bool _parse() {
    return expect(cond) || expect(lor);
  }
};
/*
struct range_t : public ast_node_t {
  std::shared_ptr<condexpr_t> lhs;
  std::shared_ptr<condexpr_t> rhs;

  virtual bool _parse() {
    return guard([&](){
      expect(lhs);
      bool r = expect_punc(":");
      return r ? expect(rhs) || true : false;
    }());
  }
};

struct rangeexpr_t : public ast_node_t {
  std::shared_ptr<condexpr_t> cond;
  std::shared_ptr<range_t> range;

  virtual bool _parse() {
    return expect(range) || expect(cond);
  }
};
*/
struct assignment_t : public ast_node_t {
  std::shared_ptr<castexpr_t> lhs;
  std::shared_ptr<assignexpr_t> rhs;
  int type;
  enum { EQ, ADDEQ, SUBEQ, SHLEQ, SHREQ, MULEQ, DIVEQ, MODEQ, ANDEQ, OREQ, XOREQ };

  virtual bool _parse() {
    return guard(expect(lhs) && expect_multipuncs(type, EQ, "=", "+=", "-=", "<<=", ">>=", "*=", "/=", "%=", "&=", "|=", "^=") && expect(rhs));
  }
};

struct assignexpr_t : public ast_node_t {
  std::shared_ptr<condexpr_t> cond;
  std::shared_ptr<assignment_t> assign;

  virtual bool _parse() {
    return expect(assign) || expect(cond);
  }
};

struct expr_t : public ast_node_t {
  std::shared_ptr<assignexpr_t> assign;
  std::shared_ptr<arglist_t> arglist;

  virtual bool _parse() {
    return expect(arglist) || expect(assign);
  }
};

struct arglist_t : public ast_node_t {
  std::shared_ptr<expr_t> lhs;
  std::shared_ptr<assignexpr_t> rhs;
  virtual bool _parse() {
    int type;
    return left_aggregate(arglist, assign, expect_punc(","));
  }
};

// statements ================================================================

struct intrinstmt_t : public ast_node_t {
  int opcode;
  enum { CONV, POOL, MM, ACT, TRANS, CYCLEADD };
  std::shared_ptr<arglist_t> args;

  virtual bool _parse() {
    return guard(expect_multikeys(opcode, CONV, "conv", "pool", "mm", "act", "trans", "cycleadd") && expect(args) && expect_punc(";"));
  }
};

struct forstmt_t : public ast_node_t {
  std::shared_ptr<expr_t> init;
  std::shared_ptr<expr_t> cond;
  std::shared_ptr<expr_t> step;
  std::shared_ptr<stmt_t> body;

  virtual bool _parse() {
    return guard(expect_key("for") && expect_punc("(") && expect(init) && expect_punc(";") && expect(cond) && expect_punc(";") && expect(step) && expect_punc(")") && expect(body));
  }
};

struct ifstmt_t : public ast_node_t {
  std::shared_ptr<expr_t> cond;
  std::shared_ptr<stmt_t> pos;
  std::shared_ptr<stmt_t> neg;

  virtual bool _parse() {
    bool r = guard(expect_key("if") && expect_punc("(") && expect(cond) && expect_punc(")") && expect(pos));
    if (r) guard(expect_key("else") && expect(neg));
    return r;
  }
};

struct dostmt_t : public ast_node_t {
  std::shared_ptr<stmt_t> body;
  std::shared_ptr<expr_t> cond;

  virtual bool _parse() {
    return guard(expect_key("do") && expect(body) && expect_key("while") && expect_punc("(") && expect(cond) && expect_punc(")") && expect_punc(";"));
  }
};

struct whilestmt_t : public ast_node_t {
  std::shared_ptr<expr_t> cond;
  std::shared_ptr<stmt_t> body;

  virtual bool _parse() {
    return guard(expect_key("while") && expect_punc("(") && expect(cond) && expect_punc(")") && expect(body));
  }
};

struct retstmt_t : public ast_node_t {
  virtual bool _parse() {
    return guard(expect_key("return") && expect_punc(";"));
  }
};

struct contstmt_t : public ast_node_t {
  virtual bool _parse() {
    return guard(expect_key("continue") && expect_punc(";"));
  }
};

struct breakstmt_t : public ast_node_t {
  virtual bool _parse() {
    return guard(expect_key("break") && expect_punc(";"));
  }
};

struct awaitstmt_t : public ast_node_t {
  std::shared_ptr<expr_t> handler;
  virtual bool _parse() {
    return guard(expect_key("await") && expect(handler) && expect_punc(";"));
  }
};

struct printstmt_t : public ast_node_t {
  std::shared_ptr<expr_t> objs;

  virtual bool _parse() {
    return guard(expect_key("print") && expect(objs) && expect_punc(";"));
  }
};

struct ctrlstmt_t : public ast_node_t {
  std::shared_ptr<forstmt_t> for_;
  std::shared_ptr<ifstmt_t> if_;
  std::shared_ptr<dostmt_t> do_;
  std::shared_ptr<whilestmt_t> while_;
  std::shared_ptr<retstmt_t> ret_;
  std::shared_ptr<contstmt_t> cont_;
  std::shared_ptr<breakstmt_t> break_;
  std::shared_ptr<awaitstmt_t> await_;

  virtual bool _parse() {
    return expect(for_) || expect(if_) || expect(do_) || expect(while_) || expect(ret_) || expect(cont_) || expect(break_) || expect(await_);
  }
};

struct type_t : public ast_node_t {
  int type;
  enum { INT, FLOAT, VECTOR, EXCEPT };
  int storage;
  enum { INTERN, EXTERN, CONST };
  std::vector<std::shared_ptr<expr_t>> size;

  virtual bool _parse() {
    return guard([&](){
      if (expect_key("except")) {
        type = EXCEPT; return true;
      }
      if (expect_key("intern")) storage = INTERN;
      else if (expect_key("extern")) storage = EXTERN;
      else if (expect_key("const")) storage = CONST;
      else storage = INTERN;
      if (expect_key("int")) type = INT;
      else if (expect_key("float")) type = FLOAT;
      else if (expect_key("vector")) type = VECTOR;
      else return false;
      if (type == VECTOR) {
        decltype(size)::value_type s;
        while(guard(expect_punc("[") && expect(s) && expect_punc("]"))) size.insert(size.begin(), s);
        return true;
      } else return true;
    }());
  }
};

struct vardecl_t : public ast_node_t {
  std::shared_ptr<type_t> type;
  std::shared_ptr<ident_t> ident;
  std::vector<std::shared_ptr<expr_t>> size;
  std::shared_ptr<expr_t> init;

  virtual bool _parse() {
    return guard([&](){
      if (expect(type) && expect(ident)) {
        decltype(size)::value_type s;
        while(guard(expect_punc("[") && expect(s) && expect_punc("]"))) size.insert(size.begin(), s);
        if (expect_punc("=")) return expect(init);
        else return true;
      } return false;
    }());
  }
};

struct funcdecl_t : public ast_node_t {
  std::string name;
  std::vector<std::shared_ptr<vexpr_t>> args;
  std::shared_ptr<compstmt_t> body;

  virtual bool _parse() {
    return guard([&](){
      if (!expect_key("def")) return false;
      if (expect_id(name) && expect_punc("(")) {
        if (expect_punc(")")) return expect(body);
        do {
          decltype(args)::value_type arg;
          if (!expect(arg)) return false;
          args.push_back(arg);
        } while(expect_punc(","));
        return expect_punc(")") && expect(body);
      } return false;
    }());
  }
};

struct declstmt_t : public ast_node_t {
  std::shared_ptr<vardecl_t> var;
  std::shared_ptr<funcdecl_t> func;

  virtual bool _parse() {
    return expect(func) || guard(expect(var) && expect_punc(";"));
  }
};

struct exprstmt_t : public ast_node_t {
  std::shared_ptr<expr_t> expr;

  virtual bool _parse() {
    return guard((expect(expr), expect_punc(";")));
  }
};

struct compstmt_t : public ast_node_t {
  std::vector<std::shared_ptr<stmt_t>> stmts;

  virtual bool _parse() {
    return guard([&](){
      if (expect_punc("{")) {
        decltype(stmts)::value_type stmt;
        while(expect(stmt)) stmts.push_back(stmt);
        return expect_punc("}");
      }
      return false;
    }());
  }
};

struct stmt_t : public ast_node_t {
  std::shared_ptr<intrinstmt_t> intrin;
  std::shared_ptr<exprstmt_t> expr;
  std::shared_ptr<declstmt_t> decl;
  std::shared_ptr<compstmt_t> comp;
  std::shared_ptr<ctrlstmt_t> ctrl;
  std::shared_ptr<printstmt_t> print_;

  virtual bool _parse() {
    return expect(print_) || expect(intrin) || expect(ctrl) || expect(comp) || expect(decl) || expect(expr);
  }
};

struct builtin_t : public ast_node_t {
  int opcode;
  enum { CONV, MLP, POOL, CONCAT, SPLIT, GROUP_CONV, DEPTHWISE_CONV };
  std::shared_ptr<arglist_t> args;

  virtual bool _parse() {
    return guard(expect_multikeys(opcode, CONV,
        "EOPConvolution", "EOPFullyConnected", "EOPPooling", "EOPConcat", "EOPSplit", "EOPGroupConv", "EOPDepthwiseConv"
      ) && expect_punc("(") && expect(args) && expect_punc(")") && expect_punc(";"));
  }
};

struct translation_unit_t : public ast_node_t {
  std::vector<std::shared_ptr<stmt_t>> stmts;
  std::shared_ptr<builtin_t> builtin;

  virtual bool _parse() {
    if (expect(builtin)) return true;
    decltype(stmts)::value_type stmt;
    while(expect(stmt)) stmts.push_back(stmt);
    return !stmts.empty();
  }
};

std::vector<std::shared_ptr<translation_unit_t>> ast;

bool parser() {
  std::shared_ptr<translation_unit_t> tru;
  while(expect(tru)) {
    ast.push_back(tru);
    syntr.clear();
  }
  if (tokens[syn_cursor].type != token_t::NIT) {
    tokens[syn_error_cur].error() << "syntax error: " << syn_error_message << tokens[syn_error_cur].eol();
    return false;
  } else {
    return true;
  }
}

struct addr_t;
using simval_t = std::variant<std::monostate, int64_t, double, std::string, std::shared_ptr<addr_t>>;

struct symbol_t : public cursor_t, std::enable_shared_from_this<symbol_t> {
  using ptr = std::shared_ptr<symbol_t>;
  symbol_t(ast_node_t::ptr cur) : cursor_t(cur->lineno, cur->charno) { }
  symbol_t(const symbol_t& rhs) : cursor_t(rhs) { }
  virtual ~symbol_t() = default;
  template <class T>
  std::shared_ptr<T> to() {
    return std::dynamic_pointer_cast<T>(this ? shared_from_this() : nullptr);
  }
};

struct symbol_null_t : public symbol_t {
  std::string name;
  symbol_null_t(ast_node_t::ptr cur, std::string name) : symbol_t(cur), name(name) { }
};

struct symbol_type_t : public symbol_t {
  using ptr = std::shared_ptr<symbol_type_t>;
  bool external;
  bool const_;
  virtual int64_t sizeof_() const = 0;
  virtual ptr degrade() const = 0;
  virtual ptr intern(bool external = false) const = 0;
  virtual ptr constant(bool constant = false) const = 0;
  virtual uint64_t signature() const = 0;
  virtual std::string _name() const = 0;
  virtual bool is_scalar() const { return false; };
  std::string name() const { return (external ? "extern "s : ""s) + _name(); }
  symbol_type_t(ast_node_t::ptr cur) : symbol_t(cur), external(false), const_(false) { }
  symbol_type_t(const symbol_type_t& rhs) : symbol_t(rhs), external(rhs.external), const_(rhs.const_) { }
};

struct symbol_val_t : public symbol_t {
  using ptr = std::shared_ptr<symbol_val_t>;
  symbol_type_t::ptr type;
  virtual simval_t constexpr_eval() const { return std::monostate(); }
  virtual bool lvalue() const { return false; }
  virtual bool rvvalue() const { return false; }
  symbol_val_t(ast_node_t::ptr cur, symbol_type_t::ptr type) : symbol_t(cur), type(type) { }
};

struct symbol_stmt_t : public symbol_t {
  using ptr = std::shared_ptr<symbol_stmt_t>;
  symbol_stmt_t(ast_node_t::ptr cur) : symbol_t(cur) { }
};

struct symbol_fundamental_type_t : public symbol_type_t {
  symbol_fundamental_type_t(ast_node_t::ptr ast) : symbol_type_t(ast ? ast : null_ast()) { }
  symbol_fundamental_type_t(const symbol_fundamental_type_t& rhs) : symbol_type_t(rhs) { }
};

struct symbol_void_type_t : public symbol_fundamental_type_t {
  symbol_void_type_t(ast_node_t::ptr ast = nullptr) : symbol_fundamental_type_t(ast) { }
  virtual int64_t sizeof_() const { return 0; }
  virtual uint64_t signature() const { return knuth(knuth(__LINE__)); }
  virtual symbol_type_t::ptr degrade() const { return std::make_shared<symbol_void_type_t>(); }
  virtual std::string _name() const { return "void"s; }
  template<class... Args>
  static std::shared_ptr<symbol_void_type_t> new_(Args&&... args) { return std::make_shared<symbol_void_type_t>(std::forward<Args>(args)...); }
  virtual ptr intern(bool ex=false) const { auto c = std::make_shared<symbol_void_type_t>(*this); c->external = ex; return c; }
  virtual ptr constant(bool co=false) const { auto c = std::make_shared<symbol_void_type_t>(*this); c->const_ = co; return c; }
  symbol_void_type_t(const symbol_void_type_t& rhs) : symbol_fundamental_type_t(rhs) { }
};

struct symbol_int_type_t : public symbol_fundamental_type_t {
  symbol_int_type_t(ast_node_t::ptr ast = nullptr) : symbol_fundamental_type_t(ast) { }
  virtual int64_t sizeof_() const { return 8; }
  virtual uint64_t signature() const { return knuth(knuth(__LINE__)); }
  virtual symbol_type_t::ptr degrade() const { return std::make_shared<symbol_void_type_t>(); }
  virtual std::string _name() const { return "int"s; }
  virtual bool is_scalar() const { return true; }
  template<class... Args>
  static std::shared_ptr<symbol_int_type_t> new_(Args&&... args) { return std::make_shared<symbol_int_type_t>(std::forward<Args>(args)...); }
  virtual ptr intern(bool ex=false) const { auto c = std::make_shared<symbol_int_type_t>(*this); c->external = ex; return c; }
  virtual ptr constant(bool co=false) const { auto c = std::make_shared<symbol_int_type_t>(*this); c->const_ = co; return c; }
  symbol_int_type_t(const symbol_int_type_t& rhs) : symbol_fundamental_type_t(rhs) { }
};

struct symbol_float_type_t : public symbol_fundamental_type_t {
  symbol_float_type_t(ast_node_t::ptr ast = nullptr) : symbol_fundamental_type_t(ast) { }
  virtual int64_t sizeof_() const { return 8; }
  virtual uint64_t signature() const { return knuth(knuth(__LINE__)); }
  virtual symbol_type_t::ptr degrade() const { return std::make_shared<symbol_void_type_t>(); }
  virtual std::string _name() const { return "float"s; }
  virtual bool is_scalar() const { return true; }
  template<class... Args>
  static std::shared_ptr<symbol_float_type_t> new_(Args&&... args) { return std::make_shared<symbol_float_type_t>(std::forward<Args>(args)...); }
  virtual ptr intern(bool ex=false) const { auto c = std::make_shared<symbol_float_type_t>(*this); c->external = ex; return c; }
  virtual ptr constant(bool co=false) const { auto c = std::make_shared<symbol_float_type_t>(*this); c->const_ = co; return c; }
  symbol_float_type_t(const symbol_float_type_t& rhs) : symbol_fundamental_type_t(rhs) { }
};

struct symbol_vec_type_t : public symbol_fundamental_type_t {
  std::vector<int64_t> size;
  symbol_vec_type_t(ast_node_t::ptr ast = nullptr, std::vector<int64_t> size = {}) : symbol_fundamental_type_t(ast), size(size) { }
  virtual int64_t sizeof_() const { int64_t ss = 2; for (auto&& s : size) ss *= s; return ss; }
  virtual uint64_t signature() const { uint64_t sig = knuth(knuth(__LINE__)); for (auto&& s : size) sig = knuth(sig ^ knuth(s ^ knuth(__LINE__))); return sig; }
  virtual symbol_type_t::ptr degrade() const { if (size.empty()) return std::make_shared<symbol_void_type_t>(); auto copy = std::make_shared<symbol_vec_type_t>(*this); copy->size.pop_back(); return copy; }
  virtual std::string _name() const {
    std::stringstream ss;
    ss << "vector";
    if (!size.empty()) {
      ss << "<";
      bool first = true;
      for (auto s = size.rbegin(); s != size.rend(); s++) {
        ss << (first ? "" : ",") << *s;
        first = false;
      }
      ss << ">";
    }
    return ss.str();
  }
  virtual bool is_scalar() const { return size.empty(); }
  template<class... Args>
  static std::shared_ptr<symbol_vec_type_t> new_(Args&&... args) { return std::make_shared<symbol_vec_type_t>(std::forward<Args>(args)...); }
  virtual ptr intern(bool ex=false) const { auto c = std::make_shared<symbol_vec_type_t>(*this); c->external = ex; return c; }
  virtual ptr constant(bool co=false) const { auto c = std::make_shared<symbol_vec_type_t>(*this); c->const_ = co; return c; }
  symbol_vec_type_t(const symbol_vec_type_t& rhs) : symbol_fundamental_type_t(rhs), size(rhs.size) { }
};

struct symbol_str_type_t : public symbol_fundamental_type_t {
  symbol_str_type_t(ast_node_t::ptr ast = nullptr) : symbol_fundamental_type_t(ast) { }
  virtual int64_t sizeof_() const { return 0; }
  virtual uint64_t signature() const { return knuth(knuth(__LINE__)); }
  virtual symbol_type_t::ptr degrade() const { return std::make_shared<symbol_void_type_t>(); }
  virtual std::string _name() const { return "<string-literal>"s; }
  template<class... Args>
  static std::shared_ptr<symbol_str_type_t> new_(Args&&... args) { return std::make_shared<symbol_str_type_t>(std::forward<Args>(args)...); }
  virtual ptr intern(bool ex=false) const { return nullptr; }
  virtual ptr constant(bool co=false) const { return nullptr; }
  symbol_str_type_t(const symbol_str_type_t& rhs) : symbol_fundamental_type_t(rhs) { }
};

struct symbol_handler_type_t : public symbol_fundamental_type_t {
  symbol_handler_type_t(ast_node_t::ptr ast = nullptr) : symbol_fundamental_type_t(ast) { }
  virtual int64_t sizeof_() const { return 0; }
  virtual uint64_t signature() const { return knuth(knuth(__LINE__)); }
  virtual symbol_type_t::ptr degrade() const { return std::make_shared<symbol_void_type_t>(); }
  virtual std::string _name() const { return "exception"s; }
  template<class... Args>
  static std::shared_ptr<symbol_handler_type_t> new_(Args&&... args) { return std::make_shared<symbol_handler_type_t>(std::forward<Args>(args)...); }
  virtual ptr intern(bool ex=false) const { auto c = std::make_shared<symbol_handler_type_t>(*this); c->external = ex; return c; }
  virtual ptr constant(bool co=false) const { auto c = std::make_shared<symbol_handler_type_t>(*this); c->const_ = co; return c; }
  symbol_handler_type_t(const symbol_handler_type_t& rhs) : symbol_fundamental_type_t(rhs) { }
};

struct symbol_func_type_t : public symbol_fundamental_type_t {
  bool except;
  std::vector<std::string> args;
  std::string func_name;
  symbol_func_type_t(ast_node_t::ptr ast = nullptr, std::string func_name = "<anonymous-func>"s, std::vector<std::string> args = {}) :
      symbol_fundamental_type_t(ast), func_name(func_name), args(args) { }
  virtual int64_t sizeof_() const { return 8; }
  virtual uint64_t signature() const { uint64_t sig = knuth(knuth(knuth(__LINE__)) ^ knuth(func_name.c_str())); for (auto&& a : args) sig = knuth(sig ^ knuth(a.c_str())); return sig; }
  virtual symbol_type_t::ptr degrade() const { return std::make_shared<symbol_void_type_t>(); }
  virtual std::string _name() const {
    std::stringstream ss;
    ss << (except ? "except:": "func:") << func_name << "(";
    bool first = true;
    for (auto&& p : args) {
      ss << (first ? "" : ",") << p;
      first = false;
    }
    ss << ")";
    return ss.str();
  }
  template<class... Args>
  static std::shared_ptr<symbol_func_type_t> new_(Args&&... args) { return std::make_shared<symbol_func_type_t>(std::forward<Args>(args)...); }
  virtual ptr intern(bool ex=false) const { return nullptr; }
  virtual ptr constant(bool co=false) const { return nullptr; }
  symbol_func_type_t(const symbol_func_type_t& rhs) : symbol_fundamental_type_t(rhs), except(rhs.except), func_name(rhs.func_name), args(rhs.args) { }
};

struct symbol_array_type_t : public symbol_type_t {
  int64_t size;
  symbol_type_t::ptr elem_type;
  symbol_array_type_t(symbol_type_t::ptr elem_type, int64_t size) : symbol_type_t(*std::static_pointer_cast<symbol_type_t>(elem_type)), elem_type(elem_type), size(size) {}
  virtual int64_t sizeof_() const {
    return elem_type->sizeof_() * size;
  }
  virtual symbol_type_t::ptr degrade() const {
    return elem_type;
  }
  virtual uint64_t signature() const {
    return knuth(size ^ knuth(elem_type->signature() ^ knuth(__LINE__)));
  }
  virtual std::string _name() const {
    std::stringstream ss;
    ss << elem_type->_name() << "[" << size << "]";
    return ss.str();
  }
  virtual ptr intern(bool ex=false) const { return nullptr; }
  virtual ptr constant(bool co=false) const { return nullptr; }
};

struct symbol_var_t : public symbol_val_t {
  using ptr = std::shared_ptr<symbol_var_t>;
  std::string name;
  simval_t constval;
  int64_t va;
  symbol_var_t(ast_node_t::ptr ast, symbol_type_t::ptr type, std::string name) : symbol_val_t(ast, type), name(name), va(-1) { }
  virtual bool lvalue() const { return !constval.index(); }
  virtual bool rvvalue() const { return false; }
  virtual simval_t constexpr_eval() const {
    return constval;
  }
};

struct symbol_arglist_t : public symbol_val_t {
  std::vector<symbol_val_t::ptr> args;
  symbol_arglist_t(ast_node_t::ptr ast, symbol_val_t::ptr lhs, symbol_val_t::ptr rhs) : symbol_val_t(ast, rhs->type) {
    auto lhs_list = lhs->to<symbol_arglist_t>();
    if (lhs_list) {
      args = lhs_list->args;
    } else {
      args.push_back(lhs);
    }
    args.push_back(rhs);
  }
  virtual simval_t constexpr_eval() const { return args.back()->constexpr_eval(); }
  virtual bool lvalue() const { return args.back()->lvalue(); }
  virtual bool rvvalue() const { return args.back()->rvvalue(); }
};

struct symbol_call_t : public symbol_val_t {
  bool async;
  symbol_val_t::ptr func;
  std::vector<symbol_val_t::ptr> args;
  std::vector<symbol_val_t::ptr> args_byval;
  symbol_stmt_t::ptr exec;
  symbol_call_t(ast_node_t::ptr ast, bool async, symbol_val_t::ptr func, std::vector<symbol_val_t::ptr>& args, std::vector<symbol_val_t::ptr>& args_byval, symbol_stmt_t::ptr exec) :
      symbol_val_t(ast, async ? static_cast<symbol_type_t::ptr>(func->type->to<symbol_func_type_t>() ? symbol_handler_type_t::new_(ast) : nullptr) : static_cast<symbol_type_t::ptr>(symbol_void_type_t::new_(ast))), async(async),
      func(func), args(args), args_byval(args_byval), exec(exec) { }
  virtual bool rvvalue() const { return async; }
};

struct symbol_func_t : public symbol_val_t {
  std::shared_ptr<compstmt_t> body;
  symbol_func_t(ast_node_t::ptr ast, symbol_func_type_t::ptr type, std::shared_ptr<compstmt_t> body) : symbol_val_t(ast, type), body(body) { }
};

struct symbol_lit_t : public symbol_val_t {
  simval_t value;
  symbol_lit_t(ast_node_t::ptr ast, std::string str) :
      symbol_val_t(ast, symbol_str_type_t::new_(ast)), value(str) { }
  symbol_lit_t(ast_node_t::ptr ast, int64_t int_) :
      symbol_val_t(ast, symbol_int_type_t::new_(ast)), value(int_) { }
  symbol_lit_t(ast_node_t::ptr ast, double float_) :
      symbol_val_t(ast, symbol_float_type_t::new_(ast)), value(float_) { }
  virtual simval_t constexpr_eval() const { return value; }
};

struct symbol_cast_t : public symbol_val_t {
  symbol_val_t::ptr from;
  symbol_cast_t(ast_node_t::ptr ast, symbol_val_t::ptr from, symbol_type_t::ptr to) :
      symbol_val_t(ast, to), from(from) {
    if (to->external && !from->lvalue()) {
      ast->warn() << "cast r-value from intern to extern have no effect. use assign instead to store values back to external storage." << ast->eol();
      to->external = false;
    }
    if (!to->external && from->type->external) {
      if (!from->lvalue()) {
        ast->error() << "cast of extern vector to intern is prohibited. use assign instead." << ast->eol();
      }
    }
  }
  virtual simval_t constexpr_eval() const {
    auto eval = from->constexpr_eval();
    auto int_sig = symbol_int_type_t::new_()->signature();
    auto float_sig = symbol_float_type_t::new_()->signature();
    auto vec_sig = symbol_vec_type_t::new_()->signature();
    switch (eval.index()) {
      case 1:
        if (type->signature() == int_sig) return std::get<int64_t>(eval);
        if (type->signature() == float_sig) return (double)std::get<int64_t>(eval);
        if (type->signature() == vec_sig) {
          float convert = (float)std::get<int64_t>(eval);
          *(uint32_t*)&convert = (uint32_t)0xFFFF0000U & *(uint32_t*)&convert;
          return (double)convert;
        }
      break;
      case 2:
        if (type->signature() == int_sig) return (int64_t)std::get<double>(eval);
        if (type->signature() == float_sig) return std::get<double>(eval);
        if (type->signature() == vec_sig) {
          float convert = (float)std::get<double>(eval);
          *(uint32_t*)&convert = (uint32_t)0xFFFF0000U & *(uint32_t*)&convert;
          return (double)convert;
        }
      break;
      default: break;
    }
    return std::monostate();
  }
  static bool op_accept(symbol_type_t::ptr from, symbol_type_t::ptr to) {
    if (from->signature() == to->signature()) return true; // self-cast / cast of storage.
    if (from is typeid(symbol_vec_type_t) && to is typeid(symbol_vec_type_t)) return true; // reinterpret vector.
    if (from->is_scalar() && to->is_scalar()) return true; // among scalar types.
    return false;
  }
  virtual bool lvalue() const { return from->type is typeid(symbol_vec_type_t) && type is typeid(symbol_vec_type_t) && from->lvalue(); }
  virtual bool rvvalue() const { return from->rvvalue() || (!(from->type is typeid(symbol_vec_type_t)) && type is typeid(symbol_vec_type_t)); }
};

struct symbol_descript_t : public symbol_val_t {
  symbol_val_t::ptr oprand;
  symbol_val_t::ptr subscript;
  symbol_descript_t(ast_node_t::ptr ast, symbol_val_t::ptr oprand, symbol_val_t::ptr subscript) : symbol_val_t(ast, oprand->type->degrade()->intern(oprand->type->external)), oprand(oprand), subscript(subscript) { }
  virtual bool lvalue() const { return oprand->lvalue(); }
};

struct symbol_unary_t : public symbol_val_t {
  symbol_val_t::ptr oprand;
  int opcode;
  enum { POSTINC, POSTDEC, INC, DEC, PROD, ALL, POS, NEG, NOT, REV, ANY, SIZEOF, TYPEOF };
  static bool op_accept(symbol_type_t::ptr oprand, int opcode) {
    switch(opcode) {
      case POSTINC: case POSTDEC: case INC: case DEC: case REV: case NOT:
        return oprand is typeid(symbol_int_type_t);
      case POS:
        return oprand is typeid(symbol_int_type_t) ||
             oprand is typeid(symbol_float_type_t) ||
             !oprand->external && oprand is typeid(symbol_vec_type_t); // reduce_sum on vector, pos on scalar.
      case NEG:
        return oprand->is_scalar() || !(oprand->external && oprand is typeid(symbol_vec_type_t));
      case PROD: case ALL: case ANY:
        return !oprand->external && oprand is typeid(symbol_vec_type_t);
      case SIZEOF: case TYPEOF:
        return true;

    }
    return false;
  }
  static symbol_type_t::ptr op_cast(symbol_type_t::ptr oprand, int opcode) {
    auto ast = null_ast();
    ast->lineno = oprand->lineno; ast->charno = oprand->charno;
    switch(opcode) {
      case POSTINC: case POSTDEC: case INC: case DEC: case REV: case NOT: case SIZEOF:
        return symbol_int_type_t::new_(ast);
      case POS:
      case PROD: case ALL: case ANY:
        if (oprand is typeid(symbol_vec_type_t) && !oprand->is_scalar()) return symbol_float_type_t::new_(ast);
           // fall-through on scalars.
      case NEG:
        return oprand;
      case TYPEOF:
        return symbol_str_type_t::new_(ast);
    }
    return nullptr;
  }
  symbol_unary_t(ast_node_t::ptr ast, symbol_val_t::ptr oprand, int opcode) :
      symbol_val_t(ast, op_accept(oprand->type, opcode) ? op_cast(oprand->type, opcode) : symbol_void_type_t::new_(ast)), oprand(oprand), opcode(opcode) { }
  virtual simval_t constexpr_eval() const {
    if (opcode == SIZEOF) {
      return oprand->type->sizeof_();
    }
    if (opcode == TYPEOF) {
      return oprand->type->name();
    }
    auto orig = symbol_cast_t(null_ast(), oprand, op_cast(oprand->type, opcode)).constexpr_eval();
    if (orig.index() == 1) switch(opcode) {
      case REV: return ~std::get<int64_t>(orig);
      case NOT: return ~std::get<int64_t>(orig);
      case POS: return +std::get<int64_t>(orig);
      case NEG: return -std::get<int64_t>(orig);
    }
    if (orig.index() == 2) switch(opcode) {
      case POS: return +std::get<double>(orig);
      case NEG: return -std::get<double>(orig);
    }
    return std::monostate();
  }
  virtual bool rvvalue() const { return oprand->type is typeid(symbol_vec_type_t) && opcode in std::set<int>{ POS, NEG } && !(type is typeid(symbol_float_type_t)); }
};

struct symbol_binary_t : public symbol_val_t {
  symbol_val_t::ptr lhs;
  symbol_val_t::ptr rhs;
  int opcode;
  enum { MUL, DIV, MOD, ADD, SUB, SHL, SHR, LT, GT, LE, GE, EQ, NE, AND, XOR, OR, LAND, LOR };
  symbol_binary_t(ast_node_t::ptr ast, symbol_val_t::ptr lhs, symbol_val_t::ptr rhs, int opcode) :
      symbol_val_t(ast, op_accept(lhs->type, rhs->type, opcode) ? op_cast(lhs->type, rhs->type, opcode) : symbol_void_type_t::new_(ast)), lhs(lhs), rhs(rhs), opcode(opcode) { } 
  static bool op_accept(symbol_type_t::ptr lhs, symbol_type_t::ptr rhs, int opcode) {
    std::vector<std::vector<int>> acclut {
      { // I-I
         1,   1,   1,   1,   1,   1,   1,   1,  1,  1,  1,  1,  1,  1,   1,   1,  1,    1,
      },
      { // F-F V-F V-V
         1,   1,   0,   1,   1,   0,   0,   1,  1,  1,  1,  1,  1,  0,   0,   0,  0,    0,
      },
      { // F-V
         1,   0,   0,   1,   1,   0,   0,   1,  1,  1,  1,  1,  1,  0,   0,   0,  0,    0,
      },
    };
    if (!lhs->is_scalar() && !(lhs is typeid(symbol_vec_type_t))) return false;
    if (!rhs->is_scalar() && !(rhs is typeid(symbol_vec_type_t))) return false;
    if (lhs->external && lhs is typeid(symbol_vec_type_t) || rhs->external && rhs is typeid(symbol_vec_type_t)) return false;
    if (!(rhs->is_scalar() || lhs->is_scalar()) && lhs->signature() != rhs->signature()) return false;
    if (lhs is typeid(symbol_int_type_t)) return acclut[0][opcode];
    if (lhs->is_scalar() && !rhs->is_scalar()) return acclut[2][opcode];
    return acclut[1][opcode];
  }
  static symbol_type_t::ptr op_cast(symbol_type_t::ptr lhs, symbol_type_t::ptr rhs, int opcode) {
    auto last = null_ast(); auto rast = null_ast();
    last->lineno = lhs->lineno; rast->lineno = rhs->lineno;
    last->charno = lhs->charno; rast->charno = rhs->charno;
    if (lhs is typeid(symbol_vec_type_t)) return lhs;
    if (rhs is typeid(symbol_vec_type_t)) return rhs;
    if (lhs is typeid(symbol_float_type_t)) return symbol_float_type_t::new_(last);
    if (rhs is typeid(symbol_float_type_t)) return symbol_float_type_t::new_(rast);
    return symbol_int_type_t::new_(last);
  }
  virtual simval_t constexpr_eval() const {
    auto op = [&](auto x, auto y, auto& z) { switch(opcode) {
      case MUL:  z = x * y;  break;
      case DIV:  z = x / y;  break;
      case MOD:  z = (int64_t)x % (int64_t)y;  break;
      case ADD:  z = x + y;  break; 
      case SUB:  z = x - y;  break; 
      case SHL:  z = (int64_t)x << (int64_t)y; break; 
      case SHR:  z = (int64_t)x >> (int64_t)y; break; 
      case LT:   z = x < y;  break; 
      case GT:   z = x > y;  break; 
      case LE:   z = x <= y; break; 
      case GE:   z = x >= y; break; 
      case EQ:   z = x == y; break; 
      case NE:   z = x != y; break; 
      case AND:  z = (int64_t)x & (int64_t)y;  break; 
      case XOR:  z = (int64_t)x ^ (int64_t)y;  break; 
      case OR:   z = (int64_t)x | (int64_t)y;  break; 
      case LAND: z = (int64_t)x && (int64_t)y; break; 
      case LOR:  z = (int64_t)x || (int64_t)y; break; 
    } return 0; };
    auto lv = lhs->constexpr_eval();
    auto rv = rhs->constexpr_eval();
    if (lv.index() == 1 && rv.index() == 1)
      { int64_t z; op(std::get<int64_t>(lv), std::get<int64_t>(rv), z); return z; }
    if (lv.index() == 2 && rv.index() == 2)
      { double z; op(std::get<double>(lv), std::get<double>(rv), z); return z; }
    return std::monostate();
  }
  virtual bool rvvalue() const { return type is typeid(symbol_vec_type_t); }
};

struct symbol_cond_t : public symbol_val_t {
  symbol_val_t::ptr cond;
  symbol_val_t::ptr lhs;
  symbol_val_t::ptr rhs;
  symbol_cond_t(ast_node_t::ptr ast, symbol_val_t::ptr cond, symbol_val_t::ptr lhs, symbol_val_t::ptr rhs) : symbol_val_t(ast, lhs->type), cond(cond), lhs(lhs), rhs(rhs) { }
  virtual simval_t constexpr_eval() const {
    auto vc = cond->constexpr_eval();
    if (vc.index() == 1) {
      if (std::get<int64_t>(vc) == 0) return rhs->constexpr_eval();
      else return lhs->constexpr_eval();
    } else if (vc.index() == 2) {
      if (std::get<double>(vc) == 0.) return rhs->constexpr_eval();
      else return lhs->constexpr_eval();
    }
    return std::monostate();
  }
  virtual bool rvvalue() const { return lhs->rvvalue() || rhs->rvvalue(); }
};

struct symbol_assign_t : public symbol_val_t {
  symbol_val_t::ptr lhs;
  symbol_val_t::ptr rhs;
  symbol_assign_t(ast_node_t::ptr ast, symbol_val_t::ptr lhs, symbol_val_t::ptr rhs) : symbol_val_t(ast, rhs->type), lhs(lhs), rhs(rhs) { }
};

struct symbol_print_t : public symbol_stmt_t {
  std::vector<symbol_val_t::ptr> objs;
  symbol_print_t(ast_node_t::ptr ast, symbol_val_t::ptr obj) : symbol_stmt_t(ast), objs(obj->to<symbol_arglist_t>() ? obj->to<symbol_arglist_t>()->args : std::vector<symbol_val_t::ptr>{ obj }) { }
};

struct symbol_alloc_t : public symbol_stmt_t {
  symbol_var_t::ptr var;
  symbol_alloc_t(ast_node_t::ptr ast, symbol_var_t::ptr var) : symbol_stmt_t(ast), var(var) { }
};

struct symbol_free_t : public symbol_stmt_t {
  symbol_var_t::ptr var;
  symbol_free_t(ast_node_t::ptr ast, symbol_var_t::ptr var) : symbol_stmt_t(ast), var(var) { }
};

struct symbol_stmt_list_t : public symbol_stmt_t {
  std::list<symbol_stmt_t::ptr> list;
  symbol_stmt_list_t(ast_node_t::ptr ast) : symbol_stmt_t(ast) { }
};

struct symbol_eval_t : public symbol_stmt_t {
  symbol_val_t::ptr expr;
  symbol_eval_t(ast_node_t::ptr ast, symbol_val_t::ptr expr) : symbol_stmt_t(ast), expr(expr) { }
};

struct symbol_break_t : public symbol_stmt_t {
  symbol_val_t::ptr cond;
  symbol_break_t(ast_node_t::ptr ast, symbol_val_t::ptr cond=nullptr) : symbol_stmt_t(ast), cond(cond) { }
};

struct symbol_continue_t : public symbol_stmt_t {
  symbol_val_t::ptr cond;
  symbol_continue_t(ast_node_t::ptr ast, symbol_val_t::ptr cond=nullptr) : symbol_stmt_t(ast), cond(cond) { }
};

struct symbol_return_t : public symbol_stmt_t {
  symbol_return_t(ast_node_t::ptr ast) : symbol_stmt_t(ast) { }
};

struct symbol_await_t : public symbol_stmt_t {
  std::shared_ptr<symbol_call_t> handler;
  symbol_await_t(ast_node_t::ptr ast, std::shared_ptr<symbol_call_t> handler) : symbol_stmt_t(ast), handler(handler) { }
};

struct symbol_loop_t : public symbol_stmt_t {
  symbol_stmt_t::ptr init;
  symbol_stmt_t::ptr prelog;
  symbol_stmt_t::ptr body;
  symbol_stmt_t::ptr epilog;
  symbol_loop_t(ast_node_t::ptr ast, symbol_stmt_t::ptr init, symbol_stmt_t::ptr prelog,
      symbol_stmt_t::ptr body, symbol_stmt_t::ptr epilog) :
      symbol_stmt_t(ast), init(init), prelog(prelog), body(body), epilog(epilog) { }
};

struct symbol_branch_t : public symbol_stmt_t {
  symbol_val_t::ptr cond;
  symbol_stmt_t::ptr pos;
  symbol_stmt_t::ptr neg;
  symbol_branch_t(ast_node_t::ptr ast, symbol_val_t::ptr cond, symbol_stmt_t::ptr pos, symbol_stmt_t::ptr neg) :
      symbol_stmt_t(ast), cond(cond), pos(pos), neg(neg) { }
};

struct symbol_conv_t : public symbol_stmt_t {
  symbol_val_t::ptr result;
  symbol_val_t::ptr weight;
  symbol_val_t::ptr input;
  int64_t stride_x;
  int64_t stride_y;
  int64_t pad_x;
  int64_t pad_y;
  symbol_conv_t(ast_node_t::ptr ast, symbol_val_t::ptr result, symbol_val_t::ptr weight, symbol_val_t::ptr input, int64_t stride_x = 1, int64_t stride_y = 1, int64_t pad_x = 0, int64_t pad_y = 0)
      : symbol_stmt_t(ast), result(result), weight(weight), input(input), stride_x(stride_x), stride_y(stride_y), pad_x(pad_x), pad_y(pad_y) { }
};

struct symbol_pool_t : public symbol_stmt_t {
  symbol_val_t::ptr result;
  symbol_val_t::ptr input;
  int64_t kernel_x;
  int64_t kernel_y;
  int64_t stride_x;
  int64_t stride_y;
  int64_t pad_x;
  int64_t pad_y;
  symbol_pool_t(ast_node_t::ptr ast, symbol_val_t::ptr result, symbol_val_t::ptr input, int64_t kernel_x, int64_t kernel_y, int64_t stride_x, int64_t stride_y, int64_t pad_x = 0, int64_t pad_y = 0)
      : symbol_stmt_t(ast), result(result), input(input), kernel_x(kernel_x), kernel_y(kernel_y), stride_x(stride_x), stride_y(stride_y), pad_x(pad_x), pad_y(pad_y) { }
};

struct symbol_mm_t : public symbol_stmt_t {
  symbol_val_t::ptr result;
  symbol_val_t::ptr weight;
  symbol_val_t::ptr input;
  bool tr;
  symbol_mm_t(ast_node_t::ptr ast, symbol_val_t::ptr result, symbol_val_t::ptr weight, symbol_val_t::ptr input, bool tr) : symbol_stmt_t(ast), result(result), weight(weight), input(input), tr(tr){ }
};

struct symbol_act_t : public symbol_stmt_t {
  symbol_val_t::ptr result;
  symbol_val_t::ptr input;
  symbol_act_t(ast_node_t::ptr ast, symbol_val_t::ptr result, symbol_val_t::ptr input) : symbol_stmt_t(ast), result(result), input(input) { }
};

struct symbol_trans_t : public symbol_stmt_t {
  symbol_val_t::ptr result;
  symbol_val_t::ptr input;
  symbol_trans_t(ast_node_t::ptr ast, symbol_val_t::ptr result, symbol_val_t::ptr input) : symbol_stmt_t(ast), result(result), input(input) { }
};

struct symbol_cycleadd_t : public symbol_stmt_t {
  symbol_val_t::ptr result;
  symbol_val_t::ptr bigger;
  symbol_val_t::ptr smaller;
  symbol_cycleadd_t(ast_node_t::ptr ast, symbol_val_t::ptr result, symbol_val_t::ptr bigger, symbol_val_t::ptr smaller) : symbol_stmt_t(ast), result(result), bigger(bigger), smaller(smaller) { }
};

struct symbol_builtin_conv_t : public symbol_stmt_t {
  int operator_type;
  enum { CONV, MLP, GROUP_CONV, DEPTHWISE_CONV };
  symbol_val_t::ptr dest;
  symbol_val_t::ptr weight;
  symbol_val_t::ptr input;
  symbol_val_t::ptr bias;
  std::vector<symbol_val_t::ptr> group; // split dest, concat source, group conv kernel.
  int64_t bt;
  int64_t fi;
  int64_t fo;
  int64_t kx;
  int64_t ky;
  int64_t xi;
  int64_t yi;
  int64_t sx;
  int64_t sy;
  int64_t px;
  int64_t py;
  symbol_builtin_conv_t(ast_node_t::ptr ast) : symbol_stmt_t(ast) { }
};

struct symbol_builtin_pool_t : public symbol_stmt_t {
  int operation_type;
  enum { POOL, };
  symbol_val_t::ptr dest;
  symbol_val_t::ptr input;
  int64_t bt;
  int64_t fi;
  int64_t fo;
  int64_t kx;
  int64_t ky;
  int64_t xi;
  int64_t yi;
  int64_t sx;
  int64_t sy;
  int64_t px;
  int64_t py;
  symbol_builtin_pool_t(ast_node_t::ptr ast) : symbol_stmt_t(ast) { }
};

struct symbol_builtin_concatsplit_t : public symbol_stmt_t {
  int operation_type;
  enum { CONCAT, SPLIT };
  std::vector<symbol_val_t::ptr> group;
  symbol_val_t::ptr other;
  int64_t bt;
  symbol_builtin_concatsplit_t(ast_node_t::ptr ast) : symbol_stmt_t(ast) { }
};

struct symbol_enscope_t : public symbol_stmt_t {
  symbol_enscope_t(ast_node_t::ptr ast) : symbol_stmt_t(ast) { }
};

struct symbol_descope_t : public symbol_stmt_t {
  symbol_descope_t(ast_node_t::ptr ast) : symbol_stmt_t(ast) { }
};

std::list<std::unordered_map<std::string, symbol_t::ptr>> symbol_registry;
#define scoped_lookup(name) [&]()->symbol_t::ptr{ for (auto&& scope : symbol_registry) if (name in scope) return scope[name]; return nullptr; }()

template<class T, class R, class... Args>
R typeswitch(std::shared_ptr<T> t, R(*...args)(std::shared_ptr<Args>)) {
  std::unordered_map<std::type_index, std::function<R(std::shared_ptr<T>)>> _case_map {
    { std::type_index(typeid(Args)),
    std::function<R(std::shared_ptr<T>)>([=](std::shared_ptr<T> fwd){return (*args)(std::dynamic_pointer_cast<Args>(fwd));}) }...
  };
  return (_case_map[std::type_index(typeid(*t))])(t);
}

symbol_t::ptr prob(std::shared_ptr<ast_node_t> ast);
using oplut_t = std::vector<std::tuple<int, std::string, bool>>;
template<class T>
symbol_t::ptr symbol_case_binary_operators(std::shared_ptr<T> ast,
     oplut_t oplut) {
      auto lhs = std::dynamic_pointer_cast<symbol_val_t>(prob(ast->lhs));
      auto rhs = std::dynamic_pointer_cast<symbol_val_t>(prob(ast->rhs));
      auto lt = lhs->type;
      auto rt = rhs->type;
      int opcode = std::get<0>(oplut[ast->type]);
      auto opr_check = [ast](symbol_val_t::ptr opr) {
        if (!opr->type->is_scalar() && !(opr->type is typeid(symbol_vec_type_t))) {
          ast->error() << "expect int/float/vector values in operands, got non-value type " << opr->type->name() << "." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        }
        if (opr->type is typeid(symbol_vec_type_t) and opr->type->external) {
          ast->error() << "extern vector must be explicitly moved to intern variables before participating operations." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        }
        return 0;
      };
      opr_check(lhs), opr_check(rhs);
      if (lhs->rvvalue() && rhs->rvvalue()) {
        ast->error() << "binary operands cannot be both rv-value. assign one of them to lvalue first." << ast->eol();
        lhs->type->note() << "type defined from here:" << lhs->type->eol();
        rhs->type->note() << "type defined from here:" << rhs->type->eol();
      }
      if (lhs->type->signature() != rhs->type->signature()) {
        if (lhs->type->signature() == symbol_vec_type_t::new_()->signature())
          lhs = std::make_shared<symbol_cast_t>(ast, lhs, symbol_float_type_t::new_(ast));
        if (rhs->type->signature() == symbol_vec_type_t::new_()->signature())
          rhs = std::make_shared<symbol_cast_t>(ast, rhs, symbol_float_type_t::new_(ast));
        if (rhs->type->is_scalar() && lhs->type is typeid(symbol_vec_type_t))
          rhs = std::make_shared<symbol_cast_t>(ast, rhs, symbol_float_type_t::new_(ast));
        if (lhs->type->is_scalar() && rhs->type is typeid(symbol_vec_type_t))
          lhs = std::make_shared<symbol_cast_t>(ast, lhs, symbol_float_type_t::new_(ast));
        if (lhs->type is typeid(symbol_vec_type_t) && rhs->type is typeid(symbol_vec_type_t)
          && lhs->type->signature() != rhs->type->signature()) {
          ast->error() << "binary operator cannot operate on two different sized vectors." << ast->eol();
          lhs->type->note() << "type defined from here:" << lhs->type->eol();
          rhs->type->note() << "type defined from here:" << rhs->type->eol();
        }
        if ((lhs->type is typeid(symbol_float_type_t) || lhs->type is typeid(symbol_vec_type_t)) && rhs->type is typeid(symbol_int_type_t))
          rhs = std::make_shared<symbol_cast_t>(ast, rhs, lhs->type);
        else if ((rhs->type is typeid(symbol_float_type_t) || rhs->type is typeid(symbol_vec_type_t)) && lhs->type is typeid(symbol_int_type_t))
          lhs = std::make_shared<symbol_cast_t>(ast, lhs, rhs->type); 
      }
      if (!std::get<2>(oplut[ast->type])) {
        ast->error() << "binary operator " << std::get<1>(oplut[ast->type]) << " is not implemented." << ast->eol();
      } else if (!symbol_binary_t::op_accept(lhs->type, rhs->type, opcode)) {
        ast->error() << "binary operator " << std::get<1>(oplut[ast->type]) << " does not accept operand of type "
                     << lhs->type->name() << (lhs->type->signature() != lt->signature() ? "(implicitly promoted from "s + lt->name() + ")"s : "") << " and "
                     << rhs->type->name() << (rhs->type->signature() != rt->signature() ? "(implicitly promoted from "s + rt->name() + ")"s : "") << "." << ast->eol();
        lt->note() << "left-hand-side operand type defined from here:" << lt->eol();
        rt->note() << "right-hand-side operand type defined from here:" << rt->eol();
      }
      return std::make_shared<symbol_binary_t>(ast, lhs, rhs, opcode); 
}

template<class T>
symbol_t::ptr symbol_case_assign(std::shared_ptr<T> ast, symbol_t::ptr _rhs) {
    auto lhs = std::dynamic_pointer_cast<symbol_val_t>(prob(ast->lhs));
    auto rhs = std::dynamic_pointer_cast<symbol_val_t>(_rhs);
    if (lhs->rvvalue()) {
      ast->error() << "assignment destination do not accept rv-value." << ast->eol();
      lhs->type->note() << "type defined from here:" << lhs->type->eol();
    }
    if (lhs->type->external && rhs->rvvalue()) {
      ast->error() << "cannot assign rv-value to extern directly. assign to intern first." << ast->eol();
      lhs->type->note() << "type defined from here:" << lhs->type->eol();
    }
    if (!lhs->type->is_scalar() && !(lhs->type is typeid(symbol_vec_type_t))) {
      ast->error() << "expect int/float/vector values in operands, got non-value type " << lhs->type->name() << "." << ast->eol();
      lhs->type->note() << "type defined from here:" << lhs->type->eol();
    }
    if (!rhs->type->is_scalar() && !(rhs->type is typeid(symbol_vec_type_t))) {
      ast->error() << "expect int/float/vector values in operands, got non-value type " << rhs->type->name() << "." << ast->eol();
      rhs->type->note() << "type defined from here:" << rhs->type->eol();
    }
    if (lhs->type is typeid(symbol_vec_type_t) && lhs->type->external && rhs->type is typeid(symbol_vec_type_t) && rhs->type->external) {
      ast->error() << "extern vector must be explicitly moved to intern variables before participating operations." << ast->eol();
      rhs->type->note() << "type defined from here:" << rhs->type->eol();
    }
    if (!lhs->lvalue()) {
      ast->error() << "assignment destination must be lvalue." << ast->eol();
    }
    if (lhs->type->signature() != rhs->type->signature() && !symbol_cast_t::op_accept(rhs->type, lhs->type)) {
      ast->error() << "cannot cast from type " << rhs->type->name() << " to " << lhs->type->name() << "." << ast->eol();
    }
    return std::make_shared<symbol_assign_t>(ast, lhs,
        rhs->type->signature() != lhs->type->signature() ? std::make_shared<symbol_cast_t>(ast, rhs, lhs->type) : rhs);
}

bool is_in_except = false;

struct function_instantiation_frame_t : public cursor_t {
  std::shared_ptr<symbol_func_type_t> func_type;
  std::vector<symbol_val_t::ptr> args;
  function_instantiation_frame_t(std::shared_ptr<cursor_t> cur, std::shared_ptr<symbol_func_type_t> func_type, std::vector<symbol_val_t::ptr> args) : cursor_t(*cur), func_type(func_type), args(args) { }
};

std::list<function_instantiation_frame_t> function_instantiation_stack;
void rewind_function_instantiation_stack() {
  for (auto&& f : function_instantiation_stack) {
    std::stringstream ss;
    bool first = true;
    ss << "[";
    for (auto&& a : f.args) {
      ss << (first ? ""s : ", "s) << a->type->name() << (a->lvalue() ? "&"s : ""s);
      first = false;
    }
    ss << "]";
    f.note() << "in instantiation of " << f.func_type->name() << " with arguments " << ss.str() << f.eol();
  }
}

symbol_t::ptr prob(std::shared_ptr<ast_node_t> ast) {
  verbose( 3, null_ast()->note() << "prob symbol tree of " << (ast ? typeid(*ast).name() : "nullptr") << (ast ? ast : null_ast())->eol() );
  if (!ast) return std::make_shared<symbol_null_t>(null_ast(), ""s);
  return typeswitch(ast, 

    +[](std::shared_ptr<ident_t> ast)->symbol_t::ptr {
      auto lu = scoped_lookup(ast->name);
      if (lu) return lu;
      return std::make_shared<symbol_null_t>(ast, ast->name);
    },

    +[](std::shared_ptr<sexpr_t> ast)->symbol_t::ptr {
      return std::make_shared<symbol_lit_t>(ast, ast->lit);
    },

    +[](std::shared_ptr<vexpr_t> ast)->symbol_t::ptr {
      auto v = prob(ast->var);
      if (v is typeid(symbol_null_t)) {
        v->error() << "\"" << ast->var->name << "\" is not defined." << v->eol();
        return std::make_shared<symbol_var_t>(ast, symbol_void_type_t::new_(ast), ast->var->name); // resume from void
      }
      return v;
    },

    +[](std::shared_ptr<iexpr_t> ast)->symbol_t::ptr {
      return std::make_shared<symbol_lit_t>(ast, ast->value);
    },

    +[](std::shared_ptr<fexpr_t> ast)->symbol_t::ptr {
      return std::make_shared<symbol_lit_t>(ast, ast->value);
    },

    +[](std::shared_ptr<pexpr_t> ast)->symbol_t::ptr {
      if (ast->s) return prob(ast->s);
      if (ast->v) return prob(ast->v);
      if (ast->i) return prob(ast->i);
      if (ast->f) return prob(ast->f);
      return prob(ast->e);
    },

    +[](std::shared_ptr<postfixexpr_t> ast)->symbol_t::ptr {
      if (ast->desc) return prob(ast->desc);
      if (ast->call) return prob(ast->call);
      if (ast->postfix) return prob(ast->postfix);
      return prob(ast->primary);
    },

    +[](std::shared_ptr<descexpr_t> ast)->symbol_t::ptr {
      auto lhs = prob(ast->lhs)->to<symbol_val_t>();
      auto desc = prob(ast->desc)->to<symbol_val_t>();
      auto rvck = [&](symbol_val_t::ptr opr) {
        if (opr->rvvalue()) {
          ast->error() << "expression do not accept rv-value. convert to lvalue by assignment first." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        } return 0;
      };
      rvck(lhs);
      if (lhs->type->degrade() is typeid(symbol_void_type_t)) {
        ast->error() << "operand of type " << lhs->type->name() << " is not subscriptable." << ast->eol();
        lhs->type->note() << "type defined from here:" << lhs->type->eol();
      }
      if (!(desc->type is typeid(symbol_int_type_t))) {
        ast->error() << "subscript must be an int, got " << desc->type->name() << "." << ast->eol();
        desc->type->note() << "type defined from here:" << desc->type->eol();
      }
      return std::make_shared<symbol_descript_t>(ast, lhs, desc);
    },

    +[](std::shared_ptr<callexpr_t> ast)->symbol_t::ptr {
      bool async = ast->async;
      if (async && is_in_except) {
        ast->error() << "cannot make async function calls from another async function call." << ast->eol();
        return std::make_shared<symbol_null_t>(ast, "<invalid-async-call>");
      }
      auto lhs = prob(ast->lhs)->to<symbol_func_t>();
      auto func = lhs ? lhs->type->to<symbol_func_type_t>() : nullptr;
      auto arg = prob(ast->args)->to<symbol_val_t>();
      auto args = arg->to<symbol_arglist_t>();
      std::vector<symbol_val_t::ptr> vargs;
      std::vector<symbol_val_t::ptr> vargs_byval;
      auto stmts = std::make_shared<symbol_stmt_list_t>(ast);
      auto rvck = [&](symbol_val_t::ptr opr) {
        if (opr->rvvalue()) {
          ast->error() << "expression do not accept rv-value. convert to lvalue by assignment first." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        } return 0;
      };
      if (args) {
        vargs = args->args;
      } else if (arg) {
        vargs.push_back(arg);
      }
      if (!func) {
        ast->error() << "callee must be a function." << ast->eol();
      } else {
        if (vargs.size() != func->args.size()) {
          vargs.back()->error() << "expect exactly " << func->args.size() << " args, got " << vargs.size() << vargs.back()->eol();
          func->note() << "function defined from here:" << func->eol();
        }
        symbol_registry.emplace_front(); // emplace new symbol scope, but alloc at origin scope in codegen.
        for (size_t i = 0; i < std::min(vargs.size(), func->args.size()); i++) {
          rvck(vargs[i]);
          symbol_val_t::ptr param = vargs[i];
          if (param->constexpr_eval().index()) {
          } else if (!param->lvalue()) {
            param = std::make_shared<symbol_var_t>(ast, param->type, func->args[i]);
            stmts->list.push_back(std::make_shared<symbol_alloc_t>(ast, param->to<symbol_var_t>()));
            auto assign = std::make_shared<fake_assign_t>();
            assign->lhs = std::make_shared<ident_t>();
            assign->lhs->name = func->args[i];
            symbol_registry.emplace_front(); // cheat the assign case.
            symbol_registry.front()[func->args[i]] = param;
            stmts->list.push_back(std::make_shared<symbol_eval_t>(ast, symbol_case_assign(assign, vargs[i])->to<symbol_val_t>()));
            symbol_registry.pop_front();
            vargs_byval.push_back(param);
          }
          symbol_registry.front()[func->args[i]] = param;
        }
        function_instantiation_stack.emplace_back(ast, func, vargs);
        if (async) is_in_except = true;
        stmts->list.push_back(prob(lhs->body)->to<symbol_stmt_t>());
        if (async) is_in_except = false;
        function_instantiation_stack.pop_back();
        if (!async) {
          for (auto&& v : vargs_byval)
            stmts->list.push_back(std::make_shared<symbol_free_t>(ast, v->to<symbol_var_t>()));
        }
        symbol_registry.pop_front();
      }

      return std::make_shared<symbol_call_t>(ast, async, lhs, vargs, vargs_byval, stmts);
    },

    +[](std::shared_ptr<postfix_t> ast)->symbol_t::ptr {
      auto lhs = prob(ast->lhs)->to<symbol_val_t>();
      std::vector<std::pair<int, std::string>> oplut { { symbol_unary_t::POSTINC, "++"s }, { symbol_unary_t::POSTDEC, "--"s } };
      int opcode = oplut[ast->type].first;
      if (!symbol_unary_t::op_accept(lhs->type, opcode)) {
        ast->error() << "postfix unary operator " << oplut[ast->type].second << " does not accept operand of type " << lhs->type->name() << "." << ast->eol();
        lhs->type->note() << "type defined from here:" << lhs->type->eol();
      }
      return std::make_shared<symbol_unary_t>(ast, lhs, opcode);
    },

    +[](std::shared_ptr<unaryexpr_t> ast)->symbol_t::ptr {
      if (ast->unary) return prob(ast->unary);
      return prob(ast->postfix);
    },

    +[](std::shared_ptr<unary_t> ast)->symbol_t::ptr {
      auto lhs = prob(ast->lhs)->to<symbol_val_t>();
      std::vector<std::tuple<int, std::string, bool>> oplut {
        { symbol_unary_t::INC, "++"s, true },
        { symbol_unary_t::DEC, "--"s, true },
        { symbol_unary_t::PROD, "*"s, true },
        { symbol_unary_t::ALL, "&"s, true },
        { symbol_unary_t::POS, "+"s, true },
        { symbol_unary_t::NEG, "-"s, true },
        { symbol_unary_t::NOT, "!"s, true },
        { symbol_unary_t::REV, "~"s, true },
        { symbol_unary_t::ANY, "|"s, true },
        { symbol_unary_t::SIZEOF, "sizeof"s, true },
        { symbol_unary_t::TYPEOF, "typeof"s, true },
      };
      int opcode = std::get<0>(oplut[ast->type]);
      if (!std::get<2>(oplut[ast->type])) {
        ast->error() << "prefix unary operator " << std::get<1>(oplut[ast->type]) << " is not implemented." << ast->eol();
      } else if (!symbol_unary_t::op_accept(lhs->type, opcode)) {
        ast->error() << "prefix unary operator " << std::get<1>(oplut[ast->type]) << " does not accept operand of type " << lhs->type->name() << "." << ast->eol();
        lhs->type->note() << "type defined from here:" << lhs->type->eol();
      }
      if (lhs->type->external && lhs->type is typeid(symbol_vec_type_t)) {
        ast->error() << "extern vector must be explicitly moved to intern variables before participating operations." << ast->eol();
        lhs->type->note() << "type defined from here:" << lhs->type->eol();
      }
      auto rvck = [&](symbol_val_t::ptr opr) {
        if (opr->rvvalue()) {
          ast->error() << "expression do not accept rv-value. convert to lvalue by assignment first." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        } return 0;
      };
      if (opcode in std::set<int>{symbol_unary_t::PROD, symbol_unary_t::ALL,
                                  symbol_unary_t::POS, symbol_unary_t::ANY})
        rvck(lhs);
      return std::make_shared<symbol_unary_t>(ast, lhs, opcode);
    },

    +[](std::shared_ptr<castexpr_t> ast)->symbol_t::ptr {
      if (ast->cast) return prob(ast->cast);
      return prob(ast->unary);
    },

    +[](std::shared_ptr<cast_t> ast)->symbol_t::ptr {
      auto type = prob(ast->type)->to<symbol_type_t>();
      auto lhs = prob(ast->lhs)->to<symbol_val_t>();
      if (lhs->type->external && lhs->type is typeid(symbol_vec_type_t) && !lhs->lvalue()) {
        ast->error() << "extern vector must be explicitly moved to intern variables before participating operations." << ast->eol();
        lhs->type->note() << "type defined from here:" << lhs->type->eol();
      }
      if (!symbol_cast_t::op_accept(lhs->type, type)) {
        ast->error() << "cannot cast from type " << lhs->type->name() << " to " << type->name() << "." << ast->eol();
      }
      return std::make_shared<symbol_cast_t>(ast, lhs, type);
    },

    +[](std::shared_ptr<mulexpr_t> ast)->symbol_t::ptr {
      if (ast->mul) return prob(ast->mul);
      return prob(ast->cast);
    },

    +[](std::shared_ptr<mul_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::MUL, "*"s, true },
        { symbol_binary_t::DIV, "/"s, true },
        { symbol_binary_t::MOD, "%"s, true },
      }));
    },

    +[](std::shared_ptr<addexpr_t> ast)->symbol_t::ptr {
      return ast->add ? prob(ast->add) : prob(ast->mul);
    },

    +[](std::shared_ptr<add_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::ADD, "+"s, true },
        { symbol_binary_t::SUB, "-"s, true },
      }));
    },

    +[](std::shared_ptr<shiftexpr_t> ast)->symbol_t::ptr {
      return ast->shift ? prob(ast->shift) : prob(ast->add);
    },

    +[](std::shared_ptr<shift_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::SHL, "<<"s, true },
        { symbol_binary_t::SHR, ">>"s, true },
      }));
    },

    +[](std::shared_ptr<relationalexpr_t> ast)->symbol_t::ptr {
      return ast->relational ? prob(ast->relational) : prob(ast->shift);
    },

    +[](std::shared_ptr<relational_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::LT, "<"s, true },
        { symbol_binary_t::GT, ">"s, true },
        { symbol_binary_t::LE, "<="s, true },
        { symbol_binary_t::GE, ">="s, true },
      }));
    },

    +[](std::shared_ptr<equalityexpr_t> ast)->symbol_t::ptr {
      return ast->equality ? prob(ast->equality) : prob(ast->relational);
    },

    +[](std::shared_ptr<equality_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::EQ, "=="s, true },
        { symbol_binary_t::NE, "!="s, true },
      }));
    },

    +[](std::shared_ptr<andexpr_t> ast)->symbol_t::ptr {
      return ast->and_ ? prob(ast->and_) : prob(ast->equality);
    },

    +[](std::shared_ptr<and_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::AND, "&"s, true },
      }));
    },

    +[](std::shared_ptr<xorexpr_t> ast)->symbol_t::ptr {
      return ast->xor_ ? prob(ast->xor_) : prob(ast->and_);
    },

    +[](std::shared_ptr<xor_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::XOR, "^"s, true },
      }));
    },

    +[](std::shared_ptr<orexpr_t> ast)->symbol_t::ptr {
      return ast->or_ ? prob(ast->or_) : prob(ast->xor_);
    },

    +[](std::shared_ptr<or_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::OR, "|"s, true },
      }));
    },

    +[](std::shared_ptr<landexpr_t> ast)->symbol_t::ptr {
      return ast->land ? prob(ast->land) : prob(ast->or_);
    },

    +[](std::shared_ptr<land_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::LAND, "&&"s, true },
      }));
    },

    +[](std::shared_ptr<lorexpr_t> ast)->symbol_t::ptr {
      return ast->lor ? prob(ast->lor) : prob(ast->land);
    },

    +[](std::shared_ptr<lor_t> ast)->symbol_t::ptr {
      return symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::LOR, "||"s, true },
      }));
    },

    +[](std::shared_ptr<condexpr_t> ast)->symbol_t::ptr {
      return ast->cond ? prob(ast->cond) : prob(ast->lor);
    },

    +[](std::shared_ptr<cond_t> ast)->symbol_t::ptr {
      auto cond = prob(ast->cond)->to<symbol_val_t>();
      auto lhs = prob(ast->lhs)->to<symbol_val_t>();
      auto rhs = prob(ast->rhs)->to<symbol_val_t>();
      auto lt = lhs->type;
      auto rt = rhs->type;
      auto rvck = [&](symbol_val_t::ptr opr) {
        if (opr->rvvalue()) {
          ast->error() << "expression do not accept rv-value. convert to lvalue by assignment first." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        } return 0;
      };
      rvck(cond);
      if (!cond->type->is_scalar()) {
        cond->error() << "condition must be a scalar, got " << cond->type->name() << "." << cond->eol();
      }
      auto op_check = [](symbol_val_t::ptr opr) {
        if (!opr->type->is_scalar() && !(opr->type is typeid(symbol_vec_type_t))) {
          opr->error() << "expect value in operands, got non-value type " << opr->type->name() << "." << opr->eol();
        }
        if (opr->type is typeid(symbol_vec_type_t) && opr->type->external) {
          opr->error() << "extern vector must be explicitly moved to intern variables before participating operations." << opr->eol();
        }
      };
      op_check(cond), op_check(lhs), op_check(rhs);
      if (lhs->type->signature() != rhs->type->signature()) {
        if (lhs->type->signature() == symbol_vec_type_t::new_()->signature())
          lhs = std::make_shared<symbol_cast_t>(ast, lhs, symbol_float_type_t::new_(ast));
        if (rhs->type->signature() == symbol_vec_type_t::new_()->signature())
          rhs = std::make_shared<symbol_cast_t>(ast, rhs, symbol_float_type_t::new_(ast));
        if (lhs->type is typeid(symbol_vec_type_t))
          rhs = std::make_shared<symbol_cast_t>(ast, rhs, symbol_float_type_t::new_(ast));
        else if (rhs->type is typeid(symbol_vec_type_t))
          lhs = std::make_shared<symbol_cast_t>(ast, lhs, symbol_float_type_t::new_(ast));
        if (lhs->type is typeid(symbol_float_type_t) && rhs->type is typeid(symbol_int_type_t))
          rhs = std::make_shared<symbol_cast_t>(ast, rhs, lhs->type);
        else if (rhs->type is typeid(symbol_float_type_t) && lhs->type is typeid(symbol_int_type_t))
          lhs = std::make_shared<symbol_cast_t>(ast, lhs, rhs->type); 
      }
      if (lhs->type->signature() != rhs->type->signature()) {
        ast->error() << "failed to predicate result type of conditional expression. got different types of operands." << ast->eol();
        lhs->note() << "left-hand-side operand: " << lhs->type->name()
         << (lt->signature() != lhs->type->signature() ? " (implicitly promoted from "s + lt->name() + ")"s : "")
         << lhs->eol();
        lt->note() << "defined from here:" << lt->eol();
        rhs->note() << "right-hand-side operand: " << rhs->type->name()
         << (rt->signature() != rhs->type->signature() ? " (implicitly promoted from "s + rt->name() + ")"s : "")
         << rhs->eol();
        rt->note() << "defined from here:" << rt->eol();
      }
      return std::make_shared<symbol_cond_t>(ast,
               std::make_shared<symbol_cast_t>(ast, cond, symbol_int_type_t::new_(ast)),
             lhs, rhs);
    },

    +[](std::shared_ptr<assignment_t> ast)->symbol_t::ptr {
      return symbol_case_assign(ast, ast->type ? symbol_case_binary_operators(ast, oplut_t({
        { symbol_binary_t::ADD, "="s, false },
        { symbol_binary_t::ADD, "+="s, true },
        { symbol_binary_t::SUB, "-="s, true },
        { symbol_binary_t::SHL, "<<="s, true },
        { symbol_binary_t::SHR, ">>="s, true },
        { symbol_binary_t::MUL, "*="s, true },
        { symbol_binary_t::DIV, "/="s, true },
        { symbol_binary_t::MOD, "%="s, true },
        { symbol_binary_t::AND, "&="s, true },
        { symbol_binary_t::OR, "|="s, true },
        { symbol_binary_t::XOR, "^="s, true },
      })) : prob(ast->rhs));
    },

    +[](std::shared_ptr<assignexpr_t> ast)->symbol_t::ptr {
      return ast->assign ? prob(ast->assign) : prob(ast->cond);
    },

    +[](std::shared_ptr<expr_t> ast)->symbol_t::ptr {
      return ast->arglist ? prob(ast->arglist) : prob(ast->assign);
    },

    +[](std::shared_ptr<arglist_t> ast)->symbol_t::ptr {
      auto lhs = prob(ast->lhs)->to<symbol_val_t>();
      auto rvck = [&](symbol_val_t::ptr opr) {
        if (opr->rvvalue()) {
          ast->error() << "expression do not accept rv-value. convert to lvalue by assignment first." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        } return 0;
      };
      rvck(lhs); 
      return std::make_shared<symbol_arglist_t>(ast, prob(ast->lhs)->to<symbol_val_t>(), prob(ast->rhs)->to<symbol_val_t>());
    },

    +[](std::shared_ptr<printstmt_t> ast)->symbol_t::ptr {
      auto obj = prob(ast->objs)->to<symbol_val_t>();
      auto objs = obj->to<symbol_arglist_t>();
      auto rvck = [&](symbol_val_t::ptr opr) {
        if (opr->rvvalue()) {
          ast->error() << "expression do not accept rv-value. convert to lvalue by assignment first." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        } return 0;
      };
      rvck(obj);
      if (objs) {
        for (auto&& o : objs->args) rvck(o);
      }
      return std::make_shared<symbol_print_t>(ast, obj);
    },

    +[](std::shared_ptr<declstmt_t> ast)->symbol_t::ptr {
      return ast->func ? prob(ast->func) : prob(ast->var);
    },

    +[](std::shared_ptr<type_t> ast)->symbol_t::ptr {
      switch (ast->type) {
        case type_t::EXCEPT:
          return symbol_handler_type_t::new_(ast);
        case type_t::INT:
          return symbol_int_type_t::new_(ast)->intern(ast->storage == type_t::EXTERN ? true : false)->constant(ast->storage == type_t::CONST ? true : false);
        case type_t::FLOAT:
          return symbol_float_type_t::new_(ast)->intern(ast->storage == type_t::EXTERN ? true : false)->constant(ast->storage == type_t::CONST ? true : false);
        case type_t::VECTOR: {
          std::vector<int64_t> size;
          for (auto&& s : ast->size) {
            auto ss = prob(s)->to<symbol_val_t>();
            try {
              size.push_back(std::get<int64_t>(ss->constexpr_eval()));
            } catch(...) {
              if (ss->constexpr_eval().index()) {
                ss->error() << "vector size must be int, got " << ss->type->name() << "." << s->eol();
                ss->type->note() << "type defined from here:" << ss->eol();
              } else {
                ss->error() << "vector size must be constant expression." << ss->eol();
              }
              size.push_back(1);
            }
          }
          return symbol_vec_type_t::new_(ast, size)->intern(ast->storage == type_t::EXTERN ? true : false)->constant(ast->storage == type_t::CONST ? true : false);
        }
      }
      return nullptr;
    },

    +[](std::shared_ptr<vardecl_t> ast)->symbol_t::ptr {
      auto type = prob(ast->type)->to<symbol_type_t>();
      std::string name = ast->ident->name;
      std::vector<int64_t> size;
      auto init = prob(ast->init)->to<symbol_val_t>();
      auto stmts = std::make_shared<symbol_stmt_list_t>(ast);
      for (auto&& s : ast->size) {
        auto ss = prob(s)->to<symbol_val_t>();
        try {
          size.push_back(std::get<int64_t>(ss->constexpr_eval()));
        } catch(...) {
          if (ss->constexpr_eval().index()) {
            ss->error() << "array size must be int, got " << ss->type->name() << "." << s->eol();
            ss->type->note() << "type defined from here:" << ss->eol();
          } else {
            ss->error() << "array size must be constant expression." << ss->eol();
          }
          size.push_back(1);
        }
      }
      for (auto&& s : size) {
        type = std::make_shared<symbol_array_type_t>(type, s);
      }
      auto var = std::make_shared<symbol_var_t>(ast, type, name);
      auto current = scoped_lookup(name);
      if (current) {
        if (!(current is typeid(symbol_var_t) || current is typeid(symbol_call_t))) {
          ast->error() << "declaration of variable conflicts with a non-variable symbol." << ast->eol();
          current->note() << "previous declaration is from here:" << current->eol();
        }
        if (name in symbol_registry.front()) {
          auto previous = current->to<symbol_var_t>();
          if (previous) stmts->list.push_back(std::make_shared<symbol_free_t>(ast, previous));
        }
      }
      if (type is typeid(symbol_handler_type_t)) {
        if (init) {
          if (init is typeid(symbol_call_t) && init->rvvalue()) {
            symbol_registry.front()[name] = init;
            stmts->list.push_back(std::make_shared<symbol_eval_t>(ast, init));
          } else if (init is typeid(symbol_call_t)) {
            init->error() << "exception handler must be initialized with an async function call. got sync function call." << ast->eol();
            init->type->note() << "type defined from here:" << init->type->eol();
          } else {
            init->error() << "exception handler must be initialized with an async function call. got " << init->type->name() << ast->eol();
            init->type->note() << "type defined from here:" << init->type->eol();
          }
        } else {
          ast->error() << "exception handler must be initialized with an async function call." << ast->eol();
        }
      } else {
        stmts->list.push_back(std::make_shared<symbol_alloc_t>(ast, var));
        if (init) {
          if (!type->is_scalar()) {
            init->error() << "initialize of non-scalar variables is not implemented." << init->eol();
          }
          if (type->const_) {
            if (init->constexpr_eval().index() == 0) {
              init->error() << "const variable initializer must be constant." << init->eol();
            }
            var->constval = init->constexpr_eval();
          }
        } else if (type->const_) {
          ast->warn() << "const variable default initialized to zero." << ast->eol();
          var->constval = (int64_t)0;
        }
        symbol_registry.front()[name] = var;
      }

      return stmts;
    },

    +[](std::shared_ptr<funcdecl_t> ast)->symbol_t::ptr {
      std::vector<std::string> args;
      std::string name = ast->name;
      for (auto&& v : ast->args)
        args.push_back(v->var->name);
      if (name in symbol_registry.front()) {
        ast->warn() << "function definition hides previous defined symbol in current scope." << ast->eol();
        symbol_registry.front()[name]->note() << "previous defined here:" << symbol_registry.front()[name]->eol();
      }
      auto func_type = symbol_func_type_t::new_(ast, name, args);
      auto func = std::make_shared<symbol_func_t>(ast, func_type, ast->body);
      symbol_registry.front()[name] = func;
      return func;
    },

    +[](std::shared_ptr<compstmt_t> ast)->symbol_t::ptr {
      symbol_registry.emplace_front();
      auto stmts = std::make_shared<symbol_stmt_list_t>(ast);
      stmts->list.push_back(std::make_shared<symbol_enscope_t>(ast));
      for (auto&& s : ast->stmts)
        stmts->list.push_back(prob(s)->to<symbol_stmt_t>());
      //for (auto&& kv : symbol_registry.front()) {
      //  auto var = kv.second->to<symbol_var_t>();
      //  if (var) stmts->list.push_back(std::make_shared<symbol_free_t>(ast, var));
      //}
      symbol_registry.pop_front();
      stmts->list.push_back(std::make_shared<symbol_descope_t>(ast));
      return stmts;
    },

    +[](std::shared_ptr<exprstmt_t> ast)->symbol_t::ptr {
      auto val = prob(ast->expr)->to<symbol_val_t>();
      auto rvck = [&](symbol_val_t::ptr opr) {
        if (opr->rvvalue()) {
          ast->error() << "expression do not accept rv-value. convert to lvalue by assignment first." << ast->eol();
          opr->type->note() << "type defined from here:" << opr->type->eol();
        } return 0;
      };
      rvck(val); 
      return std::make_shared<symbol_eval_t>(ast, val);
    },

#define LOOP_COND auto cond = prob(ast->cond)->to<symbol_val_t>(); \
      auto rvck = [&](symbol_val_t::ptr opr) { \
        if (opr->rvvalue()) { \
          ast->error() << "expression do not accept rv-value. convert to lvalue by assignment first." << ast->eol(); \
          opr->type->note() << "type defined from here:" << opr->type->eol(); \
        } return 0; \
      }; \
      rvck(cond); \
      if (!cond->type->is_scalar()) { \
        cond->error() << "condition must be scalar type, got " << cond->type->name() << "." << cond->eol(); \
        cond->type->note() << "type defined from here:" << cond->type->eol(); \
      } \
      if (cond->type is typeid(symbol_vec_type_t) && cond->type->external) { \
        cond->error() << "extern vector must be explicitly moved to intern variables before participating operations." << cond->eol(); \
      } \
      cond = std::make_shared<symbol_unary_t>(ast->cond, std::make_shared<symbol_cast_t>(ast->cond, cond, symbol_int_type_t::new_(ast->cond)), symbol_unary_t::POS);

    +[](std::shared_ptr<forstmt_t> ast)->symbol_t::ptr {
      auto init = prob(ast->init)->to<symbol_val_t>();
      auto step = prob(ast->step)->to<symbol_val_t>();

      LOOP_COND
      rvck(init), rvck(step);
      return std::make_shared<symbol_loop_t>(ast, std::make_shared<symbol_eval_t>(ast->init, init),
          std::make_shared<symbol_break_t>(ast->cond,
            std::make_shared<symbol_unary_t>(ast->cond, cond, symbol_unary_t::NOT)),
          prob(ast->body)->to<symbol_stmt_t>(), 
          std::make_shared<symbol_eval_t>(ast->step, step));
    },

    +[](std::shared_ptr<ifstmt_t> ast)->symbol_t::ptr {
      LOOP_COND
      return std::make_shared<symbol_branch_t>(ast, cond, prob(ast->pos)->to<symbol_stmt_t>(), prob(ast->neg)->to<symbol_stmt_t>());
    },

    +[](std::shared_ptr<dostmt_t> ast)->symbol_t::ptr {
      LOOP_COND
      return std::make_shared<symbol_loop_t>(ast, nullptr,
          nullptr, prob(ast->body)->to<symbol_stmt_t>(), 
          std::make_shared<symbol_break_t>(ast->cond,
            std::make_shared<symbol_unary_t>(ast->cond,cond,symbol_unary_t::NOT)));
    },

    +[](std::shared_ptr<whilestmt_t> ast)->symbol_t::ptr {
      LOOP_COND
      return std::make_shared<symbol_loop_t>(ast, nullptr,
          std::make_shared<symbol_break_t>(ast->cond, std::make_shared<symbol_unary_t>(ast->cond, cond, symbol_unary_t::NOT)),
          prob(ast->body)->to<symbol_stmt_t>(), nullptr); 
    },

    +[](std::shared_ptr<retstmt_t> ast)->symbol_t::ptr {
      return std::make_shared<symbol_return_t>(ast);
    },

    +[](std::shared_ptr<contstmt_t> ast)->symbol_t::ptr {
      return std::make_shared<symbol_continue_t>(ast);
    },

    +[](std::shared_ptr<breakstmt_t> ast)->symbol_t::ptr {
      return std::make_shared<symbol_break_t>(ast);
    },

    +[](std::shared_ptr<awaitstmt_t> ast)->symbol_t::ptr {
      auto handler = prob(ast->handler)->to<symbol_call_t>();
      if (!handler) {
        ast->error() << "await statement expects an exception handler." << ast->eol();
      }
      return std::make_shared<symbol_await_t>(ast, handler);
    },

    +[](std::shared_ptr<ctrlstmt_t> ast)->symbol_t::ptr {
      if (ast->for_) return prob(ast->for_);
      if (ast->if_) return prob(ast->if_);
      if (ast->do_) return prob(ast->do_);
      if (ast->while_) return prob(ast->while_);
      if (ast->ret_) return prob(ast->ret_);
      if (ast->cont_) return prob(ast->cont_);
      if (ast->await_) return prob(ast->await_);
      return prob(ast->break_);
    },

    +[](std::shared_ptr<intrinstmt_t> ast)->symbol_t::ptr {
      auto arglist = prob(ast->args)->to<symbol_arglist_t>();
      for (auto&& arg : arglist->args) {
        if (arg->type->external) {
          ast->error() << "intrinsics only accept intern operands." << ast->eol();
          arg->type->note() << "type defined from here:" << arg->type->eol();
        }
        if (arg->rvvalue()) {
          ast->error() << "intrinsics do not accept rv-value operands." << ast->eol();
          arg->type->note() << "type defined from here:" << arg->type->eol();
        }
      }
      if (!arglist->args.empty() && !arglist->args[0]->lvalue()) {
        ast->error() << "intrinsic destination operand must be lvalue." << ast->eol();
      }
      bool err = false;
      switch(ast->opcode) {
        case intrinstmt_t::CONV: {
            if (arglist->args.size() < 3 || arglist->args.size() > 7) {
              ast->error() << "CONV expect 3~7 arguments, got " << arglist->args.size() << "." << ast->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_conv_t>(ast, nullptr, nullptr, nullptr);
            auto res = arglist->args[0];
            auto wt = arglist->args[1];
            auto im = arglist->args[2];
            auto res_type = res->type->to<symbol_vec_type_t>();
            auto wt_type = wt->type->to<symbol_vec_type_t>();
            auto im_type = im->type->to<symbol_vec_type_t>();
            int64_t sx = 1, sy = 1, px = 0, py = 0;
            if (!res_type || res_type->is_scalar()) {
              ast->error() << "CONV expect vector type for dest operand, got " << res->type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              err = true;
            }
            if (!wt_type || wt_type->is_scalar()) {
              ast->error() << "CONV expect vector type for weight operand, got " << wt->type->name() << ast->eol();
              wt->type->note() << "type defined from here:" << wt->type->eol();
              err = true;
            }
            if (!im_type || im_type->is_scalar()) {
              ast->error() << "CONV expect vector type for image operand, got " << im->type->name() << ast->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            try {
              if (arglist->args.size() >= 4) sx = std::get<int64_t>(arglist->args[3]->constexpr_eval());
              if (arglist->args.size() >= 5) sy = std::get<int64_t>(arglist->args[4]->constexpr_eval());
              if (arglist->args.size() >= 6) px = std::get<int64_t>(arglist->args[5]->constexpr_eval());
              if (arglist->args.size() >= 7) py = std::get<int64_t>(arglist->args[6]->constexpr_eval());
            } catch(...) {
              ast->error() << "CONV expect optional constant int (SX, SY, PX, PY) at 4,5,6,7-th arguments." << ast->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_conv_t>(ast, nullptr, nullptr, nullptr);
            size_t res_dim = 0, wt_dim = 0, im_dim = 0;
            res_dim = res_type->size.size();
            wt_dim = wt_type->size.size();
            im_dim = im_type->size.size();
            if (res_dim != 3 && res_dim != 4) {
              ast->error() << "CONV expect 3 dim HWC or 4 dim NHWC vector for dest operand, got " << res_dim << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              err = true;
            }
            if (wt_dim != 4) {
              ast->error() << "CONV expect 4 dim NHWC vector for weight operand, got " << wt_dim << ast->eol();
              wt->type->note() << "type defined from here:" << wt->type->eol();
              err = true;
            }
            if (im_dim != 3 && im_dim != 4) {
              ast->error() << "CONV expect 3 dim HWC or 4 dim NHWC vector for image operand, got " << im_dim << ast->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (im_dim != res_dim) {
              ast->error() << "CONV expect same dimension of vector for result and image operands, got " << res_dim << " and " << im_dim << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (im_dim == 4 && res_dim == 4 && res_type->size[3] != im_type->size[3]) {
              ast->error() << "CONV expect same batch size of result and image operands, got " << res_type->size[3] << " and " << im_type->size[3] << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (wt_dim == 4 && res_dim >= 3 && wt_type->size[3] != res_type->size[0]) {
              ast->error() << "CONV expect result features equal to weight batch size, got " << res_type->size[0] << " and " << wt_type->size[3] << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              wt->type->note() << "type defined from here:" << wt->type->eol();
              err = true;
            }
            if (wt_dim == 4 && im_dim >= 3 && wt_type->size[0] != im_type->size[0]) {
              ast->error() << "CONV expect input features equal to weight features, got " << im_type->size[0] << " and " << wt_type->size[0] << ast->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              wt->type->note() << "type defined from here:" << wt->type->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_conv_t>(ast, nullptr, nullptr, nullptr);
            int64_t res_n = res_dim == 4 ? im_type->size[3] : 1;
            int64_t res_h = (im_type->size[2] - wt_type->size[2] + py + sy - 1) / sy + 1;
            int64_t res_w = (im_type->size[1] - wt_type->size[1] + px + sx - 1) / sx + 1;
            int64_t res_c = wt_type->size[3];
            if ((res_dim == 4 && res_n != res_type->size[3]) || res_h != res_type->size[2] || res_w != res_type->size[1] || res_c != res_type->size[0]) {
              ast->warn() << "CONV predicated result shape as <" << (res_dim == 4 ? std::to_string(res_n) + ","s : ""s) << res_h << "," << res_w << "," << res_c << ">, got " << res_type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
            }
            return std::make_shared<symbol_conv_t>(ast, res, wt, im, sx, sy, px, py);
          }
          case intrinstmt_t::POOL: {
            if (arglist->args.size() < 6 || arglist->args.size() > 8) {
              ast->error() << "POOL expect 6~8 arguments, got " << arglist->args.size() << "." << ast->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_pool_t>(ast, nullptr, nullptr, 0, 0, 0, 0);
            int64_t sx, sy, kx, ky, px = 0, py = 0;
            auto res = arglist->args[0];
            auto im = arglist->args[1];
            auto res_type = res->type->to<symbol_vec_type_t>();
            auto im_type = im->type->to<symbol_vec_type_t>();
            if (!res_type || res_type->is_scalar() || !(res_type->size.size() == 3 || res_type->size.size() == 4)) {
              ast->error() << "POOL expect 3 dim HWC or 4 dim NHWC vector type for dest operand. got " << res->type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              err = true;
            }
            if (!im_type || im_type->is_scalar() || !(im_type->size.size() == 3 || im_type->size.size() == 4)) {
              ast->error() << "POOL expect 3 dim HWC or 4 dim NHWC vector type for input operand. got " << im->type->name() << ast->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            try {
              kx = std::get<int64_t>(arglist->args[2]->constexpr_eval());
              ky = std::get<int64_t>(arglist->args[3]->constexpr_eval());
              sx = std::get<int64_t>(arglist->args[4]->constexpr_eval());
              sy = std::get<int64_t>(arglist->args[5]->constexpr_eval());
              if (arglist->args.size() >= 7) px = std::get<int64_t>(arglist->args[6]->constexpr_eval());
              if (arglist->args.size() >= 8) py = std::get<int64_t>(arglist->args[7]->constexpr_eval());
            } catch(...) {
              ast->error() << "POOL expect constant int (KX, KY, SX, SY, PX, PY) at 3,4,5,6,7,8-th arguments." << ast->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_pool_t>(ast, nullptr, nullptr, 0, 0, 0, 0);
            if (res_type->size.size() != im_type->size.size()) {
              ast->error() << "POOL expect same dimension of vector type for destination operand and input operand. got " << res->type->name() << " and " << im->type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_pool_t>(ast, nullptr, nullptr, 0, 0, 0, 0);
            if (res_type->size.size() == 4 && res_type->size[3] != im_type->size[3]) {
              ast->error() << "POOL expect same batch size for destination operand and input operand. got " << res_type->size[3] << " and " << im_type->size[3] << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (res_type->size[0] != im_type->size[0]) {
              ast->error() << "POOL expect same feature size for destination operand and input operand. got " << res_type->size[0] << " and " << im_type->size[0] << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_pool_t>(ast, nullptr, nullptr, 0, 0, 0, 0);
            int64_t res_w = (im_type->size[1] - kx + px + sx - 1) / sx + 1;
            int64_t res_h = (im_type->size[2] - ky + py + sy - 1) / sy + 1;
            if (res_type->size[1] != res_w || res_type->size[2] != res_h) {
              ast->warn() << "POOL predicated result shape as <" << (res_type->size.size() == 4 ? std::to_string(res_type->size[3]) + ","s : ""s) << res_h << "," << res_w << "," << res_type->size[0] << ">, got " << res_type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
            }
            return std::make_shared<symbol_pool_t>(ast, res, im, kx, ky, sx, sy, px, py);
          }
          case intrinstmt_t::MM: {
            if (arglist->args.size() != 3 && arglist->args.size() != 4) {
              ast->error() << "MM expect 3 or 4 arguments, got " << arglist->args.size() << "." << ast->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_mm_t>(ast, nullptr, nullptr, nullptr, false);
            auto res = arglist->args[0];
            auto wt = arglist->args[1];
            auto im = arglist->args[2];
            auto res_type = res->type->to<symbol_vec_type_t>();
            auto wt_type = wt->type->to<symbol_vec_type_t>();
            auto im_type = im->type->to<symbol_vec_type_t>();
            bool tr = false;
            if (arglist->args.size() == 4) {
              try {
                tr = !!std::get<int64_t>(arglist->args[3]->constexpr_eval());
              } catch(...) {
                arglist->args[3]->error() << "MM 4-th argument expect constant condition, got " << arglist->args[3]->type->name() << arglist->args[3]->eol();
                arglist->args[3]->type->note() << "type defined form here:" << arglist->args[3]->type->eol();
                err = true;
              }
            }
            if (!res_type || res_type->is_scalar() || !(res_type->size.size() == 2 || res_type->size.size() == 3)) {
              ast->error() << "MM expect 2 dim matrix or 3 dim batched-matrix vector type for dest operand. got " << res->type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              err = true;
            }
            if (!wt_type || wt_type->is_scalar() || !(wt_type->size.size() == 2 || wt_type->size.size() == 3)) {
              ast->error() << "MM expect 2 dim matrix or 3 dim batched-matrix vector type for weight operand. got " << wt->type->name() << ast->eol();
              wt->type->note() << "type defined from here:" << wt->type->eol();
              err = true;
            }
            if (!im_type || im_type->is_scalar() || !(im_type->size.size() == 2 || im_type->size.size() == 3)) {
              ast->error() << "MM expect 2 dim matrix or 3 dim batched-matrix vector type for input operand. got " << im->type->name() << ast->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_mm_t>(ast, nullptr, nullptr, nullptr, false);
            if (res_type->size.size() != wt_type->size.size() || res_type->size.size() != im_type->size.size() ||
                (res_type->size.size() == 3 && (res_type->size[2] != wt_type->size[2] || res_type->size[2] != im_type->size[2]))) {
              ast->error() << "MM expect same batch for all operands." << ast->eol();
              err = true;
            }
            if (wt_type->size[tr ? 1 : 0] != im_type->size[0]) {
              ast->error() << "MM expect same size on dimension " << (tr ? 1 : 0) << " of weight and on dimension 0 of input. got " << wt_type->size[tr ? 1 : 0] << " and " << im_type->size[0] << " respectively." << ast->eol();
              if (wt_type->size[tr ? 0 : 1] == im_type->size[0]) {
                ast->note() << "do you mean transpose (4-th argument) = " << !tr << " ?" << ast->eol();
              }
              wt->type->note() << "type defined from here:" << wt->type->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_mm_t>(ast, nullptr, nullptr, nullptr, false);
            if (res_type->size[0] != wt_type->size[tr ? 0 : 1] || res_type->size[1] != im_type->size[1]) {
              ast->warn() << "MM predicated result shape as <" << (res_type->size.size() == 3 ? std::to_string(im_type->size[2]) + ","s : ""s) << im_type->size[1] << "," << wt_type->size[tr ? 0 : 1] << ">, got " << res_type->name() << ast->eol();
              res_type->note() << "type defined from here:" << res_type->eol();
            }
            return std::make_shared<symbol_mm_t>(ast, res, wt, im, tr);
          }
          case intrinstmt_t::ACT: {
            if (arglist->args.size() != 2) {
              ast->error() << "ACT expect 2 arguments, got " << arglist->args.size() << "." << ast->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_act_t>(ast, nullptr, nullptr);
            auto res = arglist->args[0];
            auto im = arglist->args[1];
            auto res_type = res->type->to<symbol_vec_type_t>();
            auto im_type = im->type->to<symbol_vec_type_t>();
            if (!res_type) {
              ast->error() << "ACT expect vector type for dest operand. got " << res->type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              err = true;
            }
            if (!im_type) {
              ast->error() << "ACT expect vector type for input operand. got " << im->type->name() << ast->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (res_type->signature() != im_type->signature()) {
              ast->error() << "ACT expect same type for dest and input operand. got " << res->type->name() << " and " << im->type->name() << " respectively." << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_act_t>(ast, nullptr, nullptr);
            return std::make_shared<symbol_act_t>(ast, res, im);
          }
          case intrinstmt_t::TRANS: {
            if (arglist->args.size() != 2) {
              ast->error() << "TRANS expect 2 arguments, got " << arglist->args.size() << "." << ast->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_trans_t>(ast, nullptr, nullptr);
            auto res = arglist->args[0];
            auto im = arglist->args[1];
            auto res_type = res->type->to<symbol_vec_type_t>();
            auto im_type = im->type->to<symbol_vec_type_t>();
            if (!res_type || res_type->size.size() != 2) {
              ast->error() << "TRANS expect 2 dim vector type for dest operand. got " << res->type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              err = true;
            }
            if (!im_type || res_type->size.size() != 2) {
              ast->error() << "TRANS expect 2 dim vector type for input operand. got " << im->type->name() << ast->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (res_type->size[0] != im_type->size[1] || res_type->size[1] != im_type->size[0]) {
              ast->error() << "TRANS expect reversed dimension for dest and input operand. got " << res->type->name() << " and " << im->type->name() << " respectively." << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              im->type->note() << "type defined from here:" << im->type->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_trans_t>(ast, nullptr, nullptr);
            return std::make_shared<symbol_trans_t>(ast, res, im);
          }
          case intrinstmt_t::CYCLEADD: {
            if (arglist->args.size() != 3) {
              ast->error() << "CYCLEADD expect 3 arguments, got " << arglist->args.size() << "." << ast->eol();
              err = true;
            }
            if (err) return std::make_shared<symbol_cycleadd_t>(ast, nullptr, nullptr, nullptr);
            auto res = arglist->args[0];
            auto lhs = arglist->args[1];
            auto rhs = arglist->args[2];
            auto res_type = res->type->to<symbol_vec_type_t>();
            auto lhs_type = lhs->type->to<symbol_vec_type_t>();
            auto rhs_type = rhs->type->to<symbol_vec_type_t>();
            if (!res_type || !lhs_type || !rhs_type) {
              ast->error() << "CYCLEADD expect vector type for all operands. got " << res->type->name() << ", " << lhs->type->name() << " and " << rhs->type->name() << ast->eol();
              if (!res_type) res->type->note() << "type defined from here:" << res->type->eol();
              if (!lhs_type) lhs->type->note() << "type defined from here:" << lhs->type->eol();
              if (!rhs_type) rhs->type->note() << "type defined from here:" << rhs->type->eol();
              err = true;
            }
            if (res_type->sizeof_() != lhs_type->sizeof_()) {
              ast->error() << "CYCLEADD expect dest operand have the same size of left-hand-side operand. got " << res_type->name() << " and " << lhs_type->name() << ast->eol();
              res->type->note() << "type defined from here:" << res->type->eol();
              lhs->type->note() << "type defined from here:" << lhs->type->eol();
              err = true;
            }
            if (lhs_type->sizeof_() % rhs_type->sizeof_()) {
              ast->warn() << "CYCLEADD expect a multiple size of right-hand-size operand for left-hand-side operand. got " << lhs->type->name() << " and " << rhs->type->name() << " respectively." << ast->eol();
              lhs->type->note() << "type defined from here:" << lhs->type->eol();
              rhs->type->note() << "type defined from here:" << rhs->type->eol();
            }
            if (err) return std::make_shared<symbol_cycleadd_t>(ast, nullptr, nullptr, nullptr);
            return std::make_shared<symbol_cycleadd_t>(ast, res, lhs, rhs);
          }
          default:
            ast->error() << "intrinsic not implemented." << ast->eol();
      }
      return std::make_shared<symbol_conv_t>(ast, nullptr, nullptr, nullptr);
    },

    +[](std::shared_ptr<stmt_t> ast)->symbol_t::ptr {
      if (ast->print_) return prob(ast->print_);
      if (ast->intrin) return prob(ast->intrin);
      if (ast->ctrl) return prob(ast->ctrl);
      if (ast->comp) return prob(ast->comp);
      if (ast->decl) return prob(ast->decl);
      return prob(ast->expr);
    }
  );
}

std::vector<symbol_stmt_t::ptr> sym;

bool tr_symbol() {
  symbol_registry.emplace_front();
  sym.push_back(std::make_shared<symbol_enscope_t>(null_ast()));
  for (auto&& s : ast) sym.push_back(prob(s)->to<symbol_stmt_t>());
  symbol_registry.pop_front();
  sym.push_back(std::make_shared<symbol_descope_t>(null_ast()));
  if (errors_occurred) return false;
  return true;
}

constexpr size_t spm_size = 1024 * 1024;
std::bitset<spm_size> spm;
int64_t mem_size = 0;

struct reg_t {
  int reg;
  reg_t() : reg(0) { }
  reg_t(int regid) : reg(regid) { }
  static reg_t alloc(symbol_t::ptr val = nullptr) {
    static std::unordered_map<symbol_t::ptr, reg_t> regtbl; 
    static int global_regid = 0;
    if (!val) return reg_t(++global_regid);
    if (regtbl.find(val) != regtbl.end()) return regtbl.find(val)->second;
    return (regtbl[val] = reg_t(++global_regid));
  }
};

struct opaddr_t {
  symbol_t::ptr v;
  symbol_t::ptr e;
  std::variant<int64_t, reg_t> addr;
  int64_t start;
  int64_t size;
  int64_t strike;
  opaddr_t(symbol_t::ptr v, symbol_t::ptr e, std::variant<int64_t, reg_t> addr, int64_t start, int64_t size, int64_t strike) :
    v(v), e(e), addr(addr), start(start), size(size), strike(strike) { }
  opaddr_t(const opaddr_t& other) :
    v(other.v), e(other.e), addr(other.addr), start(other.start), size(other.size), strike(other.strike) { }
  std::string tostr() const {
    if (std::holds_alternative<int64_t>(addr)) {
      if (std::get<int64_t>(addr) < start || std::get<int64_t>(addr) + strike > start + size) {
        if (!e) {
          v->warn() << "address referenced out of variable defining range." << v->eol();
        } else {
          e->warn() << "address referenced out of variable defining range." << e->eol();
          v->note() << "variable defined from here:" << v->eol();
        }
      }
      return "ptr/#"s + std::to_string(std::get<int64_t>(addr));
    } else {
      return "ptr/r"s + std::to_string(std::get<reg_t>(addr).reg);
    }
  }
};

addr_t operator+(addr_t lhs, int64_t offset);

struct addr_t : public std::enable_shared_from_this<addr_t> {
  using ptr = std::shared_ptr<addr_t>;
  symbol_t::ptr v;
  bool spm;
  int64_t start;
  int64_t size;
  std::variant<int64_t, reg_t> addr;
  int64_t strike;
  ptr fillback;
  ptr fillback2;

  addr_t(symbol_t::ptr v, bool spm, int64_t start, int64_t size) : v(v), spm(spm), start(start), size(size), addr(start), strike(1), fillback(nullptr), fillback2(nullptr) { }
  addr_t(const addr_t& other) : v(other.v), spm(other.spm), start(other.start), size(other.size), addr(other.addr), strike(other.strike), fillback(other.fillback), fillback2(other.fillback2) { }

  opaddr_t opr(symbol_t::ptr e) const {
    return opaddr_t(v, e, addr, start, size, strike);
  }

  ptr rv(bool is_rv) {
    auto p =std::make_shared<addr_t>(*this);
    p->fillback = is_rv ? shared_from_this() : nullptr;
    return p;
  }
  void fb(ptr from) {
    if (fillback) fillback->fb(from);
    if (fillback2) fillback2->fb(from);
    *this = *from;
  }

  addr_t::ptr offset(int64_t imm) {
    auto copy = std::make_shared<addr_t>(*this);
    *copy = *copy + imm;
    return copy;
  }
};

struct inst_t {
  using ptr = std::shared_ptr<inst_t>;
static std::string tostr(int64_t i) {
    return "#"s + std::to_string(i);
  }
static std::string tostr(double i) {
    return "#"s + std::to_string(i) + "f"s;
  }
static std::string tostr(reg_t i) {
    return "r"s + std::to_string(i.reg);
  }
static std::string tostr(ptr i) {
    return "!"s + std::to_string(i->get_pc());
  }
static std::string tostr(std::string i) {
    return std::to_string(i.size()) + "s"s + i;
  }
static std::string tostr(opaddr_t i) {
    return i.tostr();
  }
static std::string tostr(addr_t::ptr i) {
    return i->opr(nullptr).tostr();
  }
  virtual int64_t get_pc() const = 0;
  virtual void set_pc(int64_t) = 0;
  virtual std::string print() const = 0;
};

template <typename T, std::size_t ...I, typename F>
void tuple_foreach_impl(T &&tuple, std::index_sequence<I...>, F &&func)
{
    ((void(func(std::get<I>(tuple))), 0), ...);
}

template <typename T, typename F> void tuple_foreach(T &&tuple, F &&func)
{
    constexpr int size = std::tuple_size<std::remove_reference_t<T>>::value;
    tuple_foreach_impl(std::forward<T>(tuple), std::make_index_sequence<size>{},
                       std::forward<F>(func));
}

template<class... Args>
struct vinst_t : public inst_t {
  std::string opname;
  std::tuple<Args...> b;
  int64_t pc;
  vinst_t(std::string opname, Args ...args) : inst_t(), opname(opname), b(args...), pc(-1) { }
  virtual std::string print() const {
    std::stringstream ss;
    ss << opname;
    tuple_foreach(b, [&](auto&& value) { ss << " " << tostr(value);  });
    return ss.str();
  }
  virtual void set_pc(int64_t pc_) {
    pc = pc_;
  }
  virtual int64_t get_pc() const {
    return pc;
  }
};

struct alloc_t {
  int64_t addr;
  int64_t size;
  alloc_t(int64_t size) : size(size) {
    int64_t start = -1;
    int64_t strike = 0;
    for (int i = 0; i < spm_size; i++) {
      if (!spm[i]) {
        if (!strike) {
          start = i;
        }
        strike++;
        if (strike == size) break;
      } else {
        strike = 0;
      }
    }
    if (strike == size) {
      for (int i = 0; i < strike; i++) {
        spm[start + i] = 1;
      }
      addr = start;
    } else {
      throw std::bad_alloc();
    }
  }
  ~alloc_t() {
    for (int i = 0; i < size; i++) {
      spm[addr + i] = 0;
    }
  }
  using ptr = std::shared_ptr<alloc_t>;
};

std::list<std::vector<alloc_t::ptr>> active;

using genval_t = std::variant<std::monostate, reg_t, addr_t::ptr, std::shared_ptr<symbol_func_t>, std::string>;
std::list<inst_t::ptr> lst;
std::list<inst_t::ptr> elst;
template<class... Args>
inst_t::ptr inst(std::string name, Args... args) {
  return std::make_shared<vinst_t<Args...>>(name, args...);
}
template<class... Args>
void pinst(std::string name, Args... args) {
  auto p = inst(name, args...);
  lst.emplace_back(std::move(p));
}
addr_t operator+(addr_t lhs, int64_t offset) {
  if (std::holds_alternative<int64_t>(lhs.addr)) {
    lhs.addr = std::get<int64_t>(lhs.addr) + offset;
    return lhs;
  } else {
    pinst("addi", std::get<reg_t>(lhs.addr), std::get<reg_t>(lhs.addr), offset);
    return lhs;
  }
}
addr_t operator+(addr_t lhs, reg_t offset) {
  if (std::holds_alternative<int64_t>(lhs.addr)) {
    auto t = reg_t::alloc();
    pinst("addi", t, offset, std::get<int64_t>(lhs.addr));
    lhs.addr = t;
    return lhs;
  } else {
    pinst("addi", std::get<reg_t>(lhs.addr), std::get<reg_t>(lhs.addr), offset);
    return lhs;
  }
}

void exec(symbol_stmt_t::ptr stmt);
reg_t except_reg(0);

genval_t eval(symbol_val_t::ptr val) {
  auto cv = val->constexpr_eval();
  switch (cv.index()) {
  case 1:
    pinst("movis", reg_t::alloc(val), std::get<int64_t>(cv));
    return reg_t::alloc(val);
  case 2:
    pinst("movis", reg_t::alloc(val), std::get<double>(cv));
    return reg_t::alloc(val);
  case 3:
    return std::get<std::string>(cv);
  default:;
  }
  return typeswitch(val,

    +[](std::shared_ptr<symbol_var_t> val)->genval_t {
      if (val->va < 0) {
        val->error() << "variable " << val->type->name() << "\"" << val->name << "\" used before allocated... this must be a bug of eopcc?" << val->eol();
        return std::monostate();
      }
      auto type = val->type;
      while (type is typeid(symbol_array_type_t)) type = type->to<symbol_array_type_t>()->elem_type;
      return std::make_shared<addr_t>(val, val->type is typeid(symbol_vec_type_t), val->va, val->type->sizeof_());
    },

    +[](std::shared_ptr<symbol_arglist_t> val)->genval_t {
      genval_t arg;
      for (auto&& a : val->args) {
        arg = eval(a);
      }
      return arg;
    },

    +[](std::shared_ptr<symbol_call_t> val)->genval_t {
      if (val->async) {
        auto async_entry = inst("nop");
        pinst("raise", reg_t(0), async_entry);
        elst.push_back(async_entry);
        swap(elst, lst);
        except_reg = reg_t::alloc(val);
        is_in_except = true;
      }
      exec(val->exec);
      if (val->async) {
        pinst("yield", reg_t::alloc(val));
        swap(elst, lst);
        is_in_except = false;
      }
      return reg_t::alloc(val);
    },

    +[](std::shared_ptr<symbol_cast_t> val)->genval_t {
      auto origin = val->from->type;
      auto from = eval(val->from);
      auto to = val->type;

      if (origin is typeid(symbol_vec_type_t) && to is typeid(symbol_vec_type_t)) {
        auto p = std::get<addr_t::ptr>(from);
        p->strike = to->sizeof_();
        return p;
      } else if (to is typeid(symbol_vec_type_t) && to->is_scalar()) {
        if (origin->external) {
          pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
          from = reg_t::alloc(val);
        }
        if (origin is typeid(symbol_int_type_t)) {
          if (std::holds_alternative<reg_t>(from)) {
            pinst("cvtif", reg_t::alloc(val), std::get<reg_t>(from));
          } else if (std::holds_alternative<addr_t::ptr>(from)) {
            pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
            pinst("cvtif", reg_t::alloc(val), reg_t::alloc(val));
          }
        }
        if (origin is typeid(symbol_float_type_t)) {
          if (std::holds_alternative<reg_t>(from)) {
          } else if (std::holds_alternative<addr_t::ptr>(from)) {
            pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
          }
        }
        if (origin is typeid(symbol_vec_type_t)) {
          pinst("movvs", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
        }
        auto rv = std::make_shared<addr_t>(val, true, 0, to->sizeof_())->rv(true);
        pinst("movsv", rv, reg_t::alloc(val));
        return rv;
      } else {
        if (origin->external) {
          pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
          from = reg_t::alloc(val);
        }
        if (origin->signature() == to->signature()) {
          return from;
        } else if (origin is typeid(symbol_float_type_t) && to is typeid(symbol_int_type_t)) {
          if (std::holds_alternative<reg_t>(from)) {
            pinst("cvtfi", reg_t::alloc(val), std::get<reg_t>(from));
          } else if (std::holds_alternative<addr_t::ptr>(from)) {
            pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
            pinst("cvtfi", reg_t::alloc(val), reg_t::alloc(val));
          }
        } else if (origin is typeid(symbol_int_type_t) && to is typeid(symbol_float_type_t)) {
          if (std::holds_alternative<reg_t>(from)) {
            pinst("cvtif", reg_t::alloc(val), std::get<reg_t>(from));
          } else if (std::holds_alternative<addr_t::ptr>(from)) {
            pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
            pinst("cvtif", reg_t::alloc(val), reg_t::alloc(val));
          }
        } else if (origin is typeid(symbol_vec_type_t) && to is typeid(symbol_float_type_t)) {
          pinst("movvs", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
        } else if (origin is typeid(symbol_vec_type_t) && to is typeid(symbol_int_type_t)) {
          pinst("movvs", reg_t::alloc(val), std::get<addr_t::ptr>(from)->opr(val));
          pinst("cvtfi", reg_t::alloc(val), reg_t::alloc(val));
        }
        return reg_t::alloc(val);
      }
    },

    +[](std::shared_ptr<symbol_descript_t> val)->genval_t {
      auto lhs = std::get<addr_t::ptr>(eval(val->oprand));
      if (val->subscript->constexpr_eval().index() == 1) {
        *lhs = *lhs + std::get<int64_t>(val->subscript->constexpr_eval()) * val->type->sizeof_();
      } else {
        auto rhs = eval(val->subscript);
        if (std::holds_alternative<addr_t::ptr>(rhs)) {
          pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(rhs)->opr(val));
          pinst("muli", reg_t::alloc(val), reg_t::alloc(val), val->type->sizeof_());
          rhs = reg_t::alloc(val);
        }
        *lhs = *lhs + std::get<reg_t>(rhs);
      }
      return lhs;
    },

    +[](std::shared_ptr<symbol_unary_t> val)->genval_t {
      auto opr = eval(val->oprand);
      auto origin = opr;
      if (val->oprand->type->is_scalar() && !(val->oprand->type is typeid(symbol_vec_type_t))
          && std::holds_alternative<addr_t::ptr>(opr)) {
        pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(opr)->opr(val));
        opr = reg_t::alloc(val);
      }
      switch(val->opcode) {
        case symbol_unary_t::POSTINC: {
          auto t = reg_t::alloc();
          pinst("addi", t, std::get<reg_t>(opr), (int64_t)1);
          if (std::holds_alternative<addr_t::ptr>(origin))
            pinst("stores", std::get<addr_t::ptr>(origin)->opr(val), t);
          return std::get<reg_t>(opr);
        }
        case symbol_unary_t::POSTDEC: {
          auto t = reg_t::alloc();
          pinst("subi", t, std::get<reg_t>(opr), (int64_t)1);
          if (std::holds_alternative<addr_t::ptr>(origin))
            pinst("stores", std::get<addr_t::ptr>(origin)->opr(val), t);
          return std::get<reg_t>(opr);
        }
        case symbol_unary_t::INC: {
          pinst("addi", reg_t::alloc(val), std::get<reg_t>(opr), (int64_t)1);
          if (std::holds_alternative<addr_t::ptr>(origin))
            pinst("stores", std::get<addr_t::ptr>(origin)->opr(val), reg_t::alloc(val));
          return reg_t::alloc(val);
        }
        case symbol_unary_t::DEC: {
          pinst("subi", reg_t::alloc(val), std::get<reg_t>(opr), (int64_t)1);
          if (std::holds_alternative<addr_t::ptr>(origin))
            pinst("stores", std::get<addr_t::ptr>(origin)->opr(val), reg_t::alloc(val));
          return reg_t::alloc(val);
        }
        case symbol_unary_t::PROD: {
          pinst("hmulv", reg_t::alloc(val), std::get<addr_t::ptr>(opr)->opr(val), val->oprand->type->sizeof_());
          return reg_t::alloc(val);
        }
        case symbol_unary_t::ALL: {
          pinst("hminv", reg_t::alloc(val), std::get<addr_t::ptr>(opr)->opr(val), val->oprand->type->sizeof_());
          return reg_t::alloc(val);
        }
        case symbol_unary_t::POS: if (val->oprand->type->is_scalar()) {
          if (val->oprand->type is typeid(symbol_vec_type_t)) {
            addr_t::ptr rv = std::get<addr_t::ptr>(opr)->rv(val->oprand->rvvalue());
            rv->strike = val->oprand->type->sizeof_();
            pinst("movv", rv, std::get<addr_t::ptr>(opr), rv->strike);
            return rv;
          } else {
            return std::get<reg_t>(opr);
          }
        } else {
          pinst("haddv", reg_t::alloc(val), std::get<addr_t::ptr>(opr)->opr(val), val->oprand->type->sizeof_());
          return reg_t::alloc(val);
        }
        case symbol_unary_t::NEG: if (val->oprand->type is typeid(symbol_int_type_t)) {
          pinst("subi", reg_t::alloc(val), reg_t(0), std::get<reg_t>(opr));
          return reg_t::alloc(val);
        } else if (val->oprand->type is typeid(symbol_float_type_t)) {
          pinst("subf", reg_t::alloc(val), reg_t(0), std::get<reg_t>(opr));
          return reg_t::alloc(val);
        } else if (val->oprand->type is typeid(symbol_vec_type_t)) {
          addr_t::ptr rv = std::get<addr_t::ptr>(opr)->rv(val->oprand->rvvalue());
          pinst("subfv", rv, reg_t(0), std::get<addr_t::ptr>(opr), val->oprand->type->sizeof_()); // accept rv-value
          return rv;
        } else {
          pinst("haddv", reg_t::alloc(val), std::get<addr_t::ptr>(opr)->opr(val), val->oprand->type->sizeof_());
          return reg_t::alloc(val);
        }
        case symbol_unary_t::NOT: {
          pinst("eqi", reg_t::alloc(val), reg_t(0), std::get<reg_t>(opr));
          return reg_t::alloc(val);
        }
        case symbol_unary_t::REV: {
          pinst("noti", reg_t::alloc(val), std::get<reg_t>(opr));
          return reg_t::alloc(val);
        }
        case symbol_unary_t::ANY: {
          pinst("hmaxv", reg_t::alloc(val), std::get<addr_t::ptr>(opr)->opr(val), val->oprand->type->sizeof_());
          return reg_t::alloc(val);
        }
      } return std::monostate();
    },

    +[](std::shared_ptr<symbol_binary_t> val)->genval_t {
      auto lhs = eval(val->lhs);
      reg_t r = reg_t::alloc(val);
      reg_t t = reg_t::alloc();
      if (val->lhs->type->is_scalar() && !(val->lhs->type is typeid(symbol_vec_type_t))
          && std::holds_alternative<addr_t::ptr>(lhs)) {
        pinst("loads", r, std::get<addr_t::ptr>(lhs)->opr(val));
        lhs = r;
      }
      auto rhs = eval(val->rhs);
      if (val->rhs->type->is_scalar() && !(val->rhs->type is typeid(symbol_vec_type_t))
          && std::holds_alternative<addr_t::ptr>(rhs)) {
        pinst("loads", t, std::get<addr_t::ptr>(rhs)->opr(val));
        rhs = t;
      }
      // I - I
      if (val->lhs->type is typeid(symbol_int_type_t) && val->rhs->type is typeid(symbol_int_type_t)) {
        const std::string iname[] = { "muli", "divi", "modi", "addi", "subi", "shli", "shri",
            "lti", "gti", "lei", "gei", "eqi", "nei", "andi", "xori", "ori", "andi", "ori" };
        if (val->opcode >= symbol_binary_t::LAND && val->opcode <= symbol_binary_t::LOR) {
          pinst("nei", r, reg_t(0), std::get<reg_t>(lhs));
          pinst("nei", t, reg_t(0), std::get<reg_t>(rhs));
          lhs = r; rhs = t;
        }
        pinst(iname[val->opcode], r, std::get<reg_t>(lhs), std::get<reg_t>(rhs));
        return r;
      }
      // F - F
      if (val->lhs->type is typeid(symbol_float_type_t) && val->rhs->type is typeid(symbol_float_type_t)) {
        const std::string iname[] = { "mulf", "divf", "?", "addf", "subf", "?", "?",
            "ltf", "gtf", "lef", "gef", "eqf", "nef", "?", "?", "?", "?", "?" };
        pinst(iname[val->opcode], reg_t::alloc(val), std::get<reg_t>(lhs), std::get<reg_t>(rhs));
        return reg_t::alloc(val);
      }
      // F - V
      if (val->lhs->type is typeid(symbol_float_type_t) && val->rhs->type is typeid(symbol_vec_type_t)) {
        const std::string iname[] = { "mulvf", "?", "?", "addvf", "subfv", "?", "?",
            "ltvf", "gtvf", "levf", "gevf", "eqvf", "nevf", "?", "?", "?", "?", "?" };
        auto rv = std::get<addr_t::ptr>(rhs)->rv(val->rhs->rvvalue());
        rv->strike = val->rhs->type->sizeof_();
        pinst(iname[val->opcode], rv, std::get<reg_t>(lhs), std::get<addr_t::ptr>(rhs), rv->strike);
        return rv;
      }
      // V - F
      if (val->lhs->type is typeid(symbol_vec_type_t) && val->rhs->type is typeid(symbol_float_type_t)) {
        const std::string iname[] = { "mulvf", "?", "?", "addvf", "subvf", "?", "?",
            "ltvf", "gtvf", "levf", "gevf", "eqvf", "nevf", "?", "?", "?", "?", "?" };
        auto rv = std::get<addr_t::ptr>(lhs)->rv(val->lhs->rvvalue());
        rv->strike = val->lhs->type->sizeof_();
        pinst(iname[val->opcode], rv, std::get<addr_t::ptr>(lhs), std::get<reg_t>(rhs), rv->strike);
        return rv;
      }
      // V - V
      const std::string iname[] = { "mulv", "divv", "?", "addv", "subv", "?", "?",
          "ltv", "gtv", "lev", "gev", "eqv", "nev", "?", "?", "?", "?", "?" };
      auto rv = std::get<addr_t::ptr>(lhs)->rv(val->lhs->rvvalue());
      if (val->rhs->rvvalue()) rv->fillback = std::get<addr_t::ptr>(rhs);
      rv->strike = std::min(val->lhs->type->sizeof_(), val->rhs->type->sizeof_());
      pinst(iname[val->opcode], rv, std::get<addr_t::ptr>(lhs), std::get<addr_t::ptr>(rhs), rv->strike);
      return rv;
    },

    +[](std::shared_ptr<symbol_cond_t> val)->genval_t {
      auto cond = eval(val->cond);
      reg_t t = reg_t::alloc();
      if (val->cond->type->is_scalar() && !(val->cond->type is typeid(symbol_vec_type_t))
          && std::holds_alternative<addr_t::ptr>(cond)) {
        pinst("loads", t, std::get<addr_t::ptr>(cond)->opr(val));
        cond = t;
      } else if (val->cond->type->is_scalar() && std::holds_alternative<addr_t::ptr>(cond)) {
        pinst("movvs", t, std::get<addr_t::ptr>(cond)->opr(val));
        cond = t;
      }
      auto entry_false = inst("nop");
      auto exit_point = inst("nop");
      pinst("jz", std::get<reg_t>(cond), entry_false);
      genval_t ret;
      // if expr
      {
        auto lhs = eval(val->lhs);
        addr_t::ptr rv;
        if (val->type is typeid(symbol_vec_type_t)) {
          rv = std::get<addr_t::ptr>(lhs)->rv(val->lhs->rvvalue());
        }
        if (val->lhs->type->is_scalar() && !(val->lhs->type is typeid(symbol_vec_type_t))
            && std::holds_alternative<addr_t::ptr>(lhs)) {
          pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(lhs)->opr(val));
          ret = reg_t::alloc(val);
        } else if (val->type is typeid(symbol_vec_type_t)) {
          if (!val->lhs->rvvalue()) {
            pinst("movv", rv, std::get<addr_t::ptr>(lhs), val->lhs->type->sizeof_());
          }
          ret = rv;
        } else if (val->type->is_scalar()) {
          pinst("movs", reg_t::alloc(val), std::get<reg_t>(lhs));
          ret = reg_t::alloc(val);
        }
      }
      pinst("jz", reg_t(0), exit_point);
      // else expr
      lst.push_back(entry_false);
      {
        auto rhs = eval(val->rhs);
        addr_t::ptr rv;
        if (val->type is typeid(symbol_vec_type_t)) {
          rv = std::get<addr_t::ptr>(ret);
          if (val->rhs->rvvalue()) rv->fillback2 = std::get<addr_t::ptr>(rhs);
        }
        if (val->rhs->type->is_scalar() && !(val->rhs->type is typeid(symbol_vec_type_t))
            && std::holds_alternative<addr_t::ptr>(rhs)) {
          pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(rhs)->opr(val));
        } else if (val->type is typeid(symbol_vec_type_t)) {
          if (!val->rhs->rvvalue()) {
            pinst("movv", rv, std::get<addr_t::ptr>(rhs), val->rhs->type->sizeof_());
          }
        } else if (val->type->is_scalar()) {
          pinst("movs", reg_t::alloc(val), std::get<reg_t>(rhs));
        }
      }
      lst.push_back(exit_point);
      return ret;
    },

    +[](std::shared_ptr<symbol_assign_t> val)->genval_t {
      auto lhs = std::get<addr_t::ptr>(eval(val->lhs));
      auto rhs = eval(val->rhs);
      if (val->lhs->type->is_scalar() && !(val->lhs->type is typeid(symbol_vec_type_t))) {
        if (std::holds_alternative<addr_t::ptr>(rhs)) {
          pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(rhs)->opr(val));
          rhs = reg_t::alloc(val);
        }
        pinst("stores", lhs->opr(val), std::get<reg_t>(rhs));
        return std::get<reg_t>(rhs);
      } else if (val->lhs->type is typeid(symbol_int_type_t)) {
        if (std::holds_alternative<addr_t::ptr>(rhs)) {
          pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(rhs)->opr(val));
          rhs = reg_t::alloc(val);
        }
        pinst("cvtif", reg_t::alloc(val), std::get<reg_t>(rhs));
        pinst("movsv", lhs->opr(val), reg_t::alloc(val));
        return std::get<reg_t>(rhs);
      } else if (val->lhs->type is typeid(symbol_float_type_t)) {
        if (std::holds_alternative<addr_t::ptr>(rhs)) {
          pinst("loads", reg_t::alloc(val), std::get<addr_t::ptr>(rhs)->opr(val));
          rhs = reg_t::alloc(val);
        }
        pinst("movsv", lhs->opr(val), reg_t::alloc(val));
        return std::get<reg_t>(rhs);
      } else {
        auto _rhs = std::get<addr_t::ptr>(rhs);
        lhs->strike = _rhs->strike = std::min(val->lhs->type->sizeof_(), val->rhs->type->sizeof_());
        if (val->lhs->type->external) {
          pinst("storev", lhs->opr(val), _rhs->opr(val), lhs->strike);
        } else if (val->rhs->type->external) {
          pinst("loadv", lhs->opr(val), _rhs->opr(val), lhs->strike);
        } else if (val->rhs->rvvalue()) {
          _rhs->fb(lhs);
        } else {
          pinst("movv", lhs->opr(val), _rhs->opr(val), lhs->strike);
        }
        return _rhs; 
      }
    }
  );
}

inst_t::ptr break_point;
inst_t::ptr continue_point;
inst_t::ptr return_point;
genval_t return_val;

void exec(symbol_stmt_t::ptr stmt) {
  if (!stmt) return;
  typeswitch(stmt,

    +[](std::shared_ptr<symbol_enscope_t> stmt) {
      active.emplace_front();
    },

    +[](std::shared_ptr<symbol_descope_t> stmt) {
      active.pop_front();
    },

    +[](std::shared_ptr<symbol_print_t> stmt) {
      std::vector<symbol_val_t::ptr> tp = stmt->objs;
      for (auto&& o : tp) {
        auto arg = eval(o);
        switch(arg.index()) {
        case 0: pinst("print", "<null>"); break;
        case 1: pinst("print", std::get<reg_t>(arg)); break;
        case 2: pinst("print", std::get<addr_t::ptr>(arg)->opr(stmt)); break;
        case 3: pinst("print", "<function>"); break;
        case 4: pinst("print", std::get<std::string>(arg)); break;
        }
      }
    },

    +[](std::shared_ptr<symbol_alloc_t> stmt) {
      if (stmt->var->va >= 0) return;
      auto type = stmt->var->type;
      int64_t size = stmt->var->type->sizeof_();
      while (type is typeid(symbol_array_type_t)) type = type->to<symbol_array_type_t>()->elem_type;
      if (type is typeid(symbol_vec_type_t)) {
        if (stmt->var->type->external) {
          stmt->var->va = spm_size;
        } else try {
          auto alloc = std::make_shared<alloc_t>(size);
          stmt->var->va = alloc->addr;
          active.front().push_back(alloc);
          //pinst("allocated spm for", stmt->var->name, stmt->var->va);
        } catch(...) {
          stmt->error() << "SPM bad alloc: try to alloc " << size << " bytes for " << stmt->var->type->name() << "\"" << stmt->var->name << "\" (currently used " << spm.count() << " bytes)." << stmt->eol();
        }
      } else {
        stmt->var->va = mem_size;
        mem_size += size;
        //pinst("allocated mem for", stmt->var->name, stmt->var->va);
      }
    },

    +[](std::shared_ptr<symbol_free_t> stmt) {
      auto type = stmt->var->type;
      while(type is typeid(symbol_array_type_t)) type = type->to<symbol_array_type_t>()->elem_type;
      if (type is typeid(symbol_vec_type_t)) {
        int64_t addr = stmt->var->va;
        for (auto&& p : active.front()) {
          if (p->addr == addr) {
            p = nullptr; return;
          }
        }
        stmt->warn() << "SPM bad alloc: failed to delete object " << stmt->var->type->name() << "\"" << stmt->var->name << "\" at address " << addr << "." << stmt->eol();
      }
    },

    +[](std::shared_ptr<symbol_await_t> stmt) {
      auto todel = stmt->handler->args_byval;
      for (auto&& v : todel)
        exec(std::make_shared<symbol_free_t>(null_ast(), v->to<symbol_var_t>()));
      pinst("await", reg_t::alloc(stmt->handler));
    },

    +[](std::shared_ptr<symbol_stmt_list_t> stmt) {
      for (auto&& s : stmt->list) {
        exec(s);
      }
    },

    +[](std::shared_ptr<symbol_eval_t> stmt) {
      eval(stmt->expr);
    },

    +[](std::shared_ptr<symbol_break_t> stmt) {
      if (!break_point) {
        stmt->error() << "break out of loop scope." << stmt->eol();
      }
      reg_t r = reg_t(0);
      if (stmt->cond) {
        r = std::get<reg_t>(eval(stmt->cond));
      }
      pinst("jz", r, break_point);
    },

    +[](std::shared_ptr<symbol_continue_t> stmt) {
      if (!continue_point) {
        stmt->error() << "continue out of loop scope." << stmt->eol();
      }
      reg_t r = reg_t(0);
      if (stmt->cond) {
        r = std::get<reg_t>(eval(stmt->cond));
        pinst("eqi", r, reg_t(0), r);
      }
      pinst("jz", r, continue_point);
    },

    +[](std::shared_ptr<symbol_return_t> stmt) {
      if (is_in_except) {
        pinst("yield", except_reg);
      } else if (!return_point) {
        pinst("halt");
      } else {
        pinst("jz", reg_t(0), return_point);
      }
    },

    +[](std::shared_ptr<symbol_loop_t> stmt) {
      auto bbp = break_point;
      auto bcp = continue_point;
      break_point = inst("nop");
      continue_point = inst("nop");
      auto start = inst("nop");
      exec(stmt->init);
      lst.push_back(start);
      exec(stmt->prelog);
      exec(stmt->body);
      lst.push_back(continue_point);
      exec(stmt->epilog);
      pinst("jz", reg_t(0), start);
      lst.push_back(break_point);
      break_point = bbp;
      continue_point = bcp;
    },

    +[](std::shared_ptr<symbol_branch_t> stmt) {
      auto entry_false = inst("nop");
      auto exit_point = inst("nop");
      reg_t r = std::get<reg_t>(eval(stmt->cond));
      pinst("jz", r, entry_false);
      exec(stmt->pos);
      pinst("jz", reg_t(0), exit_point);
      lst.push_back(entry_false);
      exec(stmt->neg);
      lst.push_back(exit_point);
    },

    +[](std::shared_ptr<symbol_conv_t> stmt) {
      auto res = std::get<addr_t::ptr>(eval(stmt->result));
      auto wt = std::get<addr_t::ptr>(eval(stmt->weight));
      auto im = std::get<addr_t::ptr>(eval(stmt->input));
      int64_t fi = stmt->input->type->to<symbol_vec_type_t>()->size[0];
      int64_t fo = stmt->weight->type->to<symbol_vec_type_t>()->size[3];
      int64_t kx = stmt->weight->type->to<symbol_vec_type_t>()->size[1];
      int64_t ky = stmt->weight->type->to<symbol_vec_type_t>()->size[2];
      int64_t xi = stmt->input->type->to<symbol_vec_type_t>()->size[1];
      int64_t yi = stmt->input->type->to<symbol_vec_type_t>()->size[2];
      int64_t bt = stmt->input->type->to<symbol_vec_type_t>()->size.size() == 4 ? 
                   stmt->input->type->to<symbol_vec_type_t>()->size[3] : 1;
      pinst("conv", res, wt, im, fi, fo, kx, ky, xi, yi, bt, stmt->stride_x, stmt->stride_y, stmt->pad_x, stmt->pad_y);
    },

    +[](std::shared_ptr<symbol_pool_t> stmt) {
      auto res = std::get<addr_t::ptr>(eval(stmt->result));
      auto im = std::get<addr_t::ptr>(eval(stmt->input));
      int64_t fi = stmt->input->type->to<symbol_vec_type_t>()->size[0];
      int64_t kx = stmt->kernel_x;
      int64_t ky = stmt->kernel_y;
      int64_t sx = stmt->stride_x;
      int64_t sy = stmt->stride_y;
      int64_t xi = stmt->input->type->to<symbol_vec_type_t>()->size[1];
      int64_t yi = stmt->input->type->to<symbol_vec_type_t>()->size[2];
      int64_t bt = stmt->input->type->to<symbol_vec_type_t>()->size.size() == 4 ? 
                   stmt->input->type->to<symbol_vec_type_t>()->size[3] : 1;
      pinst("pool", res, im, fi, kx, ky, sx, sy, xi, yi, bt, stmt->pad_x, stmt->pad_y);
    },

    +[](std::shared_ptr<symbol_mm_t> stmt) {
      auto res = std::get<addr_t::ptr>(eval(stmt->result));
      auto wt = std::get<addr_t::ptr>(eval(stmt->weight));
      auto im = std::get<addr_t::ptr>(eval(stmt->input));
      int64_t tr = stmt->tr;
      int64_t fi = stmt->input->type->to<symbol_vec_type_t>()->size[0];
      int64_t fo = stmt->weight->type->to<symbol_vec_type_t>()->size[tr ? 0 : 1];
      int64_t ni = stmt->input->type->to<symbol_vec_type_t>()->size[1];
      int64_t bt = stmt->input->type->to<symbol_vec_type_t>()->size.size() == 3 ? 
                   stmt->input->type->to<symbol_vec_type_t>()->size[2] : 1;
      pinst("mm", res, wt, im, tr, fi, ni, fo, bt);
    },

    +[](std::shared_ptr<symbol_act_t> stmt) {
      auto res = std::get<addr_t::ptr>(eval(stmt->result));
      auto im = std::get<addr_t::ptr>(eval(stmt->input));
      int64_t n = stmt->input->type->sizeof_();
      pinst("act", res, im, n);
    },

    +[](std::shared_ptr<symbol_trans_t> stmt) {
      auto res = std::get<addr_t::ptr>(eval(stmt->result));
      auto im = std::get<addr_t::ptr>(eval(stmt->input));
      int64_t n1 = stmt->input->type->to<symbol_vec_type_t>()->size[0];
      int64_t n2 = stmt->input->type->to<symbol_vec_type_t>()->size[1];
      pinst("trans", res, im, n1, n2);
    },

    +[](std::shared_ptr<symbol_cycleadd_t> stmt) {
      auto res = std::get<addr_t::ptr>(eval(stmt->result));
      auto big = std::get<addr_t::ptr>(eval(stmt->bigger));
      auto sml = std::get<addr_t::ptr>(eval(stmt->smaller));
      int64_t n1 = stmt->bigger->type->sizeof_();
      int64_t n2 = stmt->smaller->type->sizeof_();
      pinst("cycleadd", res, big, sml, n1, n2);
    },

    +[](std::shared_ptr<symbol_builtin_conv_t> stmt) {
      if(stmt->operator_type == symbol_builtin_conv_t::CONV) {
        auto dst = std::get<addr_t::ptr>(eval(stmt->dest));
        auto syn = std::get<addr_t::ptr>(eval(stmt->weight));
        auto neu = std::get<addr_t::ptr>(eval(stmt->input));
        auto bia = std::get<addr_t::ptr>(eval(stmt->bias));
        int64_t bt = stmt->bt;
        int64_t xi = stmt->xi;
        int64_t yi = stmt->yi;
        int64_t fi = stmt->fi;
        int64_t fo = stmt->fo;
        int64_t kx = stmt->kx;
        int64_t ky = stmt->ky;
        int64_t sx = stmt->sx;
        int64_t sy = stmt->sy;
        int64_t px = stmt->px;
        int64_t py = stmt->py;
        int64_t xo = (xi + 2*px - kx + sx) / sx;
        int64_t yo = (yi + 2*py - ky + sy) / sy;
        // data_size: 2byte
        int64_t total_size;
        if(stmt->bias != NULL)
          total_size = fi*fo*kx*ky + fo + 2*fi*xi*yi + 4*fo*xo*yo;
        else
          total_size = fi*fo*kx*ky + 2*fi*xi*yi + 2*fo*xo*yo;
        if(2*total_size > 1048576) {
          printf("too large to handle!\n");
          exit(0);
        }


        addr_t::ptr syn_addr  = std::make_shared<addr_t>(stmt, 1, 2*(0                                  ), 2*fi*fo*kx*ky);
        addr_t::ptr neu0_addr = std::make_shared<addr_t>(stmt, 1, 2*(fi*fo*kx*ky                        ), 2*fi*xi*yi);
        addr_t::ptr neu1_addr = std::make_shared<addr_t>(stmt, 1, 2*(fi*fo*kx*ky + fi*xi*yi             ), 2*fi*xi*yi);
        addr_t::ptr res0_addr = std::make_shared<addr_t>(stmt, 1, 2*(fi*fo*kx*ky + 2*fi*xi*yi           ), 2*fo*xo*yo);
        addr_t::ptr res1_addr = std::make_shared<addr_t>(stmt, 1, 2*(fi*fo*kx*ky + 2*fi*xi*yi + fo*xo*yo), 2*fo*xo*yo);

        addr_t::ptr bia_addr     ; 
        addr_t::ptr bia_res0_addr; 
        addr_t::ptr bia_res1_addr; 

        if(stmt->bias != NULL) {
          bia_addr      = std::make_shared<addr_t>(stmt, 1, 2*(fi*fo*kx*ky + 2*fi*xi*yi + 2*fo*xo*yo     ), 2*fo);
          bia_res0_addr = std::make_shared<addr_t>(stmt, 1, 2*(fi*fo*kx*ky + 2*fi*xi*yi + 2*fo*xo*yo + fo), 2*fo*xo*yo);
          bia_res1_addr = std::make_shared<addr_t>(stmt, 1, 2*(fi*fo*kx*ky + 2*fi*xi*yi + 3*fo*xo*yo + fo), 2*fo*xo*yo);

          // load bias
          pinst("loadv", bia_addr, bia, 2*fo);  
        }

        // load weight
        pinst("loadv", syn_addr, syn, 2*fi*fo*kx*ky);  

        for(int iter = 0; iter < bt/2; iter++) {
          // load neu0
          pinst("loadv", (neu0_addr), (neu)->offset((2*iter+0)*2*fi*xi*yi), 2*fi*xi*yi);  

          // load neu1
          pinst("loadv", (neu1_addr), (neu)->offset((2*iter+1)*2*fi*xi*yi), 2*fi*xi*yi);  

          // conv0
          pinst("conv", (res0_addr), (syn_addr), (neu0_addr), fi, fo, kx, ky, xi, yi, (int64_t)1, sx, sy, px, py);
          // conv0 mac_result add bias
          if(stmt->bias != NULL)
            pinst("cycleadd", (bia_res0_addr), (res0_addr), (bia_addr), 2*fo*xo*yo, 2*fo);

          // conv1
          pinst("conv", (res1_addr), (syn_addr), (neu1_addr), fi, fo, kx, ky, xi, yi, (int64_t)1, sx, sy, px, py);
          // conv1 mac_result add bias
          if(stmt->bias != NULL)
            pinst("cycleadd", (bia_res1_addr), (res1_addr), (bia_addr), 2*fo*xo*yo, 2*fo);

          // store res0
          if(stmt->bias != NULL)
            pinst("storev", (dst)->offset((2*iter+0)*2*fo*xo*yo), (bia_res0_addr), 2*fo*xo*yo);  
          else
            pinst("storev", (dst)->offset((2*iter+0)*2*fo*xo*yo), (res0_addr), 2*fo*xo*yo);  

          // store res1
          if(stmt->bias != NULL)
            pinst("storev", (dst)->offset((2*iter+1)*2*fo*xo*yo), (bia_res1_addr), 2*fo*xo*yo);  
          else
            pinst("storev", (dst)->offset((2*iter+1)*2*fo*xo*yo), (res1_addr), 2*fo*xo*yo);  
        }

        if(bt%2) {
          // load neu0
          pinst("loadv", (neu0_addr), (neu)->offset((bt-1)*2*fi*xi*yi), 2*fi*xi*yi);  

          // conv0
          pinst("conv", (res0_addr), (syn_addr), (neu0_addr), fi, fo, kx, ky, xi, yi, (int64_t)1, sx, sy, px, py);
          // conv0 mac_result add bias
          if(stmt->bias != NULL)
            pinst("cycleadd", (bia_res0_addr), (res0_addr), (bia_addr), 2*fo*xo*yo, 2*fo);

          // store res0
          if(stmt->bias != NULL)
            pinst("storev", (dst)->offset((bt-1)*2*fo*xo*yo), (bia_res0_addr), 2*fo*xo*yo);  
          else
            pinst("storev", (dst)->offset((bt-1)*2*fo*xo*yo), (res0_addr), 2*fo*xo*yo);  
        }
      } 

      else if(stmt->operator_type == symbol_builtin_conv_t::MLP) {
        stmt->operator_type = symbol_builtin_conv_t::CONV;
        stmt->xi = (int64_t)1;
        stmt->yi = (int64_t)1;
        stmt->kx = (int64_t)1;
        stmt->ky = (int64_t)1;
        stmt->sx = (int64_t)1;
        stmt->sy = (int64_t)1;
        stmt->px = (int64_t)0;
        stmt->py = (int64_t)0;
        exec(stmt);
      }

      else if(stmt->operator_type == symbol_builtin_conv_t::DEPTHWISE_CONV) {

      }
    }
  );
}

bool codegen(std::ofstream& f) {
  int pc = 0;
  for (auto&& s : sym) {
    exec(s);
    if (errors_occurred) return false;
  }
  for (auto&& i : lst) i->set_pc(pc++);
  for (auto&& i : elst) i->set_pc(pc++);
  f << "main:" << std::endl;
  for (auto&& i : lst) f << "  " << i->get_pc() << "  " << i->print() << std::endl;
  f << "except:" << std::endl;
  for (auto&& i : elst) f << "  " << i->get_pc() << "  " << i->print() << std::endl;
  return true;
}

int main(int argc, char** argv) {
  int fc = 0;
  std::string ofname = "eop.out";
  bool get_output = false;
  for (int fno = 1; fno < argc; fno++) {
    if (get_output) {
      ofname = argv[fno];
      get_output = false;
    } else if (argv[fno][0] == '-') {
      if (argv[fno][1] == 'v') {
        for (int i = 1; argv[fno][i] == 'v'; i++) verbose_level++;
      } else if (argv[fno][1] == 'o') {
        get_output = true;
        continue;
      }
    } else {
      fc++, read_source(argv[fno]);
    }
  }
  std::ofstream of(ofname);
  if (!fc) { std::cout << "no input files specified." << std::endl; return 0; }
  if (!of.good()) { std::cout << "failed to open output file \"" << ofname << "\"." << std::endl; return -1; }
  final_report();
  verbose( 1, std::cout << "lex..." << std::endl );
  lexer();
  verbose( 1, std::cout << "parse..." << std::endl );
  if (!parser()) return -1;
  verbose( 1, std::cout << "build semantics..." << std::endl );
  if (!tr_symbol()) return -1;
  verbose( 1, std::cout << "codegen..." << std::endl );
  if (!codegen(of)) return -1;
  return 0;
}
