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
class CommonContext;


class Builder{
public:
    KeyContext Key(std:: string key);
    BuilderContext Value(Node::Value value);
    DictContext StartDict();
    CommonContext EndDict();
    ArrayContext StartArray();
    CommonContext EndArray();
    Node Build();
private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    bool empty_json_builder = true;
    bool need_be_build = false;
};




class BuilderContext{
public:
    BuilderContext(Builder& builder)
        : builder_(builder){
    }

    KeyContext Key(std::string key);
    //BuilderContext Value(Node::Value value);
    DictContext StartDict();
    CommonContext EndDict();
    ArrayContext StartArray();
    CommonContext EndArray();
    Node Build();
//private:
    Builder& builder_;
};

class KeyContext final : public BuilderContext {
public:
    KeyContext(Builder& builder)
        : BuilderContext(builder){
    }
    KeyContext Key(std:: string key) = delete;
    CommonContext EndDict() = delete;
    CommonContext EndArray() = delete;
    Node Build() = delete;

    KeyValueContext Value(Node::Value value);
};

class KeyValueContext final : public BuilderContext {
public:
    KeyValueContext(Builder& builder)
        : BuilderContext(builder){
    }
    DictContext StartDict() = delete;
    ArrayContext StartArray() = delete;
    CommonContext EndArray() = delete;
    Node Build() = delete;
};

class DictContext final : public BuilderContext {
public:
    DictContext(Builder& builder)
        : BuilderContext(builder){
    }
    //BuilderContext Value(Node::Value value) = delete;
    DictContext StartDict() = delete;
    ArrayContext StartArray() = delete;
    CommonContext EndArray() = delete;
    Node Build() = delete;
};

class ArrayContext final : public BuilderContext {
public:
    ArrayContext(Builder& builder)
        : BuilderContext(builder){
    }
    KeyContext Key(std:: string key) = delete;
    CommonContext EndDict() = delete;
    Node Build() = delete;

    ArrayContext Value(Node::Value value);

};


class CommonContext final : public BuilderContext {
public:
    CommonContext(Builder& builder)
        : BuilderContext(builder){
    }
    CommonContext Value(Node::Value value);
};

} // namespace json
