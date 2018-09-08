#pragma GCC optimize ("O3")
#include "bits/stdc++.h"


using namespace std;
using integer = int64_t;


template<typename T, typename U>
inline bool typeis(const U& a1) {
    return typeid(a1) == typeid(T);
}


struct TODOChottomatteneException : public runtime_error {
    TODOChottomatteneException() :runtime_error("") { }
};


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
        virtual ~Token() {}
        virtual bool operator==(const Token& t) const = 0;
        virtual bool operator==(const string& s) const = 0;
    };


    class TokenStream {
        vector<unique_ptr<Token>> tokens_;
        size_t ptr;
    public:
        TokenStream() :ptr(0) {}
        TokenStream(vector<unique_ptr<Token>>&& _tokens) :tokens_(move(_tokens)), ptr(0) { }

        const Token& get() { return *tokens_[ptr++]; }
        TokenStream& seekg(size_t pos) { ptr = pos; return *this; }
        const Token& peek() const { return *tokens_[ptr]; }
        const bool eof() const { return ptr >= tokens_.size(); }
    };


    class TokenInteger : public Token {
        const integer value_;
    public:
        TokenInteger(integer _val) :value_(_val) {}

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

        inline const string& to_string() const { return keyword_; }

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
        TokenSymbol(char _s1) :_symbol{ _s1,0,0,0 } {}
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
        static bool f[] = { 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0 };
        return (32 <= c && c < 128) ? f[c - 32] : false;
    }



    TokenInteger parseInteger(istream& is) {
        integer var = 0;
        int cc = is.peek();
        for (; isdigit(cc) && cc != istream::traits_type::eof(); is.get(), cc = is.peek())
            var = (var << 3) + (var << 1) + cc - '0';
        return TokenInteger(var);
    }


    TokenKeyword parseKeyword(istream& is) {
        string str;
        int cc = is.peek();
        for (; (isdigit(cc) || isalpha(cc) || cc == '_') &&
            cc != istream::traits_type::eof(); is.get(), cc = is.peek())
            str.push_back(cc);
        return TokenKeyword(str);
    }


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
            if (isalpha(cc) || cc == '_') {
                tokens.emplace_back(new TokenKeyword(parseKeyword(is)));
                continue;
            }
            if (isValidSymbol(cc)) {
                tokens.emplace_back(new TokenSymbol(parseSymbol(is)));
                continue;
            }
            is.get(); // ignore
        }
        return move(tokens);
    }

}


namespace Compiler {
    using namespace Parser;


    struct CompileException : runtime_error {
        CompileException(const char* msg = "") :runtime_error(msg) { }
    };

    struct NameException : CompileException {
        NameException(const char* msg = "") :CompileException(msg) { }
    };


    enum struct EntryType {
        Variable,
        Function,
        Keyword
    };


    class NameEntry {
        string name_;
        integer addr_;
        EntryType type_;
        int length_;
    public:
        NameEntry(const string& _name, integer _addr, EntryType _type, int _length = 1)
            :name_(_name), addr_(_addr), type_(_type), length_(_length) { }

        const string& name() const { return name_; }
        integer address() const { return addr_; }
        EntryType type() const { return type_; }
        // Variableなら配列サイズ，Functionなら引数個数
        // 1つの変数に複数の意味持たせるのは良くないよなぁ…
        integer length() const { return length_; }
    };


    class NameTable {
    protected:
        unordered_map<string, unique_ptr<NameEntry>> entries_;
    private:
        integer addrH_; // heap
        integer addrL_; // label
    public:
        NameTable() :entries_(), addrH_(0), addrL_(0) { }
        
        integer trymakeVariableAddr(const string& name, int length) {
            auto& p = entries_[name];
            if (p) {
                if (p->type() != EntryType::Variable)
                    throw NameException();
            }
            else {
                p.reset(new NameEntry(name, addrH_, EntryType::Variable, length));
                addrH_ += length;
            }
            return p->address();
        }

        integer trymakeFunctionAddr(const string& name) {
            auto& p = entries_[name];
            if (p) {
                if (p->type() != EntryType::Function)
                    throw NameException();
            }
            else {
                p.reset(new NameEntry(name, addrL_, EntryType::Function, 1));
                addrL_ += 1;
            }
            return p->address();
        }

        inline const NameEntry& get(const string& name) const {
            return *(entries_.find(name)->second);
        }

        inline bool include(const string& name) const {
            return entries_.count(name);
        }
    };


