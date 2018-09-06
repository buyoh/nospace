#pragma GCC optimize ("O3")
#include "bits/stdc++.h"


using namespace std;
using integer = int64_t;


template<typename T, typename U>
inline bool typeis(const U& a1) {
    return typeid(a1) == typeid(T);
}


namespace WS {

    enum struct Chr {
        SP = 0, // space
        TB, // tab
        LF  // line feed
    };

    struct WhiteSpace {
        vector<Chr> raw;

        WhiteSpace() :raw() {}

        inline void push(Chr c) {
            raw.push_back(c);
        }
        inline void push(initializer_list<Chr> cl) {
            for (auto c : cl) push(c);
        }
        template<size_t size>
        inline void push(const Chr(&cl)[size]) {
            for (auto c : cl) push(c);
        }
        inline void push(int c) {
            raw.push_back(static_cast<Chr>(c));
        }
        inline void push(initializer_list<int> cl) {
            for (auto c : cl) push(static_cast<Chr>(c));
        }
        inline void push(const WhiteSpace& ws) {
            raw.insert(raw.end(), ws.raw.begin(), ws.raw.end());
        }

        inline void print(ostream& os) const {
            static const char convert[] = { 32,9,10,0,0,0,0,0 };
            for (auto c : raw)
                os << convert[static_cast<size_t>(c)];
        }
    };
    inline ostream& operator <<(ostream& os, WhiteSpace& ws) { ws.print(os); return os; }


    namespace Instruments {
        namespace Stack {
            const Chr push[] = { Chr::SP, Chr::SP }; // prm
            const Chr duplicate[] = { Chr::SP, Chr::LF, Chr::SP };
            const Chr swap[] = { Chr::SP, Chr::LF, Chr::TB };
            const Chr discard[] = { Chr::SP, Chr::LF, Chr::LF };
        }
        namespace Arithmetic {
            const Chr add[] = { Chr::TB, Chr::SP, Chr::SP, Chr::SP };
            const Chr sub[] = { Chr::TB, Chr::SP, Chr::SP, Chr::TB };
            const Chr mul[] = { Chr::TB, Chr::SP, Chr::SP, Chr::LF };
            const Chr div[] = { Chr::TB, Chr::SP, Chr::TB, Chr::SP };
            const Chr mod[] = { Chr::TB, Chr::SP, Chr::TB, Chr::TB };
        }
        namespace Heap {
            const Chr store[] = { Chr::TB, Chr::TB, Chr::SP };
            const Chr retrieve[] = { Chr::TB, Chr::TB, Chr::TB };
        }
        namespace Flow {
            const Chr label[] = { Chr::LF, Chr::SP, Chr::SP }; // prm
            const Chr call[] = { Chr::LF, Chr::SP, Chr::TB }; // prm
            const Chr jump[] = { Chr::LF, Chr::SP, Chr::LF }; // prm
            const Chr zerojump[] = { Chr::LF, Chr::TB, Chr::SP }; // prm
            const Chr negativejump[] = { Chr::LF, Chr::TB, Chr::TB }; // prm
            const Chr retun[] = { Chr::LF, Chr::TB, Chr::LF };
            const Chr exit[] = { Chr::LF, Chr::LF, Chr::LF };
        }
        namespace IO {
            const Chr putchar[] = { Chr::TB, Chr::LF, Chr::SP, Chr::SP };
            const Chr putnumber[] = { Chr::TB, Chr::LF, Chr::SP, Chr::TB };
            const Chr getchar[] = { Chr::TB, Chr::LF, Chr::TB, Chr::SP };
            const Chr getnumber[] = { Chr::TB, Chr::LF, Chr::TB, Chr::TB };
        }
    }
}


namespace Parser {

    struct ParseException : runtime_error {
        ParseException(const char* msg = "") :runtime_error(msg) { }
    };


    struct Token {
        virtual ~Token(){}
        virtual bool operator==(const Token& t) const = 0;
        virtual bool operator==(const string& s) const = 0;
    };


    class TokenStream {
        vector<unique_ptr<Token>> tokens_;
        size_t ptr;
    public:
        TokenStream():ptr(0){}
        TokenStream(vector<unique_ptr<Token>>&& _tokens) :tokens_(move(_tokens)), ptr(0) { }

        const Token& get() { return *tokens_[ptr++]; }
        TokenStream& seekg(size_t pos) { ptr = pos; return *this; }
        const Token& peek() const { return *tokens_[ptr]; }
        const bool eof() const { return ptr >= tokens_.size(); }
    };


    class TokenInteger : public Token {
        const integer value_;
    public:
        TokenInteger(integer _val):value_(_val){}

        inline integer get() const { return value_; }

        inline bool operator==(const TokenInteger& t) const {
            return value_ == t.value_;
        }
        inline bool operator==(const string& str) const {
            return false;
        }
        inline bool operator==(const Token& t) const {
            try {
                return operator==(dynamic_cast<const TokenInteger&>(t));
            }
            catch (bad_cast) {
                return false;
            }
        }
    };


