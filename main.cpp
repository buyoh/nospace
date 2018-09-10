#pragma GCC optimize ("O3")
#include "bits/stdc++.h"


using namespace std;
using integer = int64_t;


template<typename T, typename U>
inline bool typeis(const U& a1) {
    return typeid(a1) == typeid(T);
}


// todo: assert を throw に書き換える


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


    // TODO: define NameEntryVariable, NameEntryFunction, NameEntryKeyword
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


    // TODO: parentNameTable
    class NameTable {
    protected:
        unordered_map<string, unique_ptr<NameEntry>> entries_;
    private:
        integer addrH_; // heap
        integer addrL_; // label
    public:
        NameTable() :entries_(), addrH_(0), addrL_(0) { }

        const NameEntry& trymakeVariableAddr(const string& name, int length) {
            auto& p = entries_[name];
            if (p) {
                if (p->type() != EntryType::Variable)
                    throw NameException();
            }
            else {
                p.reset(new NameEntry(name, addrH_, EntryType::Variable, length));
                addrH_ += length;
            }
            return *p;
        }

        const NameEntry& trymakeFunctionAddr(const string& name) {
            auto& p = entries_[name];
            if (p) {
                if (p->type() != EntryType::Function)
                    throw NameException();
            }
            else {
                p.reset(new NameEntry(name, addrL_, EntryType::Function, 1));
                addrL_ += 1;
            }
            return *p;
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


    struct Statement {
        virtual ~Statement() { }
    };


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


    struct Factor {
        virtual ~Factor() {}
    };

    class FactorValue : public Factor {
        integer val_;
    public:
        FactorValue(integer _val) :val_(_val) { }

        inline integer& get() { return val_; }
        inline integer get() const { return val_; }
    };

    class FactorVariable : public Factor {
        integer addr_;
    public:
        FactorVariable(integer _addr) :addr_(_addr) { }

        inline integer& get() { return addr_; }
        inline integer get() const { return addr_; }
    };

    class FactorCaller : public Factor {
        integer funcid_;
    public:
        FactorCaller(integer _f) :funcid_(_f) { }

        inline integer& get() { return funcid_; }
        inline integer get() const { return funcid_; }
    };


    class Expression : public Statement {
        const OperationMode mode_;
        integer funcid_;
        unique_ptr<Factor> factor_;
        vector<unique_ptr<Expression>> args_;

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
        Expression(OperationMode _mode)
            : mode_(_mode) {
            setup();
        }
        Expression(integer _funcid) // TODO: FactorCaller&&
            : mode_(OperationMode::Call), funcid_(_funcid) {
            setup();
        }
        Expression(FactorValue&& _factor)
            : mode_(OperationMode::Value), factor_(new FactorValue(_factor)) {
            setup();
        }
        Expression(FactorVariable&& _factor)
            : mode_(OperationMode::Value), factor_(new FactorVariable(_factor)) {
            setup();
        }

        inline Factor& factor() const { return *factor_; }
        inline OperationMode mode() const { return mode_; }
        inline integer id() const { return funcid_; }

        // OperationMode::Value かつ factor が FactorVariable である
        inline bool isLvalue() const {
            return mode_ == OperationMode::Value && typeis<FactorVariable>(*factor_);
        }

        inline decltype(args_)::value_type& args(int i) { return args_[i]; }
        inline const decltype(args_)::value_type& args(int i) const { return args_[i]; }

        inline Expression& operator[](int i) { return *args_[i]; }
        inline const Expression& operator[](int i) const { return *args_[i]; }

        inline void resizeArgs(int size) { args_.resize(size); }


    };


    Expression getExpression(TokenStream& stream, NameTable& nameTable);


    // elem
    Expression getExpressionVal(TokenStream& stream, NameTable& nameTable) {

        const Token& token = stream.peek();

        if (typeis<TokenInteger>(token)) {

            integer val = dynamic_cast<const TokenInteger&>(token).get();
            stream.get();
            return Expression(FactorValue(val));
        }
        else if (typeis<TokenKeyword>(token)) {

            const auto& tokenKeyword = dynamic_cast<const TokenKeyword&>(token);

            if (reservedNameTable.include(tokenKeyword.to_string())) {

                auto& entry = reservedNameTable.get(tokenKeyword.to_string());

                if (entry.type() == EntryType::Keyword) {
                    throw CompileException();
                }
                else if (entry.type() == EntryType::Function) {
                    Expression exps(entry.address());
                    exps.resizeArgs(entry.length());

                    try {
                        stream.get();
                        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
                        for (int i = 0; i < entry.length(); ++i) {
                            exps.args(i).reset(new Expression(getExpression(stream, nameTable)));
                            if (i + 1 < entry.length())
                                assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ',');
                        }
                        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
                    }
                    catch (bad_cast) {
                        throw CompileException();
                    }
                    return move(exps);
                }
            }
            else {

                stream.get();
                const Token& nextToken = stream.peek();

                if (nextToken == "(") {
                    // function
                    auto& ref = nameTable.trymakeFunctionAddr(tokenKeyword.to_string());
                    assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
                    assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
                    return Expression(ref.address());
                }
                else {
                    // variable
                    auto& ref = nameTable.trymakeVariableAddr(tokenKeyword.to_string(), 1);
                    return Expression(FactorVariable(ref.address()));
                }

            }
        }
        else if (typeis<TokenSymbol>(token)) {
            const auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);

            if (tokenSymbol == '(') {
                stream.get();
                auto exps = getExpression(stream, nameTable);

                if (stream.get() == ")") return move(exps);
                throw CompileException();
            }

        }
        throw CompileException();
    }

    // '-'
    Expression getExpressionUnary(TokenStream& stream, NameTable& nameTable) {

        const Token& token = stream.peek();

        if (typeis<TokenSymbol>(token)) {
            const TokenSymbol& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);

            if (tokenSymbol == '-') {
                stream.get();
                auto stV = getExpressionUnary(stream, nameTable);

                if (stV.mode() == OperationMode::Value && typeis<FactorValue>(stV.factor())) {
                    dynamic_cast<FactorValue&>(stV.factor()).get() *= -1;
                    return move(stV);
                }
                else {
                    Expression stOp(OperationMode::Minus);
                    stOp.args(0).reset(new Expression(move(stV)));
                    return move(stOp);
                }
            }
            else {
                return Expression(getExpressionVal(stream, nameTable));
            }
        }
        else {
            return Expression(getExpressionVal(stream, nameTable));
        }
    }

    // '*'
    Expression getExpressionMul(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        curr->reset(new Expression(getExpressionUnary(stream, nameTable)));

        while (!stream.eof()) {
            const Token& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                const auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);

                if (tokenSymbol == '*' || tokenSymbol == '/' || tokenSymbol == '%') {
                    auto new_ex = new Expression(
                        tokenSymbol == '/' ? OperationMode::Divide :
                        tokenSymbol == '%' ? OperationMode::Modulo :
                        OperationMode::Multiply);
                    (*new_ex).args(0) = move(root);
                    root.reset(new_ex);
                    curr = &(*new_ex).args(1);
                    stream.get();
                }
                else {
                    break;
                }
            }
            else {
                throw CompileException();
            }
            curr->reset(new Expression(getExpressionUnary(stream, nameTable)));
        }
        return move(*root);
    }

    // '+'
    Expression getExpressionPls(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        curr->reset(new Expression(getExpressionMul(stream, nameTable)));

        while (!stream.eof()) {
            const Token& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                const auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);

                if (tokenSymbol == '+' || tokenSymbol == '-') {

                    auto new_ex = new Expression(
                        tokenSymbol == '-' ? OperationMode::Subtract : OperationMode::Add);
                    (*new_ex).args(0) = move(root);
                    root.reset(new_ex);
                    curr = &(*new_ex).args(1);
                    stream.get();
                }
                else {
                    break;
                }
            }
            else {
                throw CompileException();
            }
            curr->reset(new Expression(getExpressionMul(stream, nameTable)));
        }
        return move(*root);
    }

    // '='
    Expression getExpressionAsg(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        curr->reset(new Expression(getExpressionPls(stream, nameTable)));

        while (!stream.eof()) {
            const Token& token = stream.peek();
            if (typeis<TokenSymbol>(token)) {
                const auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);

                if (tokenSymbol == '=') {
                    auto new_ex = new Expression(OperationMode::Assign);
                    (*new_ex).args(0) = move(*curr);
                    curr->reset(new_ex);
                    curr = &(*new_ex).args(1);
                    stream.get();
                }
                else {
                    break;
                }
            }
            else {
                throw CompileException();
            }
            curr->reset(new Expression(getExpressionPls(stream, nameTable)));
        }
        return move(*root);
    }


    inline Expression getExpression(TokenStream& ts, NameTable& nameTable) {
        return getExpressionAsg(ts, nameTable);
    }


    //


    unique_ptr<Statement> getStatement(TokenStream&, NameTable&, bool = false);


    //


    struct StatementScope : public Statement {
        // NameTable nameTable;
        vector<unique_ptr<Statement>> statements;
    };


    struct StatementFunction : public StatementScope {
        const NameEntry* entryPtr;

        StatementFunction(StatementScope&& _scope, const NameEntry* _entryPtr) :
            StatementScope(move(_scope)), entryPtr(_entryPtr) {}
    };


    //


    StatementScope getStatementsScope(TokenStream& stream, NameTable& parentNameTable, bool globalScope = false) {
        StatementScope localScope;
        assert(globalScope || dynamic_cast<const TokenSymbol&>(stream.get()) == '{');

        while (true) {
            if (stream.eof()) {
                if (!globalScope) throw CompileException();
                break;
            }
            auto& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                auto& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);
                if (tokenSymbol == '}') {
                    stream.get(); break;
                }
            }

            //localScope.statements.emplace_back(new Statement(getStatement(stream, localScope.nameTable)));
            localScope.statements.push_back(getStatement(stream, parentNameTable));
        }
        return move(localScope);
    }


    StatementFunction getStatementFunction(TokenStream& stream, NameTable& nameTable) {
        assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "func");
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ':');
        auto funcname = dynamic_cast<const TokenKeyword&>(stream.get());
        if (nameTable.include(funcname.to_string()) ||
            reservedNameTable.include(funcname.to_string())) {
            throw CompileException();
        }

        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
        // insert: args
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
        // insert: scope
        auto& entryRef = nameTable.trymakeFunctionAddr(funcname.to_string());
        entryRef.address();

        return StatementFunction(move(getStatementsScope(stream, nameTable)), &entryRef);
    }


    unique_ptr<Statement> getStatement(TokenStream& stream, NameTable& nameTable, bool globalScope) {
        if (stream.eof()) throw CompileException();
        auto& token = stream.peek();
        if (typeis<TokenSymbol>(token)) {
            // ...
        }
        else if (typeis<TokenKeyword>(token)) {
            auto& tokenKeyword = dynamic_cast<const TokenKeyword&>(token);
            if (tokenKeyword == "func") {
                return make_unique<StatementFunction>(getStatementFunction(stream, nameTable));
            }
        }

        if (globalScope) throw CompileException();
        auto&& p = make_unique<Expression>(getExpression(stream, nameTable));
        assert(stream.get() == ";");
        return move(p);

    }


    //

}