    class ReservedNameTable : private NameTable {
    public:
        ReservedNameTable() :NameTable() { }

        integer trymakeVariableAddr(const string&, int) = delete;
        integer trymakeFunctionAddr(const string&) = delete;

        // @return =0: 存在しない
        inline integer getId(const string& name) const {
            auto it = entries_.find(name);
            return it == entries_.end() ? 0 : it->second->address();
        }

        static inline bool isFunction(integer id) { return id < 0; }
        static inline bool isKeyword(integer id) { return id > 0; }

        inline void defineKeyword(const string& name, integer id) {
            assert(isKeyword(id));
            entries_[name].reset(new NameEntry(name, id, EntryType::Keyword));
        }
        inline void defineEmbeddedFunction(const string& name, integer id, int argLength = 1) {
            assert(isFunction(id));
            entries_[name].reset(new NameEntry(name, id, EntryType::Function, argLength));
        }

        inline const NameEntry& get(const string& name) const {
            return NameTable::get(name);
        }

        inline bool include(const string& name) const {
            return NameTable::include(name);
        }
    };

    ReservedNameTable reservedNameTable; // TODO: 変な場所にstatic変数


    enum struct OperationMode {
        Value,
        Minus,
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo,
        Assign,
        Call
    };

    // inline int OperationLevel(OperationMode mode) {
    //     static const int lev[] = { 0,4,4,5,5,2 };
    //     return lev[size_t(mode)];
    // }


    struct ExpressionFactor {
        virtual ~ExpressionFactor() {}
    };

    class ExpressionValue : public ExpressionFactor {
        integer val_;
    public:
        ExpressionValue(integer _val) :val_(_val) { }

        inline integer& get() { return val_; }
        inline integer get() const { return val_; }
    };

    class ExpressionVariable : public ExpressionFactor {
        integer addr_;
    public:
        ExpressionVariable(integer _addr) :addr_(_addr) { }

        inline integer& get() { return addr_; }
        inline integer get() const { return addr_; }
    };

    class ExpressionFunction : public ExpressionFactor {
        integer funcid_;
    public:
        ExpressionFunction(integer _f) :funcid_(_f) { }

        inline integer& get() { return funcid_; }
        inline integer get() const { return funcid_; }
    };


    class ExpressionStatement {
        const OperationMode mode_;
        integer funcid_;
        unique_ptr<ExpressionFactor> factor_;
        vector<unique_ptr<ExpressionStatement>> args_;

        void setup() {
            switch (mode_) {
            case OperationMode::Add:
            case OperationMode::Subtract:
            case OperationMode::Multiply:
            case OperationMode::Divide:
            case OperationMode::Modulo:
            case OperationMode::Assign:
                args_.resize(2);
                break;
            case OperationMode::Minus:
                args_.resize(1);
                break;
            case OperationMode::Value:
            case OperationMode::Call:
                break;
            }
        }
    public:
        ExpressionStatement(OperationMode _mode)
            : mode_(_mode) {
            setup();
        }
        ExpressionStatement(integer _funcid)
            : mode_(OperationMode::Call), funcid_(_funcid) {
            setup();
        }
        ExpressionStatement(ExpressionValue&& _factor)
            : mode_(OperationMode::Value), factor_(new ExpressionValue(_factor)) {
            setup();
        }
        ExpressionStatement(ExpressionVariable&& _factor)
            : mode_(OperationMode::Value), factor_(new ExpressionVariable(_factor)) {
            setup();
        }

        inline ExpressionFactor& factor() const { return *factor_; }
        inline OperationMode mode() const { return mode_; }
        inline integer id() const { return funcid_; }

        // OperationMode::Value かつ factor が ExpressionVariable である
        inline bool isLvalue() const {
            return mode_ == OperationMode::Value && typeis<ExpressionVariable>(*factor_);
        }

        inline decltype(args_)::value_type& args(int i) { return args_[i]; }
        inline const decltype(args_)::value_type& args(int i) const { return args_[i]; }

        inline ExpressionStatement& operator[](int i) { return *args_[i]; }
        inline const ExpressionStatement& operator[](int i) const { return *args_[i]; }

        inline void resizeArgs(int size) { args_.resize(size); }


    };


    void combineLeft() {

    }

    void combineRight() {

    }