    class TokenKeyword : public Token {
        const string keyword_;
    public:
        TokenKeyword(string _keyword) :keyword_(_keyword) {}

        inline bool operator==(const TokenKeyword& t) const {
            return keyword_ == t.keyword_;
        }
        inline bool operator==(const string& str) const {
            return keyword_ == str;
        }
        inline bool operator==(const Token& t) const {
            try {
                return operator==(dynamic_cast<const TokenKeyword&>(t));
            }
            catch (bad_cast) {
                return false;
            }
        }
    };


    class TokenSymbol : public Token {
        const char _symbol[4];
    public:
        TokenSymbol(char _s1) :_symbol{_s1,0,0,0} {}
        TokenSymbol(char _s1, char _s2) :_symbol{ _s1,_s2,0 } {}
        TokenSymbol(char _s1, char _s2, char _s3) :_symbol{ _s1,_s2,_s3,0 } {}

        inline bool operator==(const TokenSymbol& t) const {
            return strcmp(_symbol, t._symbol) == 0;
        }
        inline bool operator==(char c) const {
            return _symbol[0] == c && _symbol[1] == 0;
        }
        inline bool operator==(const string& str) const {
            return _symbol == str;
        }
        inline bool operator==(const Token& t) const {
            try {
                return operator==(dynamic_cast<const TokenSymbol&>(t));
            }
            catch (bad_cast) {
                return false;
            }
        }
    };


    inline bool isValidSymbol(char c) {
        static bool f[] = {0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0 };
        return (32 <= c && c < 128) ? f[c - 32] : false;
    }



    TokenInteger parseInteger(istream& is) {
        integer var = 0; integer sign = 1;
        int cc = is.peek();
        for (; (cc < '0' || '9' < cc) && cc != istream::traits_type::eof(); is.get(), cc = is.peek())
            ;// if (cc == '-') sign = -1;
        for (; '0' <= cc && cc <= '9' && cc != istream::traits_type::eof(); is.get(), cc = is.peek())
            var = (var << 3) + (var << 1) + cc - '0';
        return TokenInteger(var * sign);
    }


    // TokenKeyword parseKeyword(istream& is)


    TokenSymbol parseSymbol(istream& is) {
        return TokenSymbol(is.get());
    }


    vector<unique_ptr<Token>> parseToTokens(istream& is) {
        vector<unique_ptr<Token>> tokens;
        tokens.reserve(24);

        while (!is.eof()) {
            int cc = is.peek();

            if (isspace(cc)) {
                is.get();
                continue;
            }
            if (isdigit(cc)) {
                tokens.emplace_back(new TokenInteger(parseInteger(is)));
                continue;
            }
            if (isValidSymbol(cc)) {
                tokens.emplace_back(new TokenSymbol(parseSymbol(is)));
                continue;
            }
            is.get();
        }
        return move(tokens);
    }

}


namespace Compiler {
    using namespace Parser;


    struct CompileException : runtime_error {
        CompileException(const char* msg = "") :runtime_error(msg) { }
    };


    enum struct OperationMode {
        Integer,
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo,
        Call
    };

    // inline int OperationLevel(OperationMode mode) {
    //     static const int lev[] = { 0,4,4,5,5,2 };
    //     return lev[size_t(mode)];
    // }


    class ExpressionStatement {
        const OperationMode mode_;
        const integer val_;
        vector<unique_ptr<ExpressionStatement>> args_;

        void setup() {
            switch (mode_) {
            case OperationMode::Add:
            case OperationMode::Subtract:
            case OperationMode::Multiply:
            case OperationMode::Divide:
            case OperationMode::Modulo:
                args_.resize(2);
                break;
            case OperationMode::Integer:
            case OperationMode::Call:
                break;
            }
        }
    public:
        ExpressionStatement(OperationMode _mode = OperationMode::Integer, integer _val = 0)
            : mode_(_mode), val_(_val) {
            setup();
        }

        inline decltype(val_) val() const { return val_; }
        inline decltype(mode_) mode() const { return mode_; }

        inline decltype(args_)::value_type& args(int i) { return args_[i]; }
        inline const decltype(args_)::value_type& args(int i) const { return args_[i]; }

        inline ExpressionStatement& operator[](int i) { return *args_[i]; }
        inline const ExpressionStatement& operator[](int i) const { return *args_[i]; }

