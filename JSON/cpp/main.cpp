#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

typedef enum {
  LCRLY = 1,
  RCRLY,
  LSQR,
  RSQR,
  ID,
  COMMA,
  COLON,
  TRUE,
  FALSE,
  NUL,
  NUM,
  STR
} token_t;

typedef tuple<string, token_t, unsigned int> token;

typedef struct {
  unsigned long cursor;
  unsigned int line;
} lexer_t;

string string_of_token_t(token_t type) {
  switch (type) {
  case LCRLY:
    return "LCRLY";
  case RCRLY:
    return "RCRLY";
  case LSQR:
    return "LSQR";
  case RSQR:
    return "RSQR";
  case ID:
    return "ID";
  case COMMA:
    return "COMMA";
  case COLON:
    return "COLON";
  case TRUE:
    return "TRUE";
  case FALSE:
    return "FALSE";
  case NUL:
    return "NUL";
  case NUM:
    return "NUM";
  case STR:
    return "STR";
  default:
    return "Unknown type";
  }
}

typedef enum { QUIET, INFO, WARN, ERROR, DEBUG } loglvl_t;

string string_of_loglvl(loglvl_t lvl) {
  switch (lvl) {
  case INFO:
    return "INFO";
  case WARN:
    return "WARN";
  case ERROR:
    return "ERROR";
  case DEBUG:
    return "DEBUG";
  default:
    return "QUIET";
  }
}

void log(lexer_t *lexer, loglvl_t severity, string msg) {
  string lvl = string_of_loglvl(severity);
  string header = lvl == "QUIET" ? "    | " : "    [ " + lvl + " ] | ";
  string state;
  if (lexer != nullptr) {
    state = format("lexer={{ cursor={:>4}, line:{:>4} }}", lexer->cursor,
                   lexer->line);
  } else {
    state = "";
  }
  cout << header << msg << state << endl;
}

string chomp(lexer_t *lexer, const string *src, char until) {
  unsigned int start = lexer->cursor;
  while ((*src)[lexer->cursor] != until) {
    log(lexer, INFO, format("{} {}", "@chomp:", (*src)[lexer->cursor]));
    lexer->cursor++;
  }
  unsigned int end = lexer->cursor;
  return (*src).substr(end - start);
}

string literal_num(lexer_t *lexer, const string *src) {
  // Numbers end with comma or space?
  string number = "";
  if ((*src)[lexer->cursor] == '-') {
    number += '-';
  }
  return chomp(lexer, src, ' ');
}

vector<token> lex(lexer_t lexer, const string *src) {
  char cur;
  token t;
  vector<token> tokens;
  while (lexer.cursor < (*src).length()) {
    cur = (*src)[lexer.cursor];
    if (cur != '\n' && cur != '\t')
      log(&lexer, INFO, format("{0} ({1:^3c}) ", "@lex:", cur));

    switch (cur) {
    case '\n':
      log(&lexer, INFO, format("{0} ({1:^3s}) ", "@lex:", "\\n"));
      lexer.line++;
      break;
    case '\t':
      log(&lexer, INFO, format("{0} ({1:^3s}) ", "@lex:", "\\t"));
    case '\r':
    case (' '):
      break;
    case '{':
      t = make_tuple("{", LCRLY, lexer.line);
      break;
    case '}':
      t = make_tuple("}", RCRLY, lexer.line);
      break;
    case '"':
      t = make_tuple(chomp(&lexer, src, '"'), STR, lexer.line);
      break;
    case '[':
      t = make_tuple("[", LSQR, lexer.line);
      break;
    case ']':
      t = make_tuple("]", RSQR, lexer.line);
      break;
    case ':':
      t = make_tuple(":", COLON, lexer.line);
      break;
    case ',':
      t = make_tuple(",", COMMA, lexer.line);
      break;
    case 't':
      t = make_tuple("true", TRUE, lexer.line);
      break;
    case 'f':
      t = make_tuple("false", FALSE, lexer.line);
      break;
    case 'n':
      t = make_tuple("null", NUL, lexer.line);
      break;
    default:
      t = make_tuple(literal_num(&lexer, src), NUM, lexer.line);
      break;
    }
    tokens.push_back(t);
    lexer.cursor++;
  }
  return tokens;
}

void print_tokens(vector<token> *tokens) {
  token t;
  string msg = "Tokens:\n";
  string token_type;
  for (vector<token>::size_type i = 0, n = size(*tokens); i < n; i++) {
    token_type = string_of_token_t(get<1>((*tokens)[i]));
    msg += format("(value: {}, type: {}, line: {})", get<0>((*tokens)[i]),
                  token_type, get<2>((*tokens)[i]));
    msg += "\n";
  }
  log(nullptr, QUIET, msg);
}

typedef struct {
  bool trace;
  bool help;
  string filepath;
} options_t;

options_t options = {
    false,
    false,
    "",
};

void help() {
  string usage = "jsonpp [OPTIONS] -- FILE\n";
  usage += "      [OPTIONS]:\n";
  usage += "        -h | --help   :  Show this message.\n";
  usage += "        -t | --trace  :  Log execution trace to stdout.\n";
  log(nullptr, QUIET, usage);
}

int main(int argc, char **argv) {
  if (argc < 1)
    return -1;
  string arg;
  for (int i = 0; i < argc; i++) {
    arg = argv[i];
    if (arg == "-t" || arg == "--trace") {
      options.trace = true;
    } else if (arg == "-h" || arg == "--help") {
      help();
      exit(0);
    } else {
      options.filepath = argv[i];
    }
  }

  ifstream json(options.filepath);
  ostringstream out_stream;
  out_stream << json.rdbuf();
  const string source = out_stream.str();
  log(nullptr, INFO, (format("{}\n{}", "@main: Source file:", source)));

  lexer_t lexer = {0, 1};
  vector<token> tokens = lex(lexer, &source);
  print_tokens(&tokens);

  return 0;
}
