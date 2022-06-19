#include "json.h"

namespace json {

    class Builder;
    class DictItemContext;
    class KeyItemContext;
    class KeyValueItemContext;
    class ArrayItemContext;
    class ArrayValueItemContext;

    class BaseContext {
    public:
        BaseContext(Builder& builder);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        KeyItemContext Key(std::string key);

        Builder& Value(json::Node value);

        Builder& EndDict();

        Builder& EndArray();

    private:
        Builder& builder_;
    };

    class KeyItemContext : public BaseContext {
    public:
        KeyItemContext(Builder& builder);

        Builder& EndDict() = delete;

        Builder& EndArray() = delete;

        Builder& Key(std::string key) = delete;

        KeyValueItemContext Value(Node value);
    };

    class KeyValueItemContext : public BaseContext {
    public:
        KeyValueItemContext(Builder& builder);

        DictItemContext StartDict() = delete;

        ArrayItemContext StartArray() = delete;

        Builder& EndArray() = delete;

        Builder& Value(Node value) = delete;
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(Builder& builder);

        DictItemContext StartDict() = delete;

        ArrayItemContext StartArray() = delete;

        Builder& EndArray() = delete;

        Builder& Value(Node value) = delete;
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(Builder& builder);

        KeyItemContext Key(std::string key) = delete;

        ArrayValueItemContext Value(Node value);

        Builder& EndDict() = delete;
    };

    class ArrayValueItemContext : public BaseContext {
    public:
        ArrayValueItemContext(Builder& builder);

        Builder& EndDict() = delete;

        KeyItemContext Key(std::string key) = delete;

        ArrayValueItemContext Value(Node value);

    };

    class Builder {
    public:
        Builder();

        KeyItemContext Key(std::string string);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder& Value(Node value);

        Builder& EndDict();

        Builder& EndArray();

        json::Node Build();

        ~Builder();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        bool NotValidNode();
    };
}