    ExpressionStatement getStatement(TokenStream& stream, NameTable& nameTable, int level, int currstate = 0) {

        unique_ptr<ExpressionStatement> root;
        unique_ptr<ExpressionStatement>* curr = &root;

        // integer intsign = 1;
        int state = currstate;
        while (!stream.eof()) {
            const Token& token = stream.peek();

            switch (state) {
            case 0:
            case 2: {
                if (typeis<TokenInteger>(token)) {
                    assert(!(*curr));

                    if (level < 5) {
                        curr->reset(new ExpressionStatement(getStatement(stream, nameTable, level + 1)));
                        state = 1;
                        break;
                    }
                    else if (level == 5) {
                        integer val = dynamic_cast<const TokenInteger&>(token).get();
                        stream.get();
                        return ExpressionStatement(ExpressionValue(val));
                    }
                }
                else if (typeis<TokenKeyword>(token)) {
                    const auto& tokenKeyword = dynamic_cast<const TokenKeyword&>(token);
                    assert(!(*curr));

                    if (level < 5) {
                        curr->reset(new ExpressionStatement(getStatement(stream, nameTable, level + 1)));
                        state = 1;
                        break;
                    }
                    else if (level == 5) {
                        auto& tokenStr = tokenKeyword.to_string();

                        if (reservedNameTable.include(tokenStr)) {
                            auto& entry = reservedNameTable.get(tokenStr);

                            if (entry.type() == EntryType::Keyword) {
                                throw CompileException();
                            }
                            else if (entry.type() == EntryType::Function) {
                                ExpressionStatement exps(entry.address());
                                exps.resizeArgs(entry.length());

                                try {
                                    stream.get();
                                    assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
                                    for (int i = 0; i < entry.length(); ++i) {
                                        exps.args(i).reset(new ExpressionStatement(getStatement(stream, nameTable, 1)));
                                        if (i + 1 < entry.length())
                                            assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ',');
                                    }
                                    assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
                                }
                                catch (bad_cast) {
                                    throw CompileException();
                                }
                                return exps;
                            }
                        }
                        else {
                            integer p = nameTable.trymakeVariableAddr(tokenStr, 1);
                            stream.get();
                            return ExpressionStatement(ExpressionVariable(p));
                        }

                        throw CompileException();
                    }
                }
                else if (typeis<TokenSymbol>(token)) {
                    const auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);
                    assert(!(*curr));

                    if (tokenSymbol == '(') {
                        if (level < 6) {
                            curr->reset(new ExpressionStatement(getStatement(stream, nameTable, level + 1)));
                            state = 1;
                            break;
                        }
                        else if (level == 6) {
                            stream.get();
                            curr->reset(new ExpressionStatement(getStatement(stream, nameTable, 1)));
                            if (stream.peek() == ")") {
                                state = 1;
                                stream.get();
                                break;
                            }
                            else
                                throw CompileException();
                        }
                    }

                    if (tokenSymbol == '-') {
                        if (level < 4) {
                            curr->reset(new ExpressionStatement(getStatement(stream, nameTable, level + 1)));
                            state = 1;
                            break;
                        }
                        else if (level == 4) { // todo: level
                            stream.get();
                            auto stp = new ExpressionStatement(getStatement(stream, nameTable, level + 1));
                            if (stp->mode() == OperationMode::Value && typeis<ExpressionValue>(stp->factor())) {
                                dynamic_cast<ExpressionValue&>(stp->factor()).get() *= -1;
                                curr->reset(stp);
                            }
                            else {
                                curr->reset(new ExpressionStatement(OperationMode::Minus));
                                (*curr)->args(0).reset(stp);
                            }
                            state = 1;
                            break;
                        }
                        else {
                            throw CompileException();
                        }
                    }
                }
                throw CompileException();
            }
            case 1: {
                if (typeis<TokenSymbol>(token)) {
                    const auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);

                    if (tokenSymbol == '+' || tokenSymbol == '-') {

                        if (level < 2) {
                            throw CompileException("internal error?");
                        }
                        else if (level == 2) {
                            auto new_ex = new ExpressionStatement(
                                tokenSymbol == '-' ? OperationMode::Subtract : OperationMode::Add);
                            (*new_ex).args(0) = move(root);
                            root.reset(new_ex);
                            curr = &(*new_ex).args(1);
                            stream.get();
                            state = 2;
                            break;
                        }
                        else if (level > 2) {
                            return move(*root);
                        }
                    }
                    else if (tokenSymbol == '*' || tokenSymbol == '/' || tokenSymbol == '%') {
                        if (level < 3) {
                            throw CompileException("internal error?");
                        }
                        else if (level == 3) {
                            auto new_ex = new ExpressionStatement(
                                tokenSymbol == '/' ? OperationMode::Divide :
                                tokenSymbol == '%' ? OperationMode::Modulo :
                                OperationMode::Multiply);
                            (*new_ex).args(0) = move(root);
                            root.reset(new_ex);
                            curr = &(*new_ex).args(1);
                            stream.get();
                            state = 2;
                            break;
                        }
                        else if (level > 3) {
                            return move(*root);
                        }
                    }
                    else if (tokenSymbol == '=') {
                        if (level < 1) {
                            throw CompileException("internal error?");
                        }
                        else if (level == 1) {
                            auto new_ex = new ExpressionStatement(OperationMode::Assign);
                            (*new_ex).args(0) = move(*curr);
                            curr->reset(new_ex);
                            curr = &(*new_ex).args(1);
                            stream.get();
                            state = 2;
                            break;
                        }
                        else if (level > 1) {
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


    ExpressionStatement getStatement(TokenStream& ts, NameTable& nameTable) {
        return getStatement(ts, nameTable, 1);
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


    WhiteSpace& convert_value(WhiteSpace& whitesp, const ExpressionFactor& factor) {
        if (typeis<ExpressionValue>(factor)) {
            whitesp.push(Instruments::Stack::push);
            convert_integer(whitesp, dynamic_cast<const ExpressionValue&>(factor).get());
            return whitesp;
        }
        else if (typeis<ExpressionVariable>(factor)) {
            auto addr = dynamic_cast<const ExpressionVariable&>(factor).get();
            whitesp.push(Instruments::Stack::push);
            convert_integer(whitesp, integer(addr));
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        throw OperatorException();
    }


    WhiteSpace& convert_statement(WhiteSpace& whitesp, const ExpressionStatement& exps) {

        switch (exps.mode())
        {
        case OperationMode::Value:
            convert_value(whitesp, exps.factor());
            return whitesp;
        case OperationMode::Minus:
            whitesp.push(Instruments::Stack::push);
            whitesp.push({ Chr::SP, Chr::LF }); // zero
            convert_statement(whitesp, exps[0]);
            whitesp.push(Instruments::Arithmetic::sub);
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
        case OperationMode::Assign: {
            assert(exps[0].isLvalue());
            auto addr = dynamic_cast<const ExpressionVariable&>(exps[0].factor()).get();
            whitesp.push(Instruments::Stack::push);
            convert_integer(whitesp, integer(addr));
            convert_statement(whitesp, exps[1]);
            whitesp.push(Instruments::Heap::store);

            whitesp.push(Instruments::Stack::push);
            convert_integer(whitesp, integer(addr));
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        case OperationMode::Call: {
            if (exps.id() < 0) {
                if (exps.id() == -99) { // __xyz todo: enum
                    whitesp.push(Instruments::Stack::push);
                    convert_integer(whitesp, integer(999));
                }
                else if (exps.id() == -10) { // __puti todo: enum
                    convert_statement(whitesp, exps[0]);
                    whitesp.push(Instruments::Stack::duplicate);
                    whitesp.push(Instruments::IO::putnumber);
                }
                else if (exps.id() == -11) { // __putc todo: enum
                    convert_statement(whitesp, exps[0]);
                    whitesp.push(Instruments::Stack::duplicate);
                    whitesp.push(Instruments::IO::putchar);
                }
                else {
                    throw OperatorException();
                }
            }
            else {
                throw OperatorException();
            }
            return whitesp;
        }
        }
        
        throw OperatorException();
    }
}



int main() {
    using namespace WS;
    using namespace Parser;
    using namespace Compiler;
    using namespace Builder;

    reservedNameTable.defineEmbeddedFunction("__xyz", -99, 0);

    reservedNameTable.defineEmbeddedFunction("__puti", -10, 1);
    reservedNameTable.defineEmbeddedFunction("__putc", -11, 1);

    TokenStream ts(parseToTokens(cin));

    WhiteSpace code;
    NameTable globalTable;

    while (!ts.eof()) {
        ExpressionStatement st = getStatement(ts, globalTable);
        convert_statement(code, st);
        code.push(Instruments::Stack::discard);
        ts.get(); // ';'
    }
    code.push(Instruments::Flow::exit);

    cout << code << flush;
    return 0;
}

