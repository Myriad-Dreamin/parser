
#include <iostream>
#include <vector>
#include <map>


template<typename T>
const char *stringify(T v);

template<typename token_t, typename uterm_t>
struct Node {
    union Data {
        token_t token;
        uterm_t uterm;
    } data;
    bool ut;

    Node(token_t x) {
        data.token = x;
        ut = false;
    }
    Node(uterm_t x, bool) {
        data.uterm = x;
        ut = true;
    }

    Node *&insert(Node * chx) {
        ch.push_back(chx);
        return ch.back();
    }
    
    std::vector<Node*> ch;
    template<typename u, typename v>
    friend std::ostream &operator<<(std::ostream & os, Node<u, v> &res);
};

template<typename token_t, typename uterm_t>
std::ostream &operator<<(std::ostream & os, Node<token_t, uterm_t> &res) {
    if (res.ut) {
        os << "{" << stringify(res.data.uterm) << ", {";
    } else {
        os << "{" << stringify(res.data.token) << ", {";
    }
    for (auto &chx: res.ch) {
        os << *chx << ", ";
    }
    os << "}}";

    return os;
}

enum class ResultCode : uint16_t {
    Ok,
    Error,
};

template<>
const char *stringify(ResultCode x) {
    switch (x){
    case ResultCode::Ok:
    return "Ok";
    case ResultCode::Error:
    return "Error";
    default:
    return "ResultCode<NotFound>";
    }
}

// template<typename token_t, class Source, class TokenTable>
// class Parser;

template<typename token_t, typename uterm_t>
struct Result {
    template<typename parser_token_t, class Source, class TokenTable>
    friend class Parser;
    using Node = Node<token_t, uterm_t>;
    Node *rt;
    ResultCode code;
    virtual ~Result() {
        release();
    }
    void release() {
        for (auto &node: nodes) {
            delete node;
        }
        nodes.clear();
        rt = nullptr;
    }
    protected:
    std::vector<Node*> nodes;
    template<typename T>
    Node *alloc(T x) {
        Node *n = new Node(x);
        nodes.push_back(n);
        return n;
    }
    template<typename T>
    Node *alloc(T x, bool y) {
        Node *n = new Node(x,y);
        nodes.push_back(n);
        return n;
    }

    template<typename u, typename v>
    friend std::ostream &operator<<(std::ostream & os, Result<u, v> &res);
};

template<typename token_t, typename uterm_t>
std::ostream &operator<<(std::ostream & os, Result<token_t, uterm_t> &res) {
    os << "{ code: " << stringify(res.code) << ", node: ";
    os << *res.rt;
    os << "}";
    return os;
}

enum class Token: uint16_t {
    Add,
    Sub,
    Mul,
    Div,
    Lbr,
    Rbr,
    Num,
    Eof = 0xffff,
};

template<>
const char *stringify(Token x) {
    switch (x){
    case Token::Add:
    return "Add";
    case Token::Sub:
    return "Sub";
    case Token::Mul:
    return "Mul";
    case Token::Div:
    return "Div";
    case Token::Lbr:
    return "Lbr";
    case Token::Rbr:
    return "Rbr";
    case Token::Num:
    return "Num";
    case Token::Eof:
    return "Eof";
    default:
    return "Token<NotFound>";
    }
}

enum class UTerm: uint16_t {
    E,
    ED,
    T,
    TD,
    F,
};

template<>
const char *stringify(UTerm x) {
    switch (x){
    case UTerm::E:
    return "E";
    case UTerm::ED:
    return "ED";
    case UTerm::T:
    return "T";
    case UTerm::TD:
    return "TD";
    case UTerm::F:
    return "F";
    default:
    return "UTerm<NotFound>";
    }
}

struct token_adapt_example {
    static const Token add = Token::Add;
    static const Token sub = Token::Sub;
    static const Token mul = Token::Mul;
    static const Token div = Token::Div;
    static const Token lbr = Token::Lbr;
    static const Token rbr = Token::Rbr;
    static const Token num = Token::Num;
    static const Token eof = Token::Eof;
};

using token_tt = std::underlying_type_t<Token>;
template<typename token_t, class Source, class TokenTable>
class Parser {
    using istream = Source;
    using result_t = Result<token_t, UTerm>;
    using node_t = Node<token_t, UTerm>;
    istream &ref;
    token_t token;

