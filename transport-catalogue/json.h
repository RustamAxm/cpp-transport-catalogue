#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using namespace std::string_literals;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
   /* Реализуйте Node, используя std::variant */
    Node() = default;
    Node(int value);
    Node(double value);
    Node(std::string value);
    Node(bool value);
    Node(Array array);
    Node(Dict map);
    Node(std::nullptr_t);

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    const Value& GetValue() const;

private:
    Value value_;
};

    bool operator==(const Node& left, const Node& right);
    bool operator!=(const Node& left, const Node& right);


class Document {
public:
    explicit Document() = default;
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

    bool operator==(const Document& left, const Document& right);
    bool operator!=(const Document& left, const Document& right);


Document Load(std::istream& input);

struct NodeValuePrinter {
    std::ostream& out;
    void operator()(std::nullptr_t) const;
    void operator()(const Array& array) const;
    void operator()(const Dict& dict) const;
    void operator()(bool value) const;
    void operator()(int value) const;
    void operator()(double value) const;
    void operator()(const std::string& value) const;
};

void Print(const Document& doc, std::ostream& output);

}  // namespace json