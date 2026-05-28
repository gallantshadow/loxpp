
#include <format>
#include <variant>

#include "token_type.h"
#include "token.h"

// helper type for the visitor
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

std::string Token::toString() const {

  const auto visitors = overloads{
      [](std::monostate) -> std::string { return "null"; },
      [](double d) -> std::string {
        auto s = std::format("{}", d);
        if (s.find('.') == std::string::npos)
          s += ".0";
        return s;
      },
      [](std::string_view sv) -> std::string {
        return std::string(sv.begin(), sv.size());
      },
  };
  
  auto literalStr = std::visit(visitors, literal);
  
  return std::format("{} {} {}", tokenToString(token), lexeme, literalStr) ;
}

