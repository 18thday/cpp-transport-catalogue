#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
// —охраните объ€влени€ Dict и Array без изменени€
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* –еализуйте Node, использу€ std::variant */
	using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

	Node() = default;
    Node(Array array);
    Node(Dict map);
    Node(double value);
    Node(int value);
    Node(bool value);
    Node(std::nullptr_t null);
    Node(std::string str);

    bool IsArray() const;
    bool IsBool() const;
    bool IsDouble() const;
    bool IsInt() const;
    bool IsMap() const;
    bool IsNull() const;
    bool IsPureDouble() const;
    bool IsString() const;

    const Array& AsArray() const;
    bool AsBool() const;
    double AsDouble() const;
    int AsInt() const;
    const Dict& AsMap() const;
    const std::string& AsString() const;

    const Value& GetValue() const;

//    bool operator== (const json::Node& rhs);
//    bool operator!= (const json::Node& rhs);
private:
    Value value_;
};

bool operator== (const json::Node& lhs, const json::Node& rhs);
bool operator!= (const json::Node& lhs, const json::Node& rhs);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator== (const json::Document& lhs, const json::Document& rhs);
bool operator!= (const json::Document& lhs, const json::Document& rhs);

Document Load(std::istream& input);

struct PrintContext  {
	PrintContext (std::ostream& out);
	PrintContext (std::ostream& out, int indent_step, int indent = 0);

	PrintContext  Indented() const;

    void PrintIndent() const;

    std::ostream& out;
    int indent_step = 4;
    int indent = 0;
};

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
	ctx.PrintIndent();
    ctx.out << value;
}
void PrintValue(std::nullptr_t, const PrintContext& ctx);
void PrintValue(const std::string& str, const PrintContext& ctx);
void PrintValue(bool b, const PrintContext& ctx);
void PrintValue(Array arr, const PrintContext& ctx);
void PrintValue(Dict dict, const PrintContext& ctx);

void PrintNode(const Node& node,  const json::PrintContext& ctx);
void Print(const Document& doc, std::ostream& output);


}  // namespace json