    result_t *result;
    public:
    Parser(istream &ref): ref(ref) {}
    result_t* parse() {
        result = new result_t();
        result->code = ResultCode::Ok;
        auto hdl = result;
        read();
        parseE(result->rt = result->alloc(UTerm::E, true));
        result = nullptr;
        return hdl;
    }
    private:
    void read() { ref >> token; }
    void error() {
        result->code = ResultCode::Error;
        std::cout << "error " << stringify(token) << std::endl;
    }
    void parseE(node_t* &rt) {
        parseT(rt->insert(result->alloc(UTerm::T, true)));
        if (rt->ch.back() == nullptr) {
            rt->ch.pop_back();
        }
        if (token != TokenTable::eof) {
            parseED(rt->insert(result->alloc(UTerm::ED, true)));
            if (rt->ch.back() == nullptr) {
                rt->ch.pop_back();
            }
        }
        if (rt->ch.empty()) {
            rt = nullptr;
        }
    }
    void parseED(node_t* &rt) {
        if (token == TokenTable::add || token == TokenTable::sub) {
            rt->insert(result->alloc(token));
            read();
            parseT(rt->insert(result->alloc(UTerm::T, true)));
            if (rt->ch.back() == nullptr) {
                rt->ch.pop_back();
            }
            parseED(rt->insert(result->alloc(UTerm::ED, true)));
            if (rt->ch.back() == nullptr) {
                rt->ch.pop_back();
            }
        }
        if (rt->ch.empty()) {
            rt = nullptr;
        }
    }
    void parseT(node_t* &rt) {
        parseF(rt->insert(result->alloc(UTerm::F, true)));
        if (rt->ch.back() == nullptr) {
            rt->ch.pop_back();
        }
        if (token != TokenTable::eof) {
            parseTD(rt->insert(result->alloc(UTerm::TD, true)));
            if (rt->ch.back() == nullptr) {
                rt->ch.pop_back();
            }
        }
        if (rt->ch.empty()) {
            rt = nullptr;
        }
    }
    void parseTD(node_t* &rt) {
        if (token == TokenTable::mul || token == TokenTable::div) {
            rt->insert(result->alloc(token));
            read();
            parseF(rt->insert(result->alloc(UTerm::F, true)));
            if (rt->ch.back() == nullptr) {
                rt->ch.pop_back();
            }
            parseTD(rt->insert(result->alloc(UTerm::TD, true)));
            if (rt->ch.back() == nullptr) {
                rt->ch.pop_back();
            }
        }
        if (rt->ch.empty()) {
            rt = nullptr;
        }
    }
    void parseF(node_t* &rt) {
        if (token == TokenTable::lbr) {
            rt->insert(result->alloc(token));
            read();
            parseE(rt->insert(result->alloc(UTerm::E, true)));
            if (token == TokenTable::rbr) {
                rt->insert(result->alloc(token));
                read();
            } else {
                error();
            }
        } else if (token == TokenTable::num) {
            rt->insert(result->alloc(token));
            read();
        } else {
            error();
        }
    }
};


struct LexerResult{
    using vector = std::vector<Token>;
    vector tokens;
    int64_t ofs;
    LexerResult() {ofs = 0;}
    LexerResult(vector &in) {
        tokens.swap(in);
    }
    void reset() {ofs = 0;}
    LexerResult &operator>>(Token &b) {
        if (ofs == tokens.size()) {
            b = Token::Eof;
            return *this;
        }
        b = tokens[ofs++];
        return *this;
    }
};
using ref_pair = std::map<std::string, Token>::value_type;
std::map<std::string, Token> ref_map = {
    ref_pair("+", Token::Add),
    ref_pair("-", Token::Sub),
    ref_pair("*", Token::Mul),
    ref_pair("/", Token::Div),
    ref_pair("(", Token::Lbr),
    ref_pair(")", Token::Rbr),
    ref_pair("num", Token::Num),
    ref_pair("\xff", Token::Eof),
};

std::vector<Token> &fromIstream(std::istream &in) {
    static std::vector<Token> tokens;
    std::string x;
    tokens.clear();
    while(in >> x) {
        tokens.push_back(ref_map[x]);
    }
    return tokens;
}

// ( num )
// ( num - num * num / num )
int main() {
    auto lexer_result = LexerResult(fromIstream(std::cin));
    // for (auto x :lexer_result.tokens) {
    //     std::cout << std::underlying_type_t<Token>(x) << " ";
    // }
    // std::cout << std::endl;
    auto parser = Parser<Token, LexerResult, token_adapt_example>(lexer_result);
    auto result = parser.parse();
    std::cout << *result << std::endl;
    delete result;
}
