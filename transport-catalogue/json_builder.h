#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json{

class BuilderContext;
class KeyContext;
class KeyValueContext;
class ArrayContext;
class DictContext;
class ArrayValueContext;
//class CommonContext;


class Builder{
public:
	KeyContext Key(std:: string key);
	Builder& Value(Node::Value value);
	DictContext StartDict();
	Builder& EndDict();
	ArrayContext StartArray();
	Builder& EndArray();
	Node Build();

private:
	Node root_;
	std::vector<Node*> nodes_stack_;
	bool empty_json_builder = true;
	bool expects_build_method = false;
};




class BuilderContext{
public:
	BuilderContext(Builder& builder)
		: builder_(builder){
	}

	KeyContext Key(std::string key);
	DictContext StartDict();
	Builder& EndDict();
	ArrayContext StartArray();
	Builder& EndArray();

protected:
	Builder& builder_;
};

class KeyContext final : public BuilderContext {
public:
	KeyContext(Builder& builder)
		: BuilderContext(builder){
	}
	KeyContext Key(std:: string key) = delete;
	Builder& EndDict() = delete;
	Builder& EndArray() = delete;

	KeyValueContext Value(Node::Value value);
};

class KeyValueContext final : public BuilderContext {
public:
	KeyValueContext(Builder& builder)
		: BuilderContext(builder){
	}
	DictContext StartDict() = delete;
	ArrayContext StartArray() = delete;
	Builder& EndArray() = delete;
};

class DictContext final : public BuilderContext {
public:
	DictContext(Builder& builder)
		: BuilderContext(builder){
	}
	DictContext StartDict() = delete;
	ArrayContext StartArray() = delete;
	Builder& EndArray() = delete;
};

class ArrayContext final : public BuilderContext {
public:
	ArrayContext(Builder& builder)
		: BuilderContext(builder){
	}
	KeyContext Key(std:: string key) = delete;
	Builder& EndDict() = delete;

	ArrayContext Value(Node::Value value);
};

} // namespace json
