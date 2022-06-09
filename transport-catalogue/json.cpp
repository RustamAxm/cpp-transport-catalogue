#include "json.h"

using namespace std;

namespace json {

namespace {

    Node LoadNode(istream& input);

    std::string LoadLiteral(std::istream& input) {
        std::string s;
        while (std::isalpha(input.peek())) {
            s.push_back(static_cast<char>(input.get()));
        }
        return s;
    }

    Node LoadArray(istream& input) {
        Array result;
        char c = '!';
        for (; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }
        if (c != ']') {
            throw ParsingError("Array parsing error");
        }

        return Node(move(result));
    }

    Node LoadNumber(std::istream& input) {
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
                    return Node(std::stoi(parsed_num));
                } catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return Node(std::stod(parsed_num));
        } catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
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

        return Node(std::move(s));
    }

    Node LoadDict(istream& input) {
        Dict result;

        char c = '!';
        for (; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.insert({move(key), LoadNode(input)});
        }
        if (c != '}') {
            throw ParsingError("Dictionary parsing error");
        }
        return Node(std::move(result));
    }

    Node LoadBool(istream& input) {
        const auto str = LoadLiteral(input);
        if (str == "true"sv) {
            return Node {true};
        } else if (str == "false") {
            return Node {false};
        } else {
            throw ParsingError("Failed bool parsing");
        }
    }

    Node LoadNull(istream& input) {
        if (auto literal = LoadLiteral(input); literal == "null"sv) {
            return Node{ nullptr };
        }
        else {
            throw ParsingError("Failed to parse null"s);
        }
    }

    Node LoadNode(istream& input) {
        char c;
        if (!(input >> c)) {
            throw ParsingError("input error"s);
        }
        switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                // go to next case
                [[fallthrough]];
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
        }
    }
}  // namespace

// constructor functions

// bool functions
bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}
bool Node::IsDouble() const {
    return std::holds_alternative<double>(*this) || IsInt();
}
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}
bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}
bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}
bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}
bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

// get functions
int Node::AsInt() const {
    if (IsInt()) {
        return get<int> (*this);
    } else {
        throw std::logic_error("cannot access to int type value");
    }
}
bool Node::AsBool() const {
    if (IsBool()) {
        return get<bool> (*this);
    } else {
        throw std::logic_error("cannot access to bool type value");
    }
}
double Node::AsDouble() const {
    if (IsPureDouble()) {
        return get<double> (*this);
    } else if (IsDouble()) {
        return get<int> (*this);
    } else {
        throw std::logic_error("cannot access to double type value");
    }
}
const std::string& Node::AsString() const {
    if (IsString()) {
        return get<std::string> (*this);
    } else {
        throw std::logic_error("cannot access to string type value");
    }
}
const Array& Node::AsArray() const {
    if (IsArray()) {
        return get<Array> (*this);
    } else {
        throw std::logic_error("cannot access to array type value");
    }
}
const Dict& Node::AsMap() const {
    if (IsMap()) {
        return get<Dict> (*this);
    } else {
        throw std::logic_error("cannot access to dictionary type value");
    }

}
const Node::Value& Node::GetValue() const {
    return *this;
}

bool operator==(const Node& left, const Node& right) {
    return left.GetValue() == right.GetValue();
}
bool operator!=(const Node& left, const Node& right) {
    return !(left == right);
}


Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator==(const Document& left, const Document& right) {
    return left.GetRoot() == right.GetRoot();
}
bool operator!=(const Document& left, const Document& right) {
    return !(left == right);
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}
// NodeValuePrinter
void NodeValuePrinter::operator()(std::nullptr_t) const {
    out << "null"sv;
}
void NodeValuePrinter::operator()(const Array& array) const {
    out << "[\n";
    for (auto it = array.begin(); it != array.end(); ++it) {
        std::visit(NodeValuePrinter{out}, it->GetValue());
        if (it != prev(array.end())) {
            out << ",\n";
        }
    }
    out << "\n]";
}
void NodeValuePrinter::operator()(const Dict& dict) const {
    out << "{\n";
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        out << "\"" << it->first << "\"" << ": ";
        std::visit(NodeValuePrinter{out}, it->second.GetValue());
        if (it != prev(dict.end())) {
            out << ",\n";
        }
    }
    out << "\n}";
}
void NodeValuePrinter::operator() (bool value) const {
    out << std::boolalpha << value;
}
void NodeValuePrinter::operator() (int value) const {
    out << value;
}
void NodeValuePrinter::operator() (double value) const {
    out << value;
}
void NodeValuePrinter::operator() (const std::string& value) const {
    out << "\""s;
    for (const char& symbol : value) {
        if (symbol == '\"') out << "\\\""s;
        else if (symbol == '\n') out << "\\n"s;
        else if (symbol == '\r') out << "\\r"s;
        else if (symbol == '\t') out << "\t"s;
        else if (symbol == '\\') out << "\\\\"s;
        else out << symbol;
    }
    out << "\""s;

}

void Print(const Document& doc, std::ostream& output) {
    std::visit(NodeValuePrinter{output}, doc.GetRoot().GetValue());
}

}  // namespace json