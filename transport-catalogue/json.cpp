#include "json.h"

#include <stdexcept>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']'){
        throw ParsingError("ParsingError exception is expected on ']'"s);
    }

    return Node(move(result));
}

Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
//    std::cerr << s << std::endl;

    return Node(move(s));;
}

Node LoadNull(istream& input) {
    char str[5];
    input.get(str, 5);
    if(string(str) == "null"s){
        return Node();
    } else {
        throw ParsingError("Failed to read null from stream"s);
    }

}

Node LoadBool(istream& input) {
    int size = (input.peek() == 't') ? 5 : 6;
    char str[6];
    input.get(str, size);
    if(string(str) == "true"s){
        return Node(true);
    } else if (string(str) == "false"s){
        return Node(false);
    } else {
        throw ParsingError("Failed to read bool from stream"s);
    }

}

Node LoadDict(istream& input) {
    Dict result;
    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (c != '}'){
        throw ParsingError("ParsingError exception is expected on '}'"s);
    }
    return Node(move(result));
}

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":


Node LoadNode(istream& input) {
    char c;
    input >> c;
//    std::cerr << "LoadNode"s << std::endl;
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        input.putback(c);
        auto num = LoadNumber(input);
        if (holds_alternative<double>(num)){
            return Node(get<double>(num));
        }
        return Node(get<int>(num));
    }
}

}  // namespace

Node::Node(Array array)
    : value_(move(array)) {
}

Node::Node(Dict map)
    : value_(move(map)) {
}

Node::Node(double value)
    : value_(value) {
}

Node::Node(int value)
    : value_(value) {
}

Node::Node(bool value)
    : value_(value) {
}

Node::Node(std::nullptr_t null)
    : value_(null){
}

Node::Node(string str)
    : value_(move(str)) {
}

bool Node::IsArray() const{
    return holds_alternative<Array>(value_);
}
bool Node::IsBool() const{
    return holds_alternative<bool>(value_);
}
bool Node::IsDouble() const{
    return holds_alternative<double>(value_) || holds_alternative<int>(value_);
}
bool Node::IsInt() const{
    return holds_alternative<int>(value_);
}
bool Node::IsMap() const{
    return holds_alternative<Dict>(value_);
}
bool Node::IsNull() const{
    return holds_alternative<nullptr_t>(value_);
}
bool Node::IsPureDouble() const{
    return holds_alternative<double>(value_);
}
bool Node::IsString() const{
    return holds_alternative<string>(value_);
}


const Array& Node::AsArray() const {
    if (!this->IsArray()){
        throw std::logic_error("problem with call Node::AsArray() : is not array"s);
    }
    return get<Array>(value_);
}

bool Node::AsBool() const{
    if (!this->IsBool()){
        throw std::logic_error("problem with call Node::AsBool() : is not bool"s);
    }
    return get<bool>(value_);
}

double Node::AsDouble() const {
    if (!this->IsDouble()){
        throw std::logic_error("problem with call Node::AsDouble() : is not int or double"s);
    }
    if (this->IsInt()){
        return get<int>(value_) * 1.0;
    }
    return get<double>(value_);
}

int Node::AsInt() const {
    if (!this->IsInt()){
        throw std::logic_error("problem with call Node::AsInt() : is not int "s);
    }
    return get<int>(value_);
}

const Dict& Node::AsMap() const {
    if (!this->IsMap()){
        throw std::logic_error("problem with call Node::AsMap() : is not map"s);
    }
    return get<Dict>(value_);
}

const string& Node::AsString() const {
    if (!this->IsString()){
        throw std::logic_error("problem with call Node::AsString() : is not string"s);
    }
    return get<string>(value_);
}

const Node::Value& Node::GetValue() const{
    return value_;
}

bool operator== (const json::Node& lhs, const json::Node& rhs){
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!= (const json::Node& lhs, const json::Node& rhs){
    return lhs.GetValue() != rhs.GetValue();
}
// ------- Document -------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator== (const json::Document& lhs, const json::Document& rhs){
    return lhs.GetRoot() == rhs.GetRoot();
}
bool operator!= (const json::Document& lhs, const json::Document& rhs){
    return lhs.GetRoot() != rhs.GetRoot();
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

// ------ Print ------

PrintContext::PrintContext (std::ostream& out)
    : out(out) {
}

PrintContext::PrintContext (std::ostream& out, int indent_step, int indent)
    : out(out)
    , indent_step(indent_step)
    , indent(indent) {
}

PrintContext  PrintContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void PrintContext::PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.PrintIndent();
    ctx.out << "null"sv;
}
// Перегрузка функции PrintValue для вывода значений string
void PrintValue(const std::string& str, const PrintContext& ctx) {
//    cerr << str << endl;
    ctx.PrintIndent();
    ctx.out << "\""sv;
    for (const auto& c : str){
        if (c == '\\'){
            ctx.out << '\\';
        } else if (c == '\"'){
            ctx.out << '\\';
        } else if (c == '\r'){
            ctx.out << '\\';
            ctx.out << 'r';
            continue;
        } else if (c == '\n'){
            ctx.out << '\\';
            ctx.out << 'n';
            continue;
        }
        ctx.out << c;
    }
    ctx.out << "\""sv;
}

void PrintValue(bool b, const PrintContext& ctx) {
    ctx.PrintIndent();
    ctx.out << std::boolalpha << b;
}

void PrintValue(Array arr, const PrintContext& ctx) {
    ctx.PrintIndent();
    ctx.out << "[";
    auto n_ctx = ctx.Indented();
    bool is_first = true;
    bool is_map = false;
    for(const auto& node : arr){
        if(is_first){
            if (node.IsMap()){
                ctx.out << "\n";
                PrintNode(node, n_ctx);
                is_map = true;
            } else{
                PrintNode(node, PrintContext(ctx));
            }
            is_first = false;
            continue;
        }
        node.IsMap() ? ctx.out << ",\n" : ctx.out << ", ";
        node.IsMap() ? PrintNode(node, n_ctx) : PrintNode(node, PrintContext(ctx));
    }
    if(is_map){
        ctx.out << "\n";
    }
    ctx.PrintIndent();
    ctx.out << "]";

}

void PrintValue(Dict dict, const PrintContext& ctx) {
    ctx.PrintIndent();
    ctx.out << "{\n";
    auto n_ctx = ctx.Indented();
    bool is_first = true;
    for(const auto& [key, node] : dict){

        if(!is_first){
            ctx.out << ",\n";
        }
        PrintNode(key, n_ctx);
        ctx.out << ": "sv;
        PrintNode(node, PrintContext(ctx.out));
        is_first = false;
    }
    ctx.out << "\n";
    ctx.PrintIndent();
    ctx.out << "}";
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext(output));
}

}  // namespace json
