#include "json_builder.h"

#include <utility>

using namespace std;

namespace json{

KeyContext Builder::Key(string key){
	//cerr << "Key() : begin "sv << endl;
	if (nodes_stack_.empty() || expects_build_method){
		throw std::logic_error("error Key(string key) : no opened Nodes"s);
	}
	if (!nodes_stack_.back()->IsDict()){
		throw std::logic_error("error Key(string key) : the last opened Node is not json::Dict"s);
	}
	Dict& tmp = get<Dict>(nodes_stack_.back()->GetValue());
	nodes_stack_.emplace_back(&tmp[move(key)]);
	//cerr << "\t - end"sv << endl;
	return *this;
}

Builder& Builder::Value(Node::Value value){
    if (root_.IsBool() || root_.IsDouble() || root_.IsString() || expects_build_method){
    	throw std::logic_error("Need be Build() after Value()"s);
    }
	//cerr << "Value() : begin "sv << endl;
	if (nodes_stack_.empty() && root_.IsNull()){
		expects_build_method = true;
		root_.GetValue() = move(value);
	} else if (nodes_stack_.back()->IsNull()) {
		nodes_stack_.back()->GetValue() = move(value);
		nodes_stack_.pop_back();
	} else if (nodes_stack_.back()->IsArray()) {
		get<Array>(nodes_stack_.back()->GetValue()).emplace_back(move(value));
	} else {
		throw std::logic_error("error Value(Node::Value value) : the last opened Node is not Null"s);
	}
	//cerr << "\t - end"sv << endl;
	this->empty_json_builder = false;
	return (*this);
}

DictContext Builder::StartDict(){
	if (expects_build_method){
		throw std::logic_error("Need be Build() after Value()"s);
	}
	//cerr << "StartDict() : begin "sv << endl;
	if (nodes_stack_.empty()){
		root_ = (Dict {});
		nodes_stack_.emplace_back(&root_);
	} else if (nodes_stack_.back()->IsNull() ){
		nodes_stack_.back()->GetValue() = Dict{};
	} else if (nodes_stack_.back()->IsArray()){
//		cerr << " ADD TO ARRAY ";

		get<Array>(nodes_stack_.back()->GetValue()).emplace_back(move(Dict {}));
		nodes_stack_.emplace_back(&get<Array>(nodes_stack_.back()->GetValue()).back());

	} else {
		throw std::logic_error("error StartDict() : the last opened Node is not Null or Array"s);
	}
	//cerr << "\t- end"sv << endl;
	this->empty_json_builder = false;
	return *this;
}

Builder& Builder::EndDict(){
	if (expects_build_method){
		throw std::logic_error("Need be Build() after Value()"s);
	}
	//cerr << "EndDict() : begin "sv << endl;
	if(!nodes_stack_.back()->IsDict()){
		throw std::logic_error("error EndDict() : the last opened Node is not json::Dict"s);
	}
	nodes_stack_.pop_back();
	//cerr << "\t- end"sv << endl;
	return *this;
}

ArrayContext Builder::StartArray(){
	if (expects_build_method){
		throw std::logic_error("Need be Build() after Value()"s);
	}
	this->empty_json_builder = false;
	//cerr << "StartArray() : begin "sv << endl;
	if (nodes_stack_.empty()){
		root_ = (Array {});
		nodes_stack_.emplace_back(&root_);
	} else if (nodes_stack_.back()->IsNull()){
		nodes_stack_.back()->GetValue() = Array{};
	} else if (nodes_stack_.back()->IsArray()){
		cerr << " ADD TO ARRAY ";
		get<Array>(nodes_stack_.back()->GetValue()).emplace_back(move(Array {}));
		nodes_stack_.emplace_back(&get<Array>(nodes_stack_.back()->GetValue()).back());
	} else {
		throw std::logic_error("error StartArray() : the last opened Node is not Null or Array"s);
	}
	//cerr << "\t - end"sv << endl;
	return *this;
}

Builder& Builder::EndArray(){
	if (expects_build_method){
		throw std::logic_error("Need be Build() after Value()"s);
	}
	//cerr << "EndArray() : begin "sv << endl;
	if(!(nodes_stack_.back()->IsArray())){
		throw std::logic_error("error EndArray() : the last opened Node is not json::Array"s);
	}
	nodes_stack_.pop_back();
	//cerr << "\t - end"sv << endl;
	return *this;
}

Node Builder::Build(){
	//cerr << "Build() : begin "sv << endl;
	if ( root_.IsNull() || this->empty_json_builder || !nodes_stack_.empty()){
		throw std::logic_error("error Build() : JSON is not fully build"s);
	}
	//cerr << "\t - end"sv << endl;
	return root_;
}

KeyContext BuilderContext::Key(std::string key){
	return builder_.Key(move(key));
}

KeyValueContext KeyContext::Value(Node::Value value){
	builder_.Value(move(value));
	return KeyValueContext(builder_);
}

ArrayContext ArrayContext::Value(Node::Value value){
	builder_.Value(move(value));
	return ArrayContext(builder_);
}

DictContext BuilderContext::StartDict(){
	return builder_.StartDict();
}
Builder& BuilderContext::EndDict(){
	return builder_.EndDict();
}
ArrayContext BuilderContext::StartArray(){
	return builder_.StartArray();
}
Builder& BuilderContext::EndArray(){
	return builder_.EndArray();
}

} // namespace json