        inline void resizeArgs(int size) { args_.resize(size); }


    };

       
    ExpressionStatement getStatement(TokenStream& stream, int level) {

        unique_ptr<ExpressionStatement> root;
        unique_ptr<ExpressionStatement>* curr = &root;

        integer intsign = 1;
        int stat = 0;
        while (!stream.eof()) {
            const Token& token = stream.peek();

            switch (stat) {
            case 0:
            case 2: {
                if (typeis<TokenInteger>(token)) {
                    assert(!(*curr));
                    if (level < 3) {
                        curr->reset(new ExpressionStatement(getStatement(stream, level + 1)));
                        stat = 1;
                        break;
                    }
                    else if (level == 3) {
                        integer val = dynamic_cast<const TokenInteger&>(token).get();
                        stream.get();
                        return ExpressionStatement(OperationMode::Integer, intsign*val);
                    }
                }
                if (typeis<TokenSymbol>(token)) {
                    const auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);
                    assert(!(*curr));

                    if (tokenSymbol == '(') {
                        stream.get();
                        curr->reset(new ExpressionStatement(getStatement(stream, 0)));
                        if (stream.peek() == ")") {
                            stat = 1;
                            stream.get();
                            break;
                        }
                        else
                            throw CompileException();
                    }
                    if (level < 3) { // todo: level
                        if (tokenSymbol == '-') {
                            curr->reset(new ExpressionStatement(getStatement(stream, level + 1)));
                            stat = 1;
                            break;
                        }
                    }
                    else if (level == 3) {
                        if (tokenSymbol == '-') {
                            stream.get();
                            intsign *= -1;
                            break;
                        }
                    }
                }
                throw CompileException();
            }
            case 1: {
                if (typeis<TokenSymbol>(token)) {
                    const auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);

                    if (tokenSymbol == '+' || tokenSymbol == '-') {

                        if (level < 1) {
                            throw CompileException("internal error?");
                        }
                        else if (level == 1) {
                            auto new_ex = new ExpressionStatement(
                                tokenSymbol == '-' ? OperationMode::Subtract : OperationMode::Add);
                            (*new_ex).args(0) = move(root);
                            root.reset(new_ex);
                            curr = &(*new_ex).args(1);
                            stream.get();
                            stat = 2;
                            break;
                        }
                        else if (level > 1) {
                            return move(*root);
                        }
                    }
                    else if (tokenSymbol == '*' || tokenSymbol == '/' || tokenSymbol == '%') {
                        if (level < 2) {
                            throw CompileException("internal error?");
                        }
                        else if (level == 2) {
                            auto new_ex = new ExpressionStatement(
                                tokenSymbol == '/' ? OperationMode::Divide :
                                tokenSymbol == '%' ? OperationMode::Modulo :
                                OperationMode::Multiply);
                            (*new_ex).args(0) = move(root);
                            root.reset(new_ex);
                            curr = &(*new_ex).args(1);
                            stream.get();
                            stat = 2;
                            break;
                        }
                        else if (level > 2) {
                            return move(*root);
                        }
                    }
                    else if (tokenSymbol == ';' || tokenSymbol == ')') {
                        return move(*root);
                    }
                }
                throw CompileException();
            }
            }
        }
        throw CompileException();
    }


    ExpressionStatement getStatement(TokenStream& ts) {
        return getStatement(ts, 1);
    }
}


//

namespace Builder {
    using namespace WS;
    using namespace Compiler;


    struct OperatorException : runtime_error {
        OperatorException(const char* msg = "") :runtime_error(msg) { }
    };


    //

    WhiteSpace& convert_integer(WhiteSpace& whitesp, integer val) {

        // 雑
        if (val < 0) {
            whitesp.push(Chr::TB);
            val = -val;
        }
        else {
            whitesp.push(Chr::SP);

        }
        uint64_t uval = val;
        for (int i = 62; 0 <= i; --i)
            whitesp.push((uval & (1ull << i)) >> i);

        whitesp.push(Chr::LF);
        return whitesp;
    }


    WhiteSpace& convert_statement(WhiteSpace& whitesp, const ExpressionStatement& exps) {

        switch (exps.mode())
        {
        case OperationMode::Integer:
            whitesp.push(Instruments::Stack::push);
            convert_integer(whitesp, exps.val());
            return whitesp;
        case OperationMode::Add:
            convert_statement(whitesp, exps[0]);
            convert_statement(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::add);
            return whitesp;
        case OperationMode::Subtract:
            convert_statement(whitesp, exps[0]);
            convert_statement(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::sub);
            return whitesp;
        case OperationMode::Multiply:
            convert_statement(whitesp, exps[0]);
            convert_statement(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::mul);
            return whitesp;
        case OperationMode::Divide:
            convert_statement(whitesp, exps[0]);
            convert_statement(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::div);
            return whitesp;
        case OperationMode::Modulo:
            convert_statement(whitesp, exps[0]);
            convert_statement(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::mod);
            return whitesp;
        }
        throw OperatorException();
    }
}


int main() {
    using namespace WS;
    using namespace Parser;
    using namespace Compiler;
    using namespace Builder;

    TokenStream ts(parseToTokens(cin));

    WhiteSpace code;

    while (!ts.eof()) {
        ExpressionStatement st = getStatement(ts);
        convert_statement(code, st);
        code.push(Instruments::IO::putnumber);
        ts.get();
    }
    code.push(Instruments::Flow::exit);

    cout << code << flush;
    return 0;
}

