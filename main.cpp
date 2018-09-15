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
            const Chr store[] = { Chr::TB, Chr::TB, Chr::SP }; // push the address then the value and run the store command.
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


    namespace Embedded {
        namespace Function {
            const signed IDxyz = -999;

            const signed IDaadd = -10;
            const signed IDasub = -11;
            const signed IDamul = -12;
            const signed IDadiv = -13;
            const signed IDamod = -14;

            const signed IDputi = -100;
            const signed IDputc = -101;
            const signed IDgeti = -110;
            const signed IDgetc = -111;


        }
    }


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
    protected:
        string name_;
        integer addr_;
    public:
        NameEntry(const string& _name, integer _addr)
            :name_(_name), addr_(_addr) { }
        ~NameEntry() {}

        virtual inline const string& name() const { return name_; }
        virtual inline integer address() const { return addr_; }
    };


    class NameEntryVariable : public NameEntry {
        int length_;
    public:
        NameEntryVariable(const string& _name, integer _addr, int _length = 1)
            :NameEntry(_name, _addr), length_(_length) { }

        inline int length() const { return length_; }
    };


    class NameEntryFunction : public NameEntry {
        int argLength_;
    public:
        NameEntryFunction(const string& _name, integer _addr, int _argLength = 1)
            :NameEntry(_name, _addr), argLength_(_argLength) { }

        inline int argLength() const { return argLength_; }
    };


    class NameEntryKeyword : public NameEntry {
    public:
        NameEntryKeyword(const string& _name, integer _addr)
            :NameEntry(_name, _addr) { }
    };


    // TODO: parentNameTable
    class NameTable {
    protected:
        unordered_map<string, unique_ptr<NameEntry>> entries_;
        shared_ptr<NameTable> parent_;
    private:
        integer addrH_; // heap
        integer addrL_; // label
    public:
        NameTable()
            :entries_(), parent_(), addrH_(0), addrL_(0) { }
        NameTable(shared_ptr<NameTable>& _parent)
            :entries_(), parent_(_parent), addrH_(0), addrL_(0) { }

        const NameEntry& trymakeVariableAddr(const string& name, int length) {
            auto& p = entries_[name];
            if (p) {
                if (!typeis<NameEntryVariable>(*p))
                    throw NameException();
            }
            else {
                p.reset(new NameEntryVariable(name, addrH_, length));
                addrH_ += length;
            }
            return *p;
        }

        const NameEntry& trymakeFunctionAddr(const string& name) {
            auto& p = entries_[name];
            if (p) {
                if (!typeis<NameEntryFunction>(*p))
                    throw NameException();
            }
            else {
                p.reset(new NameEntryFunction(name, addrL_, 1));
                addrL_ += 2; // TODO: beginとend
            }
            return *p;
        }

        // 無名スコープのジャンプ(ifなど)に必要なラベルを確保する．
        inline integer reserveLabelAddr(int size) {
            return addrL_ += size;
        }

        inline integer localHeapSize() const { return addrH_; }
        inline bool isGlobal() const { return !parent_; }

        inline const NameEntry& getLocal(const string& name) const {
            return *(entries_.find(name)->second);
        }

        inline pair<const NameEntry&, const NameTable*> getEntire(const string& name) const {
            if (includeLocal(name))
                return pair<const NameEntry&, const NameTable*>(*(entries_.find(name)->second), this);
            if (!parent_) return pair<const NameEntry&, const NameTable*>(*entries_.end()->second, this);
            return parent_->getEntire(name);
        }

        inline bool includeLocal(const string& name) const {
            return entries_.count(name);
        }

        inline bool includeEntire(const string& name) const {
            return
                includeLocal(name) ? true :
                !parent_ ? false :
                parent_->includeEntire(name);
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
            entries_[name].reset(new NameEntryKeyword(name, id));
        }
        inline void defineEmbeddedFunction(const string& name, integer id, int argLength = 1) {
            assert(isFunction(id));
            entries_[name].reset(new NameEntryFunction(name, id, argLength));
        }

        inline const NameEntry& get(const string& name) const {
            return NameTable::getLocal(name);
        }

        inline bool include(const string& name) const {
            return NameTable::includeLocal(name);
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
        integer scope_; // global?: 0 else 1
        integer addr_;
    public:
        FactorVariable(integer _scope, integer _addr) :scope_(_scope), addr_(_addr) { }

        // inline integer& get() { return addr_; }
        inline integer get() const { return addr_; }
        inline integer scope() const { return scope_; }
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


    unique_ptr<Expression> getExpression(TokenStream& stream, NameTable& nameTable);


    // elem
    unique_ptr<Expression> getExpressionVal(TokenStream& stream, NameTable& nameTable) {

        const Token& token = stream.peek();

        if (typeis<TokenInteger>(token)) {

            integer val = dynamic_cast<const TokenInteger&>(token).get();
            stream.get();
            return make_unique<Expression>(FactorValue(val));
        }
        else if (typeis<TokenKeyword>(token)) {

            const auto& tokenKeyword = dynamic_cast<const TokenKeyword&>(token);

            if (reservedNameTable.include(tokenKeyword.to_string())) {

                auto& entry = reservedNameTable.get(tokenKeyword.to_string());

                if (typeis<NameEntryKeyword>(entry)) {
                    throw CompileException();
                }
                else if (typeis<NameEntryFunction>(entry)) {
                    auto& funcEntry = dynamic_cast<const NameEntryFunction&>(entry);
                    Expression exps(funcEntry.address());
                    exps.resizeArgs(funcEntry.argLength());

                    try {
                        stream.get();
                        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
                        for (int i = 0; i < funcEntry.argLength(); ++i) {
                            exps.args(i) = move(getExpression(stream, nameTable));
                            if (i + 1 < funcEntry.argLength())
                                assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ',');
                        }
                        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
                    }
                    catch (bad_cast) {
                        throw CompileException();
                    }
                    return make_unique<Expression>(move(exps));
                }
            }
            else {

                stream.get();
                const Token& nextToken = stream.peek();

                if (nextToken == "(") {
                    // function
                    if (!nameTable.includeEntire(tokenKeyword.to_string()))
                        throw CompileException();

                    const auto& ref = nameTable.getEntire(tokenKeyword.to_string());
                    if (!typeis<NameEntryFunction>(ref.first))
                        throw CompileException();

                    assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
                    assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
                    return make_unique<Expression>(ref.first.address());
                }
                else {
                    // variable
                    if (!nameTable.includeEntire(tokenKeyword.to_string()))
                        throw CompileException();

                    const auto& ref = nameTable.getEntire(tokenKeyword.to_string());
                    if (!typeis<NameEntryVariable>(ref.first))
                        throw CompileException();
                    return make_unique<Expression>(FactorVariable(ref.second->isGlobal() ? 0 : 1, ref.first.address()));
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
    unique_ptr<Expression> getExpressionUnary(TokenStream& stream, NameTable& nameTable) {

        const Token& token = stream.peek();

        if (typeis<TokenSymbol>(token)) {
            const TokenSymbol& tokenSymbol = dynamic_cast<const TokenSymbol&>(token);

            if (tokenSymbol == '-') {
                stream.get();
                auto stV = getExpressionUnary(stream, nameTable);

                if (stV->mode() == OperationMode::Value && typeis<FactorValue>(stV->factor())) {
                    // optimization
                    dynamic_cast<FactorValue&>(stV->factor()).get() *= -1;
                    return move(stV);
                }
                else {
                    Expression stOp(OperationMode::Minus);
                    stOp.args(0) = move(stV);
                    return make_unique<Expression>(move(stOp));
                }
            }
            else {
                return getExpressionVal(stream, nameTable);
            }
        }
        else {
            return getExpressionVal(stream, nameTable);
        }
    }

    // '*'
    unique_ptr<Expression> getExpressionMul(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        *curr = move(getExpressionUnary(stream, nameTable));

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
            *curr = move(getExpressionUnary(stream, nameTable));
        }
        return root;
    }

    // '+'
    unique_ptr<Expression> getExpressionPls(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        *curr = move(getExpressionMul(stream, nameTable));

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
            *curr = move(getExpressionMul(stream, nameTable));
        }
        return root;
    }

    // '='
    unique_ptr<Expression> getExpressionAsg(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        *curr = move(getExpressionPls(stream, nameTable));

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
            *curr = move(getExpressionPls(stream, nameTable));
        }
        return root;
    }


    inline unique_ptr<Expression> getExpression(TokenStream& ts, NameTable& nameTable) {
        return getExpressionAsg(ts, nameTable);
    }


    //


    unique_ptr<Statement> getStatement(TokenStream&, shared_ptr<NameTable>&, bool, bool, bool);


    //


    struct StatementScope : public Statement {
        shared_ptr<NameTable> nameTable;
        vector<unique_ptr<Statement>> statements;

        StatementScope() :nameTable(new NameTable()), statements() { }
        StatementScope(shared_ptr<NameTable>& _parentNameTable, bool)
            :nameTable(new NameTable(_parentNameTable)), statements() { }


    };


    struct StatementFunction : public StatementScope {
        integer funcLabel;
        StatementFunction(StatementScope&& _scope, integer _funcLabel) :
            StatementScope(move(_scope)), funcLabel(_funcLabel) {
        }
    };


    // TODO: 無くす方向で
    // if,while等のlet,funcが使えないスコープ
    struct StatementOpenScope : public Statement {
        weak_ptr<NameTable> parentNameTable;
        vector<unique_ptr<Statement>> statements;

        StatementOpenScope(shared_ptr<NameTable>& _parentNameTable, bool)
            :parentNameTable(_parentNameTable), statements() { }
    };

    // while文．
    struct StatementWhile : public StatementOpenScope {
        integer label;
        unique_ptr<Expression> cond;

        StatementWhile(StatementOpenScope&& _scope, unique_ptr<Expression>&& _cond, integer _label) :
            StatementOpenScope(move(_scope)), label(_label), cond(move(_cond)) {
        }
    };


    // if文
    // elsifに後続するelseを入れる．
    struct StatementIf : public StatementOpenScope {
        integer label;
        unique_ptr<Expression> cond;
        unique_ptr<StatementIf> elsif;

        StatementIf(StatementOpenScope&& _scope, unique_ptr<Expression>&& _cond, integer _label) :
            StatementOpenScope(move(_scope)), label(_label), cond(move(_cond)) {
        }

        StatementIf(StatementOpenScope&& _scope, integer _label) :
            StatementOpenScope(move(_scope)), label(_label) {
        }

        integer getLabelLast() const {
            return elsif ? elsif->getLabelLast() : label;
        }
    };


    //


    unique_ptr<StatementScope> getStatementsScope(TokenStream& stream, shared_ptr<NameTable>& parentNameTable, bool globalScope) {
        auto localScope = globalScope ?
            make_unique<StatementScope>() :
            make_unique<StatementScope>(parentNameTable, true);

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
            localScope->statements.push_back(getStatement(stream, localScope->nameTable, globalScope, !globalScope, false));
            if (!localScope->statements.back()) localScope->statements.pop_back();
        }
        return localScope;
    }


    // TODO: 無くす方向で
    // if,while等のlet,funcが使えないスコープ
    unique_ptr<StatementOpenScope> getStatementsOpenScope(TokenStream& stream, shared_ptr<NameTable>& parentNameTable) {
        auto scope = make_unique<StatementOpenScope>(parentNameTable, true);

        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '{');

        while (true) {
            if (stream.eof()) {
                throw CompileException();
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
            scope->statements.push_back(getStatement(stream, parentNameTable, false, true, true));
            if (!scope->statements.back()) scope->statements.pop_back();
        }
        return scope;
    }


    unique_ptr<Statement> getStatementLet(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "let");
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ':');
        auto& varName = dynamic_cast<const TokenKeyword&>(stream.get());
        if (nameTable->includeLocal(varName.to_string()) ||
            reservedNameTable.include(varName.to_string())) {
            throw CompileException();
        }
        nameTable->trymakeVariableAddr(varName.to_string(), 1);
        assert(stream.get() == ";");
        return unique_ptr<Statement>(); // empty
    }


    unique_ptr<StatementFunction> getStatementFunction(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "func");
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ':');
        auto& funcName = dynamic_cast<const TokenKeyword&>(stream.get());
        if (nameTable->includeLocal(funcName.to_string()) ||
            reservedNameTable.include(funcName.to_string())) {
            throw CompileException();
        }

        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
        // insert: args
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
        // insert: scope
        auto& entryRef = nameTable->trymakeFunctionAddr(funcName.to_string());

        // todo: unique_ptrの中身をmoveする操作は正しいか？
        return make_unique<StatementFunction>(move(*getStatementsScope(stream, nameTable, false)), entryRef.address());
    }


    unique_ptr<StatementWhile> getStatementWhile(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "while");
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
        auto&& condition = getExpression(stream, *nameTable);
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
        auto label = nameTable->reserveLabelAddr(2);
        return make_unique<StatementWhile>(move(*getStatementsOpenScope(stream, nameTable)), move(condition), label);
    }


    unique_ptr<StatementIf> getStatementElse(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "else");
        auto label = nameTable->reserveLabelAddr(2);
        return make_unique<StatementIf>(move(*getStatementsOpenScope(stream, nameTable)), label);
    }


    unique_ptr<StatementIf> getStatementElsif(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        // getStatementIfと全く同じですね．
        assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "elsif");
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
        auto&& condition = getExpression(stream, *nameTable);
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
        auto label = nameTable->reserveLabelAddr(2);
        auto ifscope = make_unique<StatementIf>(move(*getStatementsOpenScope(stream, nameTable)), move(condition), label);

        const auto& token = stream.peek();
        if (typeis<TokenKeyword>(token)) {
            if (token == "elsif") {
                ifscope->elsif = getStatementElsif(stream, nameTable);
                return move(ifscope);
            }
            else if (token == "else") {
                ifscope->elsif = getStatementElse(stream, nameTable);
                return move(ifscope);
            }
        }

        return move(ifscope);
    }


    unique_ptr<StatementIf> getStatementIf(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "if");
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == '(');
        auto&& condition = getExpression(stream, *nameTable);
        assert(dynamic_cast<const TokenSymbol&>(stream.get()) == ')');
        auto label = nameTable->reserveLabelAddr(2);
        auto ifscope = make_unique<StatementIf>(move(*getStatementsOpenScope(stream, nameTable)), move(condition), label);

        const auto& token = stream.peek();
        if (typeis<TokenKeyword>(token)) {
            if (token == "elsif") {
                ifscope->elsif = getStatementElsif(stream, nameTable);
                return move(ifscope);
            }
            else if (token == "else") {
                ifscope->elsif = getStatementElse(stream, nameTable);
                return move(ifscope);
            }
        }

        return move(ifscope);
    }


    unique_ptr<Statement> getStatement(TokenStream& stream, shared_ptr<NameTable>& nameTable,
        bool disableExpr, bool disableFunc, bool disableLet) {

        if (stream.eof()) throw CompileException();
        auto& token = stream.peek();
        if (typeis<TokenSymbol>(token)) {
            // ...
        }
        else if (typeis<TokenKeyword>(token)) {
            auto& tokenKeyword = dynamic_cast<const TokenKeyword&>(token);
            if (tokenKeyword == "func") {
                if (disableFunc) throw CompileException();
                return getStatementFunction(stream, nameTable);
            }
            if (tokenKeyword == "let") {
                if (disableLet) throw CompileException();
                return getStatementLet(stream, nameTable);
            }
        }

        if (disableExpr) throw CompileException();

        if (typeis<TokenSymbol>(token)) {
            // ...
        }
        else if (typeis<TokenKeyword>(token)) {
            auto& tokenKeyword = dynamic_cast<const TokenKeyword&>(token);
            if (tokenKeyword == "if") {
                return getStatementIf(stream, nameTable);
            }
            if (tokenKeyword == "while") {
                return getStatementWhile(stream, nameTable);
            }
        }

        auto&& p = getExpression(stream, *nameTable);
        assert(stream.get() == ";");
        return move(p);

    }


    //

}


