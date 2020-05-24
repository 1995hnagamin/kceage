#include <cctype>
#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include "lexer.hpp"

BadGetterException::BadGetterException(std::string const & cause):
  std::domain_error(cause)
{}

Token::Token(TokenType type, std::string const & token_literal):
  token_type(type),
  literal(token_literal)
{}

TokenType Token::type() const {
  return token_type;
}

std::string Token::representation() const {
  return literal;
}

std::string Token::get_as_name() const {
  switch (token_type) {
    case TokenType::SmallName:
    case TokenType::CapitalName:
      return literal;
    default:
      throw BadGetterException("Not a name");
  }
}

int Token::get_as_integer() const {
  switch (token_type) {
    case TokenType::Digit:
      return std::stoi(literal);
    default:
      throw BadGetterException("Not an integer");
  }
}

TokenStream::TokenStream(const std::vector<Token>& tokens):
  stream(tokens)
{}

Token *
TokenStream::get() {
  return &(stream[idx++]);
}

void
TokenStream::expect(TokenType type) {
  auto const tok = get();
  assert(tok->type() == type);
}

void
TokenStream::expect(TokenType type, char const *repr) {
  auto const tok = get();
  assert(tok->type() == type);
  assert(tok->representation() == repr);
}

struct char_stream {
  explicit char_stream() {}
  bool is_valid() {
    return index <= cur_line.length();
  }
  bool open(std::string const &filename) {
    ifs.open(filename);
    if (!ifs) {
      return false;
    }
    std::getline(ifs, cur_line);
    index = 0;
    return true;
  }
  char get_next_char() {
    if (index < cur_line.length()) {
      return cur_line[index++];
    }
    if (!ifs) {
      return '\0';
    }
    std::getline(ifs, cur_line);
    index = 0;
    return '\n';
  }
  void back() {
    --index;
  }
  std::ifstream ifs;
  std::string cur_line;
  size_t index;
};

std::vector<Token>
LexicalAnalysis(std::string const &filename) {
  auto const syms = static_cast<std::string>("!$%&-=~^|@+:*<>/?.");

  char_stream stream;
  if (!stream.open(filename)) {
    return {};
  }

  std::vector<Token> tokens;
  while (char c = stream.get_next_char()) {
    switch (c) {
      case '(':
        tokens.emplace_back(TokenType::LParen, "(");
        continue;
      case ')':
        tokens.emplace_back(TokenType::RParen, ")");
        continue;
      case '[':
        tokens.emplace_back(TokenType::LBracket, "[");
        continue;
      case ']':
        tokens.emplace_back(TokenType::RBracket, "]");
        continue;
      case '{':
        tokens.emplace_back(TokenType::LBrace, "{");
        continue;
      case '}':
        tokens.emplace_back(TokenType::RBrace, "}");
        continue;
      case ',':
        tokens.emplace_back(TokenType::Comma, ",");
        continue;
      default:
        ; // do nothing
    }
    if (std::isdigit(c)) {
      std::string cur_token;
      while (std::isdigit(c)) {
        cur_token.push_back(c);
        c = stream.get_next_char();
      }
      stream.back();
      tokens.emplace_back(TokenType::Digit, cur_token);
    } else if (std::islower(c)) {
      std::string cur_token;
      while (std::islower(c) || std::isdigit(c) || c == '_') {
        cur_token.push_back(c);
        c = stream.get_next_char();
      }
      stream.back();
      tokens.emplace_back(TokenType::SmallName, cur_token);
    } else if (std::isupper(c)) {
      std::string capital;
      while (std::isalnum(c)) {
        capital.push_back(c);
        c = stream.get_next_char();
      }
      stream.back();
      tokens.emplace_back(TokenType::SmallName, capital);
    } else if (syms.find(c) != std::string::npos) {
      std::string tok;
      while (syms.find(c) != std::string::npos) {
        tok.push_back(c);
        c = stream.get_next_char();
      }
      stream.back();
      tokens.emplace_back(TokenType::Symbol, tok);
    }
  }
  return tokens;
}
