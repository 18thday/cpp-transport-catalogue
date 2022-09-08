#include "json.h"

#include <stdexcept>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

string LoadLiteral(istream& input) {
    string s;
    while (isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}

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
            // РџРѕС‚РѕРє Р·Р°РєРѕРЅС‡РёР»СЃСЏ РґРѕ С‚РѕРіРѕ, РєР°Рє РІСЃС‚СЂРµС‚РёР»Рё Р·Р°РєСЂС‹РІР°СЋС‰СѓСЋ РєР°РІС‹С‡РєСѓ?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Р’СЃС‚СЂРµС‚РёР»Рё Р·Р°РєСЂС‹РІР°СЋС‰СѓСЋ РєР°РІС‹С‡РєСѓ
            ++it;
            break;
        } else if (ch == '\\') {
            // Р’СЃС‚СЂРµС‚РёР»Рё РЅР°С‡Р°Р»Рѕ escape-РїРѕСЃР»РµРґРѕРІР°С‚РµР»СЊРЅРѕСЃС‚Рё
            ++it;
            if (it == end) {
                // РџРѕС‚РѕРє Р·Р°РІРµСЂС€РёР»СЃСЏ СЃСЂР°Р·Сѓ РїРѕСЃР»Рµ СЃРёРјРІРѕР»Р° РѕР±СЂР°С‚РЅРѕР№ РєРѕСЃРѕР№ С‡РµСЂС‚С‹
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // РћР±СЂР°Р±Р°С‚С‹РІР°РµРј РѕРґРЅСѓ РёР· РїРѕСЃР»РµРґРѕРІР°С‚РµР»СЊРЅРѕСЃС‚РµР№: \\, \n, \t, \r, \"
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
                    // Р’СЃС‚СЂРµС‚РёР»Рё РЅРµРёР·РІРµСЃС‚РЅСѓСЋ escape-РїРѕСЃР»РµРґРѕРІР°С‚РµР»СЊРЅРѕСЃС‚СЊ
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // РЎС‚СЂРѕРєРѕРІС‹Р№ Р»РёС‚РµСЂР°Р» РІРЅСѓС‚СЂРё- JSON РЅРµ РјРѕР¶РµС‚ РїСЂРµСЂС‹РІР°С‚СЊСЃСЏ СЃРёРјРІРѕР»Р°РјРё \r РёР»Рё \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // РџСЂРѕСЃС‚Рѕ СЃС‡РёС‚С‹РІР°РµРј РѕС‡РµСЂРµРґРЅРѕР№ СЃРёРјРІРѕР» Рё РїРѕРјРµС‰Р°РµРј РµРіРѕ РІ СЂРµР·СѓР»СЊС‚РёСЂСѓСЋС‰СѓСЋ СЃС‚СЂРѕРєСѓ
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
    if(string(str) == "true"sv){
        return Node(true);
    } else if (string(str) == "false"sv){
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

    // РЎС‡РёС‚С‹РІР°РµС‚ РІ parsed_num РѕС‡РµСЂРµРґРЅРѕР№ СЃРёРјРІРѕР» РёР· input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // РЎС‡РёС‚С‹РІР°РµС‚ РѕРґРЅСѓ РёР»Рё Р±РѕР»РµРµ С†РёС„СЂ РІ parsed_num РёР· input
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
    // РџР°СЂСЃРёРј С†РµР»СѓСЋ С‡Р°СЃС‚СЊ С‡РёСЃР»Р°
    if (input.peek() == '0') {
        read_char();
        // РџРѕСЃР»Рµ 0 РІ JSON РЅРµ РјРѕРіСѓС‚ РёРґС‚Рё РґСЂСѓРіРёРµ С†РёС„СЂС‹
    } else {
        read_digits();
    }

    bool is_int = true;
    // РџР°СЂСЃРёРј РґСЂРѕР±РЅСѓСЋ С‡Р°СЃС‚СЊ С‡РёСЃР»Р°
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // РџР°СЂСЃРёРј СЌРєСЃРїРѕРЅРµРЅС†РёР°Р»СЊРЅСѓСЋ С‡Р°СЃС‚СЊ С‡РёСЃР»Р°
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
            // РЎРЅР°С‡Р°Р»Р° РїСЂРѕР±СѓРµРј РїСЂРµРѕР±СЂР°Р·РѕРІР°С‚СЊ СЃС‚СЂРѕРєСѓ РІ int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // Р’ СЃР»СѓС‡Р°Рµ РЅРµСѓРґР°С‡Рё, РЅР°РїСЂРёРјРµСЂ, РїСЂРё РїРµСЂРµРїРѕР»РЅРµРЅРёРё,
                // РєРѕРґ РЅРёР¶Рµ РїРѕРїСЂРѕР±СѓРµС‚ РїСЂРµРѕР±СЂР°Р·РѕРІР°С‚СЊ СЃС‚СЂРѕРєСѓ РІ double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// РЎС‡РёС‚С‹РІР°РµС‚ СЃРѕРґРµСЂР¶РёРјРѕРµ СЃС‚СЂРѕРєРѕРІРѕРіРѕ Р»РёС‚РµСЂР°Р»Р° JSON-РґРѕРєСѓРјРµРЅС‚Р°
// Р¤СѓРЅРєС†РёСЋ СЃР»РµРґСѓРµС‚ РёСЃРїРѕР»СЊР·РѕРІР°С‚СЊ РїРѕСЃР»Рµ СЃС‡РёС‚С‹РІР°РЅРёСЏ РѕС‚РєСЂС‹РІР°СЋС‰РµРіРѕ СЃРёРјРІРѕР»Р° ":


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
bool Node::IsDict() const{
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

const Dict& Node::AsDict() const {
    if (!this->IsDict()){
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

Node::Value& Node::GetValue() {
    return value_;
}

//bool Node::operator== (const json::Node& rhs) const{
//    return GetValue() == rhs.GetValue();
//}
//
//bool Node::operator!= (const json::Node& rhs) const{
//    return GetValue() != rhs.GetValue();
//}

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

PrintContext PrintContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void PrintContext::PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

// Рrint null
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    //ctx.PrintIndent();
    ctx.out << "null"sv;
}
// Print string
void PrintValue(const std::string& str, const PrintContext& ctx) {
//    cerr << str << endl;
    //ctx.PrintIndent();
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
    //ctx.PrintIndent();
    ctx.out << std::boolalpha << b;
}

void PrintValue(const Array& arr, const PrintContext& ctx) {
    //ctx.PrintIndent();
    ctx.out << "[\n";
    auto n_ctx = ctx.Indented();
    bool is_first = true;
    for(const auto& node : arr){
        if(is_first){
            is_first = false;
        } else{
            ctx.out << ",\n";
        }
        n_ctx.PrintIndent();
        //node.IsDict() ? ctx.out << ",\n" : ctx.out << ", ";
        PrintNode(node, n_ctx);
    }
    ctx.out << "\n";
    ctx.PrintIndent();
    ctx.out << "]";

}

void PrintValue(const Dict& dict, const PrintContext& ctx) {
    //ctx.PrintIndent();
    ctx.out << "{\n";
    auto n_ctx = ctx.Indented();
    bool is_first = true;
    for(const auto& [key, node] : dict){
        if(is_first){
            is_first = false;
        } else{
            ctx.out << ",\n";
        }
        n_ctx.PrintIndent();
        PrintNode(key, ctx);
        ctx.out << ": "sv;
        PrintNode(node, n_ctx);

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