//


namespace Builder {
    using namespace WS;
    using namespace Compiler;


    namespace Alignment {
        // 諸操作に必要なHeap領域．
        // [0][reserved][localBegin][localEnd][calc][calc][calc][calc]
        const integer ReservedHeapSize = 8;
        const integer LocalHeapBegin = 2;
        const integer LocalHeapEnd = 3;
        const integer TempPtr = 4;
        const integer GlobalPtr = 8;
    }


    // class CodeBuilder{}; // TODO:


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


    // WhiteSpace& convertSwap(WhiteSpace& whitesp, integer destPtr, integer fromPtr) {
    // 
    // }


    // *destPtr = *fromPtr
    WhiteSpace& convertCopy(WhiteSpace& whitesp, integer destPtr, integer fromPtr) {
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, integer(fromPtr));
        whitesp.push(Instruments::Heap::retrieve);
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, integer(destPtr));
        whitesp.push(Instruments::Stack::swap);
        whitesp.push(Instruments::Heap::store);
        return whitesp;
    }


    //


    // subroutine label の後に挿入する：
    WhiteSpace& convertLocalAllocate(WhiteSpace& whitesp, const StatementFunction& func) {
        // - local_begin を stack に積む
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, Alignment::LocalHeapBegin);
        whitesp.push(Instruments::Heap::retrieve);
        // - local_begin := local_end．(dup local_end)
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, Alignment::LocalHeapEnd);
        whitesp.push(Instruments::Heap::retrieve);
        whitesp.push(Instruments::Stack::duplicate); // dup!
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, Alignment::LocalHeapBegin);
        whitesp.push(Instruments::Stack::swap);
        whitesp.push(Instruments::Heap::store);
        // remain local_begin value on stack.
        // - local_end := local_begin(stacked by dup) + scopesize．
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, func.nameTable->localHeapSize());
        whitesp.push(Instruments::Arithmetic::add);
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, Alignment::LocalHeapEnd);
        whitesp.push(Instruments::Stack::swap);
        whitesp.push(Instruments::Heap::store);
        return whitesp;
    }


    // call の後に挿入する：
    WhiteSpace& convertLocalDeallocate(WhiteSpace& whitesp) {
        // return するとき，
        // - local_end := local_begin．
        convertCopy(whitesp, Alignment::LocalHeapEnd, Alignment::LocalHeapBegin);
        // - local_begin を stack から取り出す
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, Alignment::LocalHeapBegin);
        whitesp.push(Instruments::Stack::swap);
        whitesp.push(Instruments::Heap::store);
        return whitesp;
    }


    WhiteSpace& convertCalculateLocalVariablePtr(WhiteSpace& whitesp, integer addr) {
        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, addr);

        whitesp.push(Instruments::Stack::push);
        convertInteger(whitesp, Alignment::LocalHeapBegin);
        whitesp.push(Instruments::Heap::retrieve);

        whitesp.push(Instruments::Arithmetic::add);
        return whitesp;
    }


    WhiteSpace& convertCalculateLocalVariablePtr(WhiteSpace& whitesp, const FactorVariable& var) {

        if (var.scope() > 0)
            // local
            return convertCalculateLocalVariablePtr(whitesp, var.get());
        else {
            // global
            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, var.get() + Alignment::GlobalPtr);
            return whitesp;
        }
    }


    //


    WhiteSpace& convertExpression(WhiteSpace&, const Expression&);


    WhiteSpace& convertValue(WhiteSpace& whitesp, const Factor& factor) {
        if (typeis<FactorValue>(factor)) {
            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, dynamic_cast<const FactorValue&>(factor).get());
            return whitesp;
        }
        else if (typeis<FactorVariable>(factor)) {
            const auto& var = dynamic_cast<const FactorVariable&>(factor);
            // calculate addr
            convertCalculateLocalVariablePtr(whitesp, var);
            // retirieve
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        throw OperatorException();
    }


    WhiteSpace& convertEmbeddedExpression(WhiteSpace& whitesp, const Expression& exps) {

        if (exps.id() == Embedded::Function::IDxyz) {
            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, integer(999));
        }
        else if (exps.id() == Embedded::Function::IDputi) {
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Stack::duplicate);
            whitesp.push(Instruments::IO::putnumber);
        }
        else if (exps.id() == Embedded::Function::IDputc) {
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Stack::duplicate);
            whitesp.push(Instruments::IO::putchar);
        }
        else if (exps.id() == Embedded::Function::IDgeti) {
            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, Alignment::TempPtr);
            whitesp.push(Instruments::IO::getnumber);
            whitesp.push(Instruments::Stack::push);
            convertInteger(whitesp, Alignment::TempPtr);
            whitesp.push(Instruments::Heap::retrieve);
        }
        else {
            throw OperatorException();
        }

        return whitesp;
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
            const auto& var = dynamic_cast<const FactorVariable&>(exps[0].factor()); // thrower

            convertCalculateLocalVariablePtr(whitesp, var);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Heap::store);

            convertCalculateLocalVariablePtr(whitesp, var); // TODO: 
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        case OperationMode::Call: {
            if (exps.id() < 0) {
                convertEmbeddedExpression(whitesp, exps);
            }
            else {
                whitesp.push(Instruments::Flow::call);
                convertInteger(whitesp, integer(exps.id()));

                convertLocalDeallocate(whitesp);

                whitesp.push(Instruments::Stack::push); // always return 0
                convertInteger(whitesp, 0);
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


    WhiteSpace& convertOpenScope(WhiteSpace& whitesp, const StatementOpenScope& scope) {
        for (auto& stat : scope.statements) {
            convertStatement(whitesp, *stat);
        }
        return whitesp;
    }


    WhiteSpace& convertFunction(WhiteSpace& whitesp, const StatementFunction& func) {
        integer label = func.funcLabel;
        // TODO: insert here: goto label+1;
        whitesp.push(Instruments::Flow::label);
        convertInteger(whitesp, label);
        convertLocalAllocate(whitesp, func);

        convertScope(whitesp, dynamic_cast<const StatementScope&>(func));

        whitesp.push(Instruments::Flow::retun);

        whitesp.push(Instruments::Flow::label);
        convertInteger(whitesp, label + 1);
        return whitesp;
    }


    WhiteSpace& convertWhile(WhiteSpace& whitesp, const StatementWhile& whilestat) {
        integer label = whilestat.label;

        whitesp.push(Instruments::Flow::label);
        convertInteger(whitesp, label);

        convertExpression(whitesp, *(whilestat.cond));
        whitesp.push(Instruments::Flow::zerojump);
        convertInteger(whitesp, label + 1);

        convertOpenScope(whitesp, whilestat);

        whitesp.push(Instruments::Flow::jump); // loop
        convertInteger(whitesp, label);
        whitesp.push(Instruments::Flow::label);
        convertInteger(whitesp, label + 1);

        return whitesp;
    }


    // elsif, else も処理する
    WhiteSpace& convertIf(WhiteSpace& whitesp, const StatementIf& ifstat) {

        integer label = ifstat.label;

        whitesp.push(Instruments::Flow::label);
        convertInteger(whitesp, label);

        if (ifstat.cond) {
            convertExpression(whitesp, *(ifstat.cond));
            whitesp.push(Instruments::Flow::zerojump);
            convertInteger(whitesp, label + 1);
        }

        convertOpenScope(whitesp, ifstat);

        if (ifstat.elsif) {
            whitesp.push(Instruments::Flow::jump);
            convertInteger(whitesp, ifstat.getLabelLast() + 1);
        }

        whitesp.push(Instruments::Flow::label);
        convertInteger(whitesp, label + 1);

        if (ifstat.elsif)
            convertIf(whitesp, *(ifstat.elsif));

        return whitesp;
    }


    WhiteSpace& convertStatement(WhiteSpace& whitesp, const Statement& stat) {
        if (typeis<StatementFunction>(stat))
            return convertFunction(whitesp, dynamic_cast<const StatementFunction&>(stat));
        if (typeis<StatementScope>(stat)) // 未実装()
            return convertScope(whitesp, dynamic_cast<const StatementScope&>(stat));
        if (typeis<StatementIf>(stat))
            return convertIf(whitesp, dynamic_cast<const StatementIf&>(stat));
        if (typeis<StatementWhile>(stat))
            return convertWhile(whitesp, dynamic_cast<const StatementWhile&>(stat));
        if (typeis<Expression>(stat)) {
            convertExpression(whitesp, dynamic_cast<const Expression&>(stat));
            whitesp.push(Instruments::Stack::discard);
            return whitesp;
        }

        throw GenerationException();
    }

}



int main(int argc, char** argv) {
    using namespace WS;
    using namespace Parser;
    using namespace Compiler;
    using namespace Builder;

    // embedded definition

    reservedNameTable.defineEmbeddedFunction("__xyz", Embedded::Function::IDxyz, 0);

    reservedNameTable.defineEmbeddedFunction("__puti", Embedded::Function::IDputi, 1);
    reservedNameTable.defineEmbeddedFunction("__putc", Embedded::Function::IDputc, 1);
    reservedNameTable.defineEmbeddedFunction("__geti", Embedded::Function::IDgeti, 0);

    // analysis

    TokenStream tokenStream(parseToTokens(cin));

    shared_ptr<NameTable> dammyNT;

    StatementScope globalScope = move(*getStatementsScope(tokenStream, dammyNT, true));

    if (!globalScope.nameTable->includeLocal("main"))
        throw GenerationException();
    auto& mainEntry = globalScope.nameTable->getLocal("main");
    if (!typeis<NameEntryFunction>(mainEntry))
        throw GenerationException();

    WhiteSpace code;

    // header

    code.push(Instruments::Stack::push); //
    convertInteger(code, Alignment::LocalHeapBegin);
    code.push(Instruments::Stack::push);
    convertInteger(code, Alignment::GlobalPtr);
    code.push(Instruments::Heap::store);

    code.push(Instruments::Stack::push); //
    convertInteger(code, Alignment::LocalHeapEnd);
    code.push(Instruments::Stack::push);
    convertInteger(code, Alignment::GlobalPtr + globalScope.nameTable->localHeapSize());
    code.push(Instruments::Heap::store);


    code.push(Instruments::Flow::call);
    convertInteger(code, mainEntry.address());
    code.push(Instruments::Flow::exit);

    // body

    convertScope(code, globalScope);

    // flush

    cout << code << flush;
    return 0;
}



// 欲しい情報
// globalVariable を呼ぶために：
// 1つ上のlocalVariable を呼ぶために：
// localVariable を呼ぶために：  スコープの階層
// これらは今実行中の関数で決まる（決めなければならない）
// globalVar_begin,size : 定数(実行時常に不変)
// 今はいらないが後で必要：呼び出し元のlocalvalue
// これは呼び出し元の関数で決まる

// 内部スコープが無いと仮定．つまり，グローバルか，1層のfuncか．
// call するとき，
// - local_begin を stack に積む
// - local_begin := local_end．
// - local_end := local_begin + scopesize．
// return するとき，
// - local_end := local_begin．
// - local_begin を stack から取り出す


/*

let:v1;
func:a(){
    let:v2;
    {
        let:v3;
        // HERE
    }
}
func:main(){
    let:v4;
    a();
}

v1,v2,v3は参照可能であるべき
v4は出来ないが，


*/

// ifのラベルの貼り方とジャンプ

// if only
// [label0@if]
// zj label1@if
// block
// [label1@if]

// ifelse
// [label0@if]
// zj label1@if
// block
// j label1@else
// [label1@if]
// [label0@else]
// block
// [label1@else]

// elsif
// [label0@if]
// zj label1@if
// block
// j label1@else
// [label1@if]
// [label0@elsif]
// zj label1@elsif
// block
// j label1@else
// [label1@elsif]
// [label0@else]
// block
// [label1@else]