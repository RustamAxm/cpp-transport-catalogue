#include "json_builder.h"

namespace json {

    BaseContext::BaseContext(Builder &builder) : builder_(builder) {}

    DictItemContext BaseContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext BaseContext::StartArray() {
        return builder_.StartArray();
    }

    Builder &BaseContext::EndArray() {
        return builder_.EndArray();
    }

    Builder &BaseContext::EndDict() {
        return builder_.EndDict();
    }

    KeyItemContext BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder &BaseContext::Value(Node value) {
        return builder_.Value(std::move(value));
    }

    // KeyItemContext

    KeyItemContext::KeyItemContext(Builder &builder) : BaseContext(builder) {}

    KeyValueItemContext KeyItemContext::Value(Node value) {
        return BaseContext::Value(std::move(value));
    }

    // KeyValueItemContext

    KeyValueItemContext::KeyValueItemContext(Builder &builder) : BaseContext(builder) {}


    // DictItemContext

    DictItemContext::DictItemContext(Builder &builder) : BaseContext(builder) {}


    // ArrayItemContext

    ArrayItemContext::ArrayItemContext(Builder &builder) : BaseContext(builder) {}

    ArrayValueItemContext ArrayItemContext::Value(Node value) {
        return BaseContext::Value(move(value));
    }

    // ArrayValueItemContext

    ArrayValueItemContext::ArrayValueItemContext(Builder &builder) : BaseContext(builder) {}

    ArrayValueItemContext ArrayValueItemContext::Value(Node value) {
        return BaseContext::Value(move(value));
    }

    // Builder

    Builder::Builder() {
        nodes_stack_.emplace_back(&root_);
    }

    KeyItemContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() ||
            !nodes_stack_.back()->IsMap()) {
            throw std::logic_error("cant build key");
        }
        nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsMap())[key]);
        return *this;
    }

    Builder& Builder::Value(Node value ) {
        if (NotValidNode()) {
            throw std::logic_error("cant emplace value");
        }
        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(value);
        } else {
            *nodes_stack_.back() = value;
            nodes_stack_.pop_back();
        }
        return *this;
    }

    DictItemContext Builder::StartDict() {
        if (NotValidNode()) {
            throw std::logic_error("cant start Dictionary");
        }
        if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Dict()));

        } else {
            *nodes_stack_.back() = Dict();
        }
        return *this;
    }

    ArrayItemContext Builder::StartArray() {
        if (NotValidNode()) {
            throw std::logic_error("cant start Array");
        }
        if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Array()));
        } else {
            *nodes_stack_.back() = Array();
        }
        return *this;
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty() ||
            !nodes_stack_.back()->IsMap()) {
            throw std::logic_error("this node is not Dictionary");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty() ||
            !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("this node is not Array");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    json::Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("cant build node");
        }
        return root_;
    }

    Builder::~Builder() = default;

    bool Builder::NotValidNode() {
        return (nodes_stack_.empty() ||
                (!nodes_stack_.back()->IsNull() &&
                 !nodes_stack_.back()->IsArray()));
    }
}