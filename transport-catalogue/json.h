#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Exception in JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    Node() = default;
    template<typename Type>
    Node(Type value) : value_(std::move(value)){
    }

    bool IsArray() const;
    bool IsBool() const;
    bool IsDouble() const;
    bool IsInt() const;
    bool IsDict() const;
    bool IsNull() const;
    bool IsPureDouble() const;
    bool IsString() const;

    const Array& AsArray() const;
    bool AsBool() const;
    double AsDouble() const;
    int AsInt() const;
    const Dict& AsDict() const;
    const std::string& AsString() const;

    const Value& GetValue() const;
    Value& GetValue();

//    bool operator== (const json::Node& rhs) const;
//    bool operator!= (const json::Node& rhs) const;
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
    ctx.out << value;
}
void PrintValue(std::nullptr_t, const PrintContext& ctx);
void PrintValue(const std::string& str, const PrintContext& ctx);
void PrintValue(bool b, const PrintContext& ctx);
void PrintValue(const Array& arr, const PrintContext& ctx);
void PrintValue(const Dict& dict, const PrintContext& ctx);

void PrintNode(const Node& node,  const json::PrintContext& ctx);
void Print(const Document& doc, std::ostream& output);


}  // namespace json