//


namespace Builder {
    using namespace WS;
    using namespace Compiler;


    struct OperatorException : runtime_error {
        OperatorException(const char* msg = "") :runtime_error(msg) { }
    };


    struct GenerationException : runtime_error {
        GenerationException(const char* msg = "") :runtime_error(msg) { }
    };


    //

    WhiteSpace& convertInteger(WhiteSpace& whitesp, integer val) {

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


    WhiteSpace& convertValue(WhiteSpace& whitesp, const Factor& factor) {
        if (typeis<FactorValue>(factor)) {
            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, dynamic_cast<const FactorValue&>(factor).get());
            return whitesp;
        }
        else if (typeis<FactorVariable>(factor)) {
            auto addr = dynamic_cast<const FactorVariable&>(factor).get();
            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, integer(addr));
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        throw OperatorException();
    }


    WhiteSpace& convertExpression(WhiteSpace& whitesp, const Expression& exps) {

        switch (exps.mode())
        {
        case OperationMode::Value:
            convertValue(whitesp, exps.factor());
            return whitesp;
        case OperationMode::Minus:
            whitesp.push(Instruments::Stack::push);
            whitesp.push({ Chr::SP, Chr::LF }); // zero
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Arithmetic::sub);
            return whitesp;
        case OperationMode::Add:
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::add);
            return whitesp;
        case OperationMode::Subtract:
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::sub);
            return whitesp;
        case OperationMode::Multiply:
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::mul);
            return whitesp;
        case OperationMode::Divide:
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::div);
            return whitesp;
        case OperationMode::Modulo:
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::mod);
            return whitesp;
        case OperationMode::Assign: {
            assert(exps[0].isLvalue());
            auto addr = dynamic_cast<const FactorVariable&>(exps[0].factor()).get();
            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, integer(addr));
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Heap::store);

            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, integer(addr));
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        case OperationMode::Call: {
            if (exps.id() < 0) {
                if (exps.id() == -99) { // __xyz todo: enum
                    whitesp.push(Instruments::Stack::push);
                    convertInteger(whitesp, integer(999));
                }
                else if (exps.id() == -10) { // __puti todo: enum
                    convertExpression(whitesp, exps[0]);
                    whitesp.push(Instruments::Stack::duplicate);
                    whitesp.push(Instruments::IO::putnumber);
                }
                else if (exps.id() == -11) { // __putc todo: enum
                    convertExpression(whitesp, exps[0]);
                    whitesp.push(Instruments::Stack::duplicate);
                    whitesp.push(Instruments::IO::putchar);
                }
                else {
                    throw OperatorException();
                }
            }
            else {
                whitesp.push(Instruments::Flow::call);
                convertInteger(whitesp, integer(exps.id()));
            }
            return whitesp;
        }
        }

        throw OperatorException();
    }


    WhiteSpace& convertStatement(WhiteSpace&, const Statement&);


    WhiteSpace& convertScope(WhiteSpace& whitesp, const StatementScope& scope) {
        for (auto& stat : scope.statements) {
            convertStatement(whitesp, *stat);
        }
        return whitesp;
    }


    WhiteSpace& convertFunction(WhiteSpace& whitesp, const StatementFunction& func) {
        whitesp.push(Instruments::Flow::label);
        convertInteger(whitesp, func.entryPtr->address());
        convertScope(whitesp, dynamic_cast<const StatementScope&>(func));
        whitesp.push(Instruments::Flow::retun);
        return whitesp;
    }


    WhiteSpace& convertStatement(WhiteSpace& whitesp, const Statement& stat) {
        if (typeis<StatementFunction>(stat))
            return convertFunction(whitesp, dynamic_cast<const StatementFunction&>(stat));
        if (typeis<StatementScope>(stat))
            return convertScope(whitesp, dynamic_cast<const StatementScope&>(stat));
        if (typeis<Expression>(stat))
            return convertExpression(whitesp, dynamic_cast<const Expression&>(stat));

        throw GenerationException();
    }

}



int main() {
    using namespace WS;
    using namespace Parser;
    using namespace Compiler;
    using namespace Builder;

    // embedded definition

    reservedNameTable.defineEmbeddedFunction("__xyz", -99, 0);

    reservedNameTable.defineEmbeddedFunction("__puti", -10, 1);
    reservedNameTable.defineEmbeddedFunction("__putc", -11, 1);

    // analysis

    TokenStream tokenStream(parseToTokens(cin));

    NameTable globalTable;

    StatementScope globalScope = getStatementsScope(tokenStream, globalTable, true);

    if (!globalTable.include("main"))
        throw GenerationException();
    auto& mainEntry = globalTable.get("main");
    if (mainEntry.type() != EntryType::Function)
        throw GenerationException();


    WhiteSpace code;

    // header

    code.push(Instruments::Flow::call);
    convertInteger(code, mainEntry.address());
    code.push(Instruments::Flow::exit);

    // body

    convertScope(code, globalScope);

    // flush

    cout << code << flush;
    return 0;
}
