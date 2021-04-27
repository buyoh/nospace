#include <cassert>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
using integer = int64_t;


template<typename Derived, typename Base,
    typename enable_if<is_base_of<Base, Derived>::value>::type* = nullptr>
    inline bool typeis(Base& a1) {
    return typeid(a1) == typeid(Derived);
}


template<typename Exception, typename... Args,
    typename enable_if<is_base_of<runtime_error, Exception>::value>::type* = nullptr>
    inline void assert_throw(bool expr, Args&&... args) { if (!expr) throw Exception(args...); }




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


    WhiteSpace& pushInteger(WhiteSpace& whitesp, integer val) {

        if (val < 0) {
            whitesp.push(Chr::TB);
            val = -val;
        }
        else {
            whitesp.push(Chr::SP);

        }
        uint64_t uval = val, rev = 0;
        int len = 0;
        while (uval) {
            rev = (rev << 1) | (uval & 1);
            uval >>= 1;
            ++len;
        }
        while (len--) {
            whitesp.push(rev & 1);
            rev >>= 1;
        }

        whitesp.push(Chr::LF);
        return whitesp;
    }
}


namespace Parser {

    struct ParseException : runtime_error {
        ParseException(const char* msg = "") :runtime_error(msg) { }
    };


    struct Token {
        virtual ~Token() {}
        virtual bool operator==(const Token&) const = 0;
        virtual bool operator==(const string&) const = 0;
        virtual bool operator==(char) const = 0;
        virtual void print(ostream&) const = 0;
    };

    inline ostream& operator<<(ostream& os, const Token& token) {
        token.print(os); return os;
    }


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

        void print(ostream& os, int front = 0, int back = 0) const {
            int head = max((int)ptr - front, 0);
            int tail = min((int)ptr + back, (int)(tokens_.size()) - 1);
            for (; head <= tail; ++head)
                os << *tokens_[head] << ' ';
        }
    };


    class TokenInteger : public Token {
        const integer value_;
    public:
        TokenInteger(integer _val) :value_(_val) {}

        inline integer get() const { return value_; }

        inline bool operator==(const TokenInteger& t) const {
            return value_ == t.value_;
        }
        inline bool operator==(const string& str) const override {
            return false;
        }
        inline bool operator==(char) const override {
            return false;
        }
        inline bool operator==(const Token& t) const override {
            try {
                return operator==(dynamic_cast<const TokenInteger&>(t));
            }
            catch (const bad_cast& _) {
                return false;
            }
        }
        inline void print(ostream& os) const override {
            os << value_;
        }
    };


    class TokenKeyword : public Token {
        const string keyword_;
    public:
        TokenKeyword(string _keyword) :keyword_(_keyword) {}

        inline const string& str() const { return keyword_; }

        inline bool operator==(const TokenKeyword& t) const {
            return keyword_ == t.keyword_;
        }
        inline bool operator==(const string& str) const override {
            return keyword_ == str;
        }
        inline bool operator==(char c) const override {
            return keyword_.size() == (size_t)1 && keyword_[0] == c;
        }
        inline bool operator==(const Token& t) const override {
            try {
                return operator==(dynamic_cast<const TokenKeyword&>(t));
            }
            catch (const bad_cast& _) {
                return false;
            }
        }
        inline void print(ostream& os) const override {
            os << keyword_;
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
        inline bool operator==(char c) const override {
            return _symbol[0] == c && _symbol[1] == 0;
        }
        inline bool operator==(const string& str) const override {
            return _symbol == str;
        }
        inline bool operator==(const Token& t) const override {
            try {
                return operator==(dynamic_cast<const TokenSymbol&>(t));
            }
            catch (const bad_cast& _) {
                return false;
            }
        }
        inline void print(ostream& os) const override {
            for (int i = 0; i < 4 && _symbol[i]; ++i)
                os << _symbol[i];
        }
    };


    inline bool isValidSymbol(char c) {
        static bool f[] = { 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0 };
        return 32 <= c ? f[c - 32] : false;
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
        int c1 = is.get();
        int c2 = is.peek();

        if (c1 == '|') {
            if (c2 == '|') { is.get(); return TokenSymbol('|', '|'); }
        }
        if (c1 == '&') {
            if (c2 == '&') { is.get(); return TokenSymbol('&', '&'); }
        }
        if (c2 == '=') {
            if (c1 == '+' || c1 == '-' || c1 == '*' || c1 == '/' || c1 == '%' ||
                c1 == '=' || c1 == '!' || c1 == '<' || c1 == '>') {
                is.get(); return TokenSymbol(c1, '=');
            }
        }

        return TokenSymbol(c1);
    }



    TokenInteger parseChar(istream& is) {
        is.get(); // '

        integer var = 0;
        int cc = is.peek();
        bool escape = false;

        while (cc = is.get(), escape || cc != '\'') {
            if (!escape) {
                if (cc == '\\') escape = true;
                else var = (var << 8ll) | integer(cc);
            }
            else {
                if (cc == '\\') var = (var << 8ll) | '\\';
                else if (cc == 't') var = (var << 8ll) | '\t';
                else if (cc == 'n') var = (var << 8ll) | '\n';
                else if (cc == 's') var = (var << 8ll) | ' ';
                else if (cc == '\'') var = (var << 8ll) | '\'';
                else throw ParseException("unknown escape char");
                escape = false;
            }
            if (is.eof()) throw ParseException("missing [']");
        }

        return TokenInteger(var);
    }


    void parseLineCommentOut(istream& is) {
        while (!is.eof()) {
            int cc = is.get();
            if (cc == '\r' || cc == '\n') break;
        }
    }


    vector<unique_ptr<Token>> parseToTokens(istream& is) {
        vector<unique_ptr<Token>> tokens;
        tokens.reserve(24);

        while (!is.eof()) {
            int cc = is.peek();

            if (cc == '#') {
                parseLineCommentOut(is);
                continue;
            }

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
            if (cc == '\'') {
                tokens.emplace_back(new TokenInteger(parseChar(is)));
            }
            if (isValidSymbol(cc)) {
                tokens.emplace_back(new TokenSymbol(parseSymbol(is)));
                continue;
            }
            is.get(); // ignore
        }
        return tokens;
    }

}


namespace Compiler {
    using namespace Parser;


    namespace Embedded {
        namespace Function {

            const signed IDaadd = -10;
            const signed IDasub = -11;
            const signed IDamul = -12;
            const signed IDadiv = -13;
            const signed IDamod = -14;
            const signed IDaminus = -15; // 単項

            const signed IDand = -30;
            const signed IDor = -31;

            const signed IDnot = -35;
            const signed IDnotnot = -36;

            const signed IDequal = -40;
            const signed IDnotequal = -41;
            const signed IDless = -42;
            const signed IDlesseq = -43;
            const signed IDgreater = -44;
            const signed IDgreatereq = -45;

            const signed IDassign = -60;
            const signed IDassignadd = -61;
            const signed IDassignsub = -62;
            const signed IDassignmul = -63;
            const signed IDassigndiv = -64;
            const signed IDassignmod = -65;
            const signed IDdereference = -70;
            const signed IDindexer = -75;

            const signed IDputi = -100;
            const signed IDputc = -101;
            const signed IDgeti = -110;
            const signed IDgetc = -111;
            const signed IDgetiv = -112;
            const signed IDgetcv = -113;


        }
    }


    struct CompileException : runtime_error {
        CompileException(const TokenStream& ts, const char* msg = "") :runtime_error(
            [&ts, &msg]() {stringstream s; s << "at: "; ts.print(s, 8, 8); s << "\n" << msg; return s.str(); }()) {
        }
        CompileException(const TokenStream& ts, string msg) :runtime_error(
            [&ts, &msg]() {stringstream s; s << "at: "; ts.print(s, 8, 8); s << "\n" << msg; return s.str(); }()) {
        }
        CompileException(const char* msg = "") :runtime_error(msg) {
        }
        CompileException(string msg) :runtime_error(msg) {
        }
    };

    struct NameException : CompileException {
        NameException(const char* msg = "") :CompileException(msg) { }
        NameException(string msg) :CompileException(msg) { }
    };


    // usage: assert_token<TokenSymbol, CompileException>(token, ':', stream, ":(");
    template<typename ExpectedToken, typename Exception,
        typename... Args,
        typename enable_if<is_base_of<runtime_error, Exception>::value>::type* = nullptr>
        inline void assert_token(const Token& token, char expected, Args&&... args) {
        if (!(typeis<ExpectedToken>(token) && static_cast<const ExpectedToken&>(token) == expected))
            throw Exception(args...);
    }


    // usage: assert_token<TokenSymbol, CompileException>(token, "+=", stream, ":(");
    template<typename ExpectedToken, typename Exception,
        typename... Args,
        typename enable_if<is_base_of<runtime_error, Exception>::value>::type* = nullptr>
        inline void assert_token(const Token& token, string expected, Args&&... args) {
        if (!(typeis<ExpectedToken>(token) && static_cast<const ExpectedToken&>(token) == expected))
            throw Exception(args...);
    }


    //


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
        virtual ~NameEntry() {}

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
            :entries_(), parent_(_parent), addrH_(0), addrL_(_parent->addrL_) { }

        // 何かと制限が多い
        // void merge(NameTable&& _nameTable) {
        //     auto nameTable = _nameTable;
        //     assert(!nameTable.parent_);
        //     for (auto& p : nameTable.entries_) {
        //         auto& entry = p.second;
        //         assert(!entries_.count(p.first));
        //         entries_[p.first] = move(p.second);
        //     }
        // }

        NameTable& parent() { return *parent_; }
        const NameTable& parent() const { return *parent_; }

        const NameEntry& trymakeVariableAddr(const string& name, int length) {
            auto& p = entries_[name];
            if (p) {
                if (!typeis<NameEntryVariable>(*p))
                    throw NameException([&name]() {stringstream s; s << name << " is not a variable name"; return s.str(); }());
            }
            else {
                p.reset(new NameEntryVariable(name, addrH_, length));
                addrH_ += length;
            }
            return *p;
        }

        const NameEntry& trymakeFunctionAddr(const string& name, int argLen = 0) {
            auto& p = entries_[name];
            if (p) {
                if (!typeis<NameEntryFunction>(*p))
                    throw NameException([&name]() {stringstream s; s << name << " is not a function name"; return s.str(); }());
            }
            else {
                p.reset(new NameEntryFunction(name, addrL_, argLen));
                addrL_ += 2; // memo: beginとend
            }
            return *p;
        }

        // 無名スコープのジャンプ(ifなど)に必要なラベルを確保する．
        inline integer reserveLabelAddr(int size) {
            integer a = addrL_; addrL_ += size;
            return a;
        }

        // ラベルIDをtableと同じ進捗にする．ローカルで使用した分進めるために使う．
        inline void seekLabelAddr(const NameTable& table) {
            addrL_ = table.addrL_;
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
            assert(isKeyword(id)); // memo: 製作者のミスによってのみraiseするのでassert
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
        Call
    };

    // inline int OperationLevel(OperationMode mode) {
    //     static const int lev[] = { 0,4,4,5,5,2 };
    //     return lev[size_t(mode)];
    // }


    struct Expression : public Statement {
    };


    class Operation : public Expression {
        integer funcid_;
        vector<unique_ptr<Expression>> args_;
    public:
        Operation(integer _f) :funcid_(_f) { }
        Operation(integer _f, int _argSize) :funcid_(_f), args_(_argSize) { }
        //Operation(integer _f, initializer_list<decltype(args_)::value_type> _args)
        //    :funcid_(_f), args_(_args) { }

        inline integer& id() { return funcid_; }
        inline integer id() const { return funcid_; }

        inline decltype(args_)::value_type& args(int i) { return args_[i]; }
        inline const decltype(args_)::value_type& args(int i) const { return args_[i]; }

        inline Expression& operator[](int i) { return *args_[i]; }
        inline const Expression& operator[](int i) const { return *args_[i]; }

        inline int argSize() const { return (int)args_.size(); }
        inline void resizeArgs(int size) { args_.resize(size); }
    };


    struct Factor : public Expression {
        virtual bool isLValue() const = 0;
    };

    class FactorValue : public Factor {
        integer val_;
    public:
        FactorValue(integer _val) :val_(_val) { }

        inline integer& get() { return val_; }
        inline integer get() const { return val_; }

        inline bool isLValue() const override { return false; }
    };

    class FactorVariable : public Factor {
        integer scope_; // global?: 0 else 1
        integer addr_;
    public:
        FactorVariable(integer _scope, integer _addr) :scope_(_scope), addr_(_addr) { }

        // inline integer& get() { return addr_; }
        inline integer get() const { return addr_; }
        inline integer scope() const { return scope_; }

        inline bool isLValue() const override { return true; }
    };

    class FactorAddress : public FactorVariable {
    public:
        FactorAddress(integer _scope, integer _addr) :FactorVariable(_scope, _addr) { }
        FactorAddress(const FactorVariable& _var) :FactorVariable(_var) { }
        FactorAddress(FactorVariable&& _var) :FactorVariable(move(_var)) { }
    };


    unique_ptr<Expression> getExpression(TokenStream& stream, NameTable& nameTable);


    // elem
    unique_ptr<Expression> getExpressionVal(TokenStream& stream, NameTable& nameTable) {

        const Token& token = stream.peek();

        if (typeis<TokenInteger>(token)) {

            integer val = static_cast<const TokenInteger&>(token).get();
            stream.get();
            return make_unique<FactorValue>(val);
        }
        else if (typeis<TokenKeyword>(token)) {

            const auto& tokenKeyword = static_cast<const TokenKeyword&>(token);

            if (reservedNameTable.include(tokenKeyword.str())) {

                auto& entry = reservedNameTable.get(tokenKeyword.str());

                if (typeis<NameEntryKeyword>(entry)) {
                    throw CompileException(stream, "it is a reserved keyword");
                }
                else if (typeis<NameEntryFunction>(entry)) {
                    auto& funcEntry = static_cast<const NameEntryFunction&>(entry);
                    Operation exps(funcEntry.address(), funcEntry.argLength());

                    try {
                        stream.get();
                        assert_token<TokenSymbol, CompileException>(stream.get(), '(', stream, "expected (");
                        for (int i = 0; i < funcEntry.argLength(); ++i) {
                            exps.args(i) = getExpression(stream, nameTable);
                            if (i + 1 < funcEntry.argLength())
                                assert_token<TokenSymbol, CompileException>(stream.get(), ',', stream, "expected ,");
                        }
                        assert_token<TokenSymbol, CompileException>(stream.get(), ')', stream, "expected )");
                    }
                    catch (const bad_cast& _) {
                        throw CompileException(stream, "invalid function call syntax");
                    }
                    return make_unique<Operation>(move(exps));
                }
            }
            else {

                stream.get();
                const Token& nextToken = stream.peek();

                if (nextToken == "(") {
                    // function
                    if (!nameTable.includeEntire(tokenKeyword.str()))
                        throw CompileException(stream, "undefined identifier");

                    const auto& ref = nameTable.getEntire(tokenKeyword.str());
                    if (!typeis<NameEntryFunction>(ref.first))
                        throw CompileException(stream, "it is not a function");

                    const auto& funcEntry = static_cast<const NameEntryFunction&>(ref.first);
                    Operation exps(funcEntry.address(), funcEntry.argLength());

                    assert_token<TokenSymbol, CompileException>(stream.get(), '(', stream, "expected (");

                    for (int i = 0; i < funcEntry.argLength(); ++i) {
                        exps.args(i) = getExpression(stream, nameTable);
                        if (i + 1 < funcEntry.argLength())
                            assert_token<TokenSymbol, CompileException>(stream.get(), ',', stream, "expected ,");
                    }
                    assert_token<TokenSymbol, CompileException>(stream.get(), ')', stream, "expected )");
                    return make_unique<Operation>(move(exps));
                }
                else {
                    // variable
                    if (!nameTable.includeEntire(tokenKeyword.str()))
                        throw CompileException();

                    const auto& ref = nameTable.getEntire(tokenKeyword.str());
                    if (!typeis<NameEntryVariable>(ref.first))
                        throw CompileException();
                    return make_unique<FactorVariable>(ref.second->isGlobal() ? 0 : 1, ref.first.address());
                }

            }
        }
        else if (typeis<TokenSymbol>(token)) {
            const auto& tokenSymbol = static_cast<const TokenSymbol&>(token);

            if (tokenSymbol == '(') {
                stream.get();
                auto exps = getExpression(stream, nameTable);

                if (stream.get() == ")") return exps;
                throw CompileException(stream, "expected )");
            }
            else if (tokenSymbol == '&') {
                stream.get();
                auto stV = getExpressionVal(stream, nameTable);

                if (typeis<FactorVariable>(*stV)) {
                    return make_unique<FactorAddress>(static_cast<FactorVariable&>(*stV));
                }
                else {
                    throw CompileException(stream, "it's not a variable");
                }
            }

        }
        throw CompileException(stream, "syntax error");
    }

    // '-'
    // TODO: 優先順位．`-*p[1]`は`(-*p)[1]`．`-((*p)[1])`にするには，getExpressionIndexerの前後に分割する必要がある．
    unique_ptr<Expression> getExpressionUnary(TokenStream& stream, NameTable& nameTable) {

        const Token& token = stream.peek();

        if (typeis<TokenSymbol>(token)) {
            const TokenSymbol& tokenSymbol = static_cast<const TokenSymbol&>(token);

            if (tokenSymbol == '-') {
                stream.get();
                auto stV = getExpressionUnary(stream, nameTable);

                if (typeis<FactorValue>(*stV)) {
                    // optimization
                    static_cast<FactorValue&>(*stV).get() *= -1;
                    return stV;
                }
                else {
                    Operation stOp(Embedded::Function::IDaminus, 1);
                    stOp.args(0) = move(stV);
                    return make_unique<Operation>(move(stOp));
                }
            }
            else if (tokenSymbol == '!') {
                stream.get();
                auto stV = getExpressionUnary(stream, nameTable);

                if (typeis<Operation>(*stV)) {
                    auto& stvRef = static_cast<Operation&>(*stV);
                    if (stvRef.id() == Embedded::Function::IDnot) {
                        // optimization
                        static_cast<Operation&>(*stV).id() = Embedded::Function::IDnotnot;
                        return stV;
                    }
                    else if (stvRef.id() == Embedded::Function::IDnotnot) {
                        // optimization(`!!!`)
                        static_cast<Operation&>(*stV).id() = Embedded::Function::IDnot;
                        return stV;
                    }
                }

                {
                    Operation stOp(Embedded::Function::IDnot, 1);
                    stOp.args(0) = move(stV);
                    return make_unique<Operation>(move(stOp));
                }
            }
            else if (tokenSymbol == '*') {
                stream.get();
                Operation stOp(Embedded::Function::IDdereference, 1);
                stOp.args(0) = getExpressionUnary(stream, nameTable);
                return make_unique<Operation>(move(stOp));
            }
            else {
                return getExpressionVal(stream, nameTable);
            }
        }
        else {
            return getExpressionVal(stream, nameTable);
        }
    }

    // a[1]
    unique_ptr<Expression> getExpressionIndexer(TokenStream& stream, NameTable& nameTable) {

        auto stV = getExpressionUnary(stream, nameTable);

        const Token& token = stream.peek();

        if (token == "[") {
            // a[1] => op(&a, 1)

            stream.get();
            auto index = getExpression(stream, nameTable);
            assert_token<TokenSymbol, CompileException>(stream.get(), ']', stream, "expected ]");

            if (typeis<FactorVariable>(*stV)) {
                // variable
                auto stOp = make_unique<Operation>(Embedded::Function::IDindexer, 2);
                stOp->args(0) = make_unique<FactorAddress>(static_cast<FactorVariable&>(*stV));
                stOp->args(1) = move(index);
                return stOp;
            }
            else if (typeis<Operation>(*stV)) {
                // derefference
                auto& o = static_cast<Operation&>(*stV);
                assert_throw<CompileException>(
                    o.id() == Embedded::Function::IDdereference,
                    stream, "must be a derefference or a variable");
                auto stOp = make_unique<Operation>(Embedded::Function::IDindexer, 2);
                stOp->args(0) = move(o.args(0));
                stOp->args(1) = move(index);
                return stOp;
            }
            else {
                Operation stOp(Embedded::Function::IDaminus, 1);
                stOp.args(0) = move(stV);
                return make_unique<Operation>(move(stOp));
            }
        }
        else {
            return stV;
        }
    }

    // '*'
    unique_ptr<Expression> getExpressionMul(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        *curr = getExpressionIndexer(stream, nameTable);

        while (!stream.eof()) {
            const Token& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                const auto& tokenSymbol = static_cast<const TokenSymbol&>(token);

                if (tokenSymbol == '*' || tokenSymbol == '/' || tokenSymbol == '%') {
                    auto new_ex = new Operation(
                        tokenSymbol == '/' ? Embedded::Function::IDadiv :
                        tokenSymbol == '%' ? Embedded::Function::IDamod :
                        Embedded::Function::IDamul, 2);
                    new_ex->args(0) = move(root);
                    root.reset(new_ex);
                    curr = &(*new_ex).args(1);
                    stream.get();
                }
                else {
                    break;
                }
            }
            else {
                throw CompileException(stream, "syntax error");
            }
            *curr = getExpressionIndexer(stream, nameTable);
        }
        return root;
    }

    // '+'
    unique_ptr<Expression> getExpressionPls(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        *curr = getExpressionMul(stream, nameTable);

        while (!stream.eof()) {
            const Token& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                const auto& tokenSymbol = static_cast<const TokenSymbol&>(token);

                if (tokenSymbol == '+' || tokenSymbol == '-') {

                    auto new_ex = new Operation(
                        tokenSymbol == '-' ? Embedded::Function::IDasub :
                        Embedded::Function::IDaadd, 2);
                    new_ex->args(0) = move(root);
                    root.reset(new_ex);
                    curr = &(*new_ex).args(1);
                    stream.get();
                }
                else {
                    break;
                }
            }
            else {
                throw CompileException(stream, "syntax error");
            }
            *curr = getExpressionMul(stream, nameTable);
        }
        return root;
    }

    // '<'
    unique_ptr<Expression> getExpressionComp(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        *curr = getExpressionPls(stream, nameTable);

        while (!stream.eof()) {
            const Token& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                const auto& tokenSymbol = static_cast<const TokenSymbol&>(token);

                // 左結合
                auto commonProcedure = [&](Operation* new_ex) {
                    (*new_ex).args(0) = move(root);
                    root.reset(new_ex);
                    curr = &(*new_ex).args(1);
                    stream.get();
                };

                if (tokenSymbol == "==") {
                    auto new_ex = new Operation(Embedded::Function::IDequal, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == "!=") {
                    auto new_ex = new Operation(Embedded::Function::IDnotequal, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == '<') {
                    auto new_ex = new Operation(Embedded::Function::IDless, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == "<=") {
                    auto new_ex = new Operation(Embedded::Function::IDlesseq, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == '>') {
                    auto new_ex = new Operation(Embedded::Function::IDgreater, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == ">=") {
                    auto new_ex = new Operation(Embedded::Function::IDgreatereq, 2);
                    commonProcedure(new_ex);
                }
                else {
                    break;
                }
            }
            else {
                throw CompileException(stream, "syntax error");
            }
            *curr = getExpressionPls(stream, nameTable);
        }
        return root;
    }

    // &&
    unique_ptr<Expression> getExpressionLogi(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        *curr = getExpressionComp(stream, nameTable);

        while (!stream.eof()) {
            const Token& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                const auto& tokenSymbol = static_cast<const TokenSymbol&>(token);

                // 左結合
                auto commonProcedure = [&](Operation* new_ex) {
                    (*new_ex).args(0) = move(root);
                    root.reset(new_ex);
                    curr = &(*new_ex).args(1);
                    stream.get();
                };

                if (tokenSymbol == "&&") {
                    auto new_ex = new Operation(Embedded::Function::IDand, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == "||") {
                    auto new_ex = new Operation(Embedded::Function::IDor, 2);
                    commonProcedure(new_ex);
                }
                else {
                    break;
                }
            }
            else {
                throw CompileException(stream, "syntax error");
            }
            *curr = getExpressionComp(stream, nameTable);
        }
        return root;
    }

    // '='
    unique_ptr<Expression> getExpressionAsg(TokenStream& stream, NameTable& nameTable) {
        unique_ptr<Expression> root;
        unique_ptr<Expression>* curr = &root;

        *curr = getExpressionLogi(stream, nameTable);

        while (!stream.eof()) {
            const Token& token = stream.peek();
            if (typeis<TokenSymbol>(token)) {
                const auto& tokenSymbol = static_cast<const TokenSymbol&>(token);

                // 右結合
                auto commonProcedure = [&](Operation* new_ex) {
                    new_ex->args(0) = move(*curr);
                    curr->reset(new_ex);
                    curr = &(*new_ex).args(1);
                    stream.get();
                };

                if (tokenSymbol == '=') {
                    auto new_ex = new Operation(Embedded::Function::IDassign, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == "+=") {
                    auto new_ex = new Operation(Embedded::Function::IDassignadd, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == "-=") {
                    auto new_ex = new Operation(Embedded::Function::IDassignsub, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == "*=") {
                    auto new_ex = new Operation(Embedded::Function::IDassignmul, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == "/=") {
                    auto new_ex = new Operation(Embedded::Function::IDassigndiv, 2);
                    commonProcedure(new_ex);
                }
                else if (tokenSymbol == "%=") {
                    auto new_ex = new Operation(Embedded::Function::IDassignmod, 2);
                    commonProcedure(new_ex);
                }
                else {
                    break;
                }
            }
            else {
                throw CompileException(stream, "syntax error");
            }
            *curr = getExpressionLogi(stream, nameTable);
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
        list<unique_ptr<Statement>> statements;

        StatementScope(shared_ptr<NameTable>& _nameTable) :nameTable(_nameTable), statements() { }
        StatementScope(shared_ptr<NameTable>&& _nameTable) :nameTable(move(_nameTable)), statements() { }


    };


    struct StatementFunction : public StatementScope {
        integer funcLabel;
        const vector<integer> argAddrs;
        StatementFunction(StatementScope&& _scope, integer _funcLabel, vector<integer>&& _argAddrs) :
            StatementScope(move(_scope)), funcLabel(_funcLabel), argAddrs((_argAddrs.shrink_to_fit(), _argAddrs)) {
        }
    };


    // nameTableを持たないスコープ
    struct StatementOpenScope : public Statement {
    private:
        weak_ptr<NameTable> parentNameTable; // TODO: 使われていないので消す
    public:
        list<unique_ptr<Statement>> statements;

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


    struct StatementReturn : public Statement {
        unique_ptr<Expression> retVal;
        StatementReturn(unique_ptr<Expression>&& _retVal) :retVal(move(_retVal)) { }
        StatementReturn() { }
    };


    struct StatementLetInit : public Statement {
        // <address, expr>
        vector<pair<integer, unique_ptr<Expression>>> assignments;
    };


    //


    // 予めScope用のlocalNameTableを生成しておく必要がある
    unique_ptr<StatementScope> getStatementsScope(TokenStream& stream, shared_ptr<NameTable>& localNameTable,
        bool globalScope) {

        auto localScope = make_unique<StatementScope>(localNameTable);

        if (!globalScope) stream.get();

        while (true) {
            if (stream.eof()) {
                if (!globalScope) throw CompileException(stream, "missing }");
                break;
            }
            auto& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                auto& tokenSymbol = static_cast<const TokenSymbol&>(token);
                if (tokenSymbol == '}') {
                    stream.get(); break;
                }
            }

            //localScope.statements.emplace_back(new Statement(getStatement(stream, localScope.nameTable)));
            auto statement = getStatement(stream, localScope->nameTable, globalScope, !globalScope, false);
            localScope->statements.push_back(move(statement));
            if (!localScope->statements.back()) localScope->statements.pop_back();
        }
        if (!globalScope)
            localScope->nameTable->parent().seekLabelAddr(localScope->nameTable);
        return localScope;
    }


    // nameTableを持たないスコープ
    unique_ptr<StatementOpenScope> getStatementsOpenScope(TokenStream& stream, shared_ptr<NameTable>& parentNameTable) {
        auto scope = make_unique<StatementOpenScope>(parentNameTable, true);

        assert_token<TokenSymbol, CompileException>(stream.get(), '{', stream, "expected {");

        while (true) {
            if (stream.eof()) {
                throw CompileException(stream, "missing }");
            }
            auto& token = stream.peek();

            if (typeis<TokenSymbol>(token)) {
                auto& tokenSymbol = static_cast<const TokenSymbol&>(token);
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


    void procStatementLetItem(TokenStream& stream, shared_ptr<NameTable>& nameTable, StatementLetInit& letinit) {
        auto& varName = dynamic_cast<const TokenKeyword&>(stream.get()); // throw
        if (nameTable->includeLocal(varName.str()) ||
            reservedNameTable.include(varName.str())) {
            throw CompileException(stream, "is already defined");
        }
        integer length = 1;
        // array length
        if (stream.peek() == '[') {
            stream.get();
            auto expr = getExpression(stream, *nameTable);
            assert_throw<CompileException>(typeis<FactorValue>(*expr), stream, "expected value");
            length = dynamic_cast<const FactorValue&>(*expr).get();
            assert_token<TokenSymbol, CompileException>(stream.get(), ']', stream, "expected ]");
        }
        const NameEntry& entry = nameTable->trymakeVariableAddr(varName.str(), length);
        // initialization
        if (stream.peek() == '(') {
            stream.get();
            for (int first = true, cnt = 0;; first = false, ++cnt) {
                if (stream.peek() == ')') {
                    if (!first) throw CompileException(stream, "unexpected ,");
                    stream.get();
                    break;
                }
                if (cnt >= length) throw CompileException(stream, "too many expressions");

                letinit.assignments.emplace_back(entry.address() + cnt, getExpression(stream, *nameTable));

                auto& term = stream.get();
                if (term == ',') continue;
                if (term == ')') break;
                throw CompileException(stream, "expected )");
            }
        }
    }


    unique_ptr<StatementLetInit> getStatementLet(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        stream.get(); // assert_token<TokenKeyword>(stream.get(), "let", CompileException(stream, "expected let"));
        assert_token<TokenSymbol, CompileException>(stream.get(), ':', stream, "expected :");

        StatementLetInit letinit;

        while (true) {
            procStatementLetItem(stream, nameTable, letinit);

            auto& next = stream.get();
            if (next == ';') break;
            if (next == ',') continue;
            throw CompileException(stream, "expected ;");
        }
        return make_unique<StatementLetInit>(move(letinit));
    }


    unique_ptr<StatementFunction> getStatementFunction(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        stream.get(); // assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "func");
        assert_token<TokenSymbol, CompileException>(stream.get(), ':', stream, "expected :");
        auto& funcName = dynamic_cast<const TokenKeyword&>(stream.get());
        if (nameTable->includeLocal(funcName.str()) ||
            reservedNameTable.include(funcName.str())) {
            throw CompileException();
        }

        assert_token<TokenSymbol, CompileException>(stream.get(), '(', stream, "expected (");


        list<string> argStrs;

        for (bool first = true;; first = false) {
            assert_throw<CompileException>(!stream.eof(), stream, "expected )");
            auto& token = stream.get();

            if (token == ')') {
                if (!first) throw CompileException(stream, "unexpected ,");
                break;
            }

            assert_throw<CompileException>(typeis<TokenKeyword>(token), stream, "expected argument");
            // let
            auto& varName = dynamic_cast<const TokenKeyword&>(token);
            if (nameTable->includeLocal(funcName.str()) ||
                reservedNameTable.include(funcName.str())) {
                throw CompileException();
            }
            argStrs.push_back(varName.str());

            auto& token2 = stream.get();
            if (token2 == ")") break;
            if (token2 == ",") continue;
            throw CompileException(stream, "syntax error");
        }

        // trymakeFunctionAddr=>makeの呼び出し順序に注意
        const auto& entryRef = nameTable->trymakeFunctionAddr(funcName.str(), (int)argStrs.size());
        auto localNameTable = make_shared<NameTable>(nameTable);

        vector<integer> argAddrs;
        for (auto& a : argStrs) {
            const auto& entry = localNameTable->trymakeVariableAddr(a, 1);
            argAddrs.push_back(entry.address());
        }

        auto scope = getStatementsScope(stream, localNameTable, false);

        // todo: unique_ptrの中身をmoveする操作は正しいか？
        return make_unique<StatementFunction>(move(*scope), entryRef.address(), move(argAddrs));
    }


    unique_ptr<StatementWhile> getStatementWhile(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        stream.get(); // assert(dynamic_cast<const TokenKeyword&>(stream.get()) == "while");
        assert_token<TokenSymbol, CompileException>(stream.get(), '(', stream, "expected (");
        auto&& condition = getExpression(stream, *nameTable);
        assert_token<TokenSymbol, CompileException>(stream.get(), ')', stream, "expected )");
        auto label = nameTable->reserveLabelAddr(2);
        return make_unique<StatementWhile>(move(*getStatementsOpenScope(stream, nameTable)), move(condition), label);
    }


    unique_ptr<StatementIf> getStatementElse(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        stream.get(); // assert_token<TokenKeyword, CompileException>(stream.get(), "else", stream, "expected else");
        auto label = nameTable->reserveLabelAddr(2);
        return make_unique<StatementIf>(move(*getStatementsOpenScope(stream, nameTable)), label);
    }


    unique_ptr<StatementIf> getStatementIf(TokenStream& stream, shared_ptr<NameTable>& nameTable, bool elsif) {
        stream.get();
        // if (elsif)
        //     assert_token<TokenKeyword, CompileException>(stream.get(), "elsif", stream, "expected elsif");
        // else
        //     assert_token<TokenKeyword, CompileException>(stream.get(), "if", stream, "expected if");
        assert_token<TokenSymbol, CompileException>(stream.get(), '(', stream, "expected (");
        auto&& condition = getExpression(stream, *nameTable);
        assert_token<TokenSymbol, CompileException>(stream.get(), ')', stream, "expected )");
        auto label = nameTable->reserveLabelAddr(2);
        auto ifscope = make_unique<StatementIf>(move(*getStatementsOpenScope(stream, nameTable)), move(condition), label);

        const auto& token = stream.peek();
        if (typeis<TokenKeyword>(token)) {
            if (token == "elsif") {
                ifscope->elsif = getStatementIf(stream, nameTable, true);
                return ifscope;
            }
            else if (token == "else") {
                ifscope->elsif = getStatementElse(stream, nameTable);
                return ifscope;
            }
        }

        return ifscope;
    }


    unique_ptr<StatementReturn> getStatementReturn(TokenStream& stream, shared_ptr<NameTable>& nameTable) {
        stream.get(); // assert_token<TokenKeyword, CompileException>(stream.get(), "return", stream, "expected return");

        assert_throw<CompileException>(typeis<TokenSymbol>(stream.peek()), stream, "expected )");
        auto& token = static_cast<const TokenSymbol&>(stream.peek());

        if (token == ':') {
            stream.get();
            auto&& expr = getExpression(stream, *nameTable);
            assert_token<TokenSymbol, CompileException>(stream.get(), ';', stream, "expected ;");
            return make_unique<StatementReturn>(move(expr));
        }
        else if (token == ';') {
            stream.get();
            return make_unique<StatementReturn>();
        }
        throw CompileException(stream, "expected ;");
    }


    unique_ptr<Statement> getStatement(TokenStream& stream, shared_ptr<NameTable>& nameTable,
        bool disableExpr, bool disableFunc, bool disableLet) {

        if (stream.eof()) throw CompileException(stream, "has reached eof");
        auto& token = stream.peek();
        if (typeis<TokenSymbol>(token)) {
            // ...
        }
        else if (typeis<TokenKeyword>(token)) {
            auto& tokenKeyword = static_cast<const TokenKeyword&>(token);
            if (tokenKeyword == "func") {
                if (disableFunc) throw CompileException(stream, "can't define function");
                return getStatementFunction(stream, nameTable);
            }
            if (tokenKeyword == "let") {
                if (disableLet) throw CompileException(stream, "can't define let");
                return getStatementLet(stream, nameTable);
            }
        }

        if (disableExpr) throw CompileException(stream, "syntax error");;

        if (typeis<TokenSymbol>(token)) {
            // ...
        }
        else if (typeis<TokenKeyword>(token)) {
            auto& tokenKeyword = static_cast<const TokenKeyword&>(token);
            if (tokenKeyword == "if") {
                return getStatementIf(stream, nameTable, false);
            }
            if (tokenKeyword == "while") {
                return getStatementWhile(stream, nameTable);
            }
            if (tokenKeyword == "return") {
                return getStatementReturn(stream, nameTable);
            }
        }

        auto&& p = getExpression(stream, *nameTable);
        assert_token<TokenSymbol, CompileException>(stream.get(), ';', stream, "expected ;");
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
        const integer LocalHeapBegin = 2;
        const integer LocalHeapEnd = 3;
        const integer TempPtr = 4;
        const integer GlobalPtr = 8;

        const integer LabelOffset = 16;
        const integer LabelUserCodeBegin = 0;

        const integer LabelComparatorZero = 2;
        const integer LabelComparatorZero2 = 3;
        const integer LabelComparatorNegative = 4;
        const integer LabelComparatorNegative2 = 5;
        const integer LabelComparatorAnd = 6;
        const integer LabelComparatorAnd2 = 7;
        const integer LabelComparatorOr = 8;
        const integer LabelComparatorOr2 = 9;
        const integer LabelComparatorOr3 = 10;
    }


    inline integer solveLabel(integer labelId) {
        return labelId + Alignment::LabelOffset;
    }


    struct OperatorException : runtime_error {
        OperatorException(const char* msg = "") :runtime_error(msg) { }
    };


    struct GenerationException : runtime_error {
        GenerationException(const char* msg = "") :runtime_error(msg) { }
    };


    //


    // WhiteSpace& convertSwap(WhiteSpace& whitesp, integer destPtr, integer fromPtr) {
    //
    // }


    // *destPtr = *fromPtr
    WhiteSpace& convertCopy(WhiteSpace& whitesp, integer destPtr, integer fromPtr) {
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, integer(fromPtr));
        whitesp.push(Instruments::Heap::retrieve);
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, integer(destPtr));
        whitesp.push(Instruments::Stack::swap);
        whitesp.push(Instruments::Heap::store);
        return whitesp;
    }


    //


    // subroutine label の後に挿入する：
    WhiteSpace& convertLocalAllocate(WhiteSpace& whitesp, const StatementFunction& func) {
        // - local_begin を stack に積む
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, Alignment::LocalHeapBegin);
        whitesp.push(Instruments::Heap::retrieve);
        // - local_begin := local_end．(dup local_end)
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, Alignment::LocalHeapEnd);
        whitesp.push(Instruments::Heap::retrieve);
        whitesp.push(Instruments::Stack::duplicate); // dup!
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, Alignment::LocalHeapBegin);
        whitesp.push(Instruments::Stack::swap);
        whitesp.push(Instruments::Heap::store);
        // remain local_begin value on stack.
        // - local_end := local_begin(stacked by dup) + scopesize．
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, func.nameTable->localHeapSize());
        whitesp.push(Instruments::Arithmetic::add);
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, Alignment::LocalHeapEnd);
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
        pushInteger(whitesp, Alignment::LocalHeapBegin);
        whitesp.push(Instruments::Stack::swap);
        whitesp.push(Instruments::Heap::store);
        return whitesp;
    }


    WhiteSpace& convertCalculateLocalVariablePtr(WhiteSpace& whitesp, integer addr) {
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, addr);

        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, Alignment::LocalHeapBegin);
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
            pushInteger(whitesp, var.get() + Alignment::GlobalPtr);
            return whitesp;
        }
    }


    //


    WhiteSpace& convertExpression(WhiteSpace&, const Expression&);


    WhiteSpace& convertValue(WhiteSpace& whitesp, const Factor& factor) {
        if (typeis<FactorValue>(factor)) {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, static_cast<const FactorValue&>(factor).get());
            return whitesp;
        }
        else if (typeis<FactorVariable>(factor)) {
            const auto& var = static_cast<const FactorVariable&>(factor);
            // calculate addr
            convertCalculateLocalVariablePtr(whitesp, var);
            // retirieve
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        else if (typeis<FactorAddress>(factor)) {
            const auto& var = static_cast<const FactorAddress&>(factor);
            // calculate addr
            convertCalculateLocalVariablePtr(whitesp, var);
            return whitesp;
        }
        throw OperatorException();
    }


    WhiteSpace& convertEmbeddedExpression(WhiteSpace& whitesp, const Operation& exps) {

        switch (exps.id())
        {
        case Embedded::Function::IDequal: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(1)); // zero
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(0)); // notzero
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::sub);

            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorZero);
            return whitesp;
        }
        case Embedded::Function::IDnotequal: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(0)); // zero
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(1)); // notzero
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::sub);

            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorZero);
            return whitesp;
        }
        case Embedded::Function::IDless: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(1)); // neg
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(0)); // notneg
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::sub);

            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorNegative);
            return whitesp;
        }
        case Embedded::Function::IDgreatereq: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(0)); // neg
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(1)); // notneg
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::sub);

            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorNegative);
            return whitesp;
        }
        case Embedded::Function::IDgreater: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(1)); // neg
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(0)); // notneg
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Stack::swap);
            whitesp.push(Instruments::Arithmetic::sub);

            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorNegative);
            return whitesp;
        }
        case Embedded::Function::IDlesseq: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(0)); // neg
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(1)); // notneg
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Stack::swap);
            whitesp.push(Instruments::Arithmetic::sub);

            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorNegative);
            return whitesp;
        }
        case Embedded::Function::IDand: {
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorAnd);
            return whitesp;
        }
        case Embedded::Function::IDor: {
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorOr);
            return whitesp;
        }
        case Embedded::Function::IDnot: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(1)); // zero
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(0)); // notzero
            convertExpression(whitesp, exps[0]);

            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorZero);
            return whitesp;
        }
        case Embedded::Function::IDnotnot: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(0)); // zero
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, integer(1)); // notzero
            convertExpression(whitesp, exps[0]);

            whitesp.push(Instruments::Flow::call);
            pushInteger(whitesp, Alignment::LabelComparatorZero);
            return whitesp;
        }
        case Embedded::Function::IDaadd: {
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::add);
            return whitesp;
        }
        case Embedded::Function::IDasub: {
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::sub);
            return whitesp;
        }
        case Embedded::Function::IDamul: {
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::mul);
            return whitesp;
        }
        case Embedded::Function::IDadiv: {
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::div);
            return whitesp;
        }
        case Embedded::Function::IDamod: {
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::mod);
            return whitesp;
        }
        case Embedded::Function::IDaminus: {
            whitesp.push(Instruments::Stack::push);
            whitesp.push({ Chr::SP, Chr::LF }); // zero
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Arithmetic::sub);
            return whitesp;
        }
        case Embedded::Function::IDassign:
        case Embedded::Function::IDassignadd:
        case Embedded::Function::IDassignsub:
        case Embedded::Function::IDassignmul:
        case Embedded::Function::IDassigndiv:
        case Embedded::Function::IDassignmod: {
            if (typeis<FactorVariable>(exps[0])) {
                const FactorVariable& var = static_cast<const FactorVariable&>(exps[0]);
                convertCalculateLocalVariablePtr(whitesp, var);
            }
            else if (typeis<Operation>(exps[0])) {
                const auto& dref = static_cast<const Operation&>(exps[0]);
                if (dref.id() == Embedded::Function::IDdereference) {
                    convertExpression(whitesp, dref[0]);
                }
                else if (dref.id() == Embedded::Function::IDindexer) {
                    convertExpression(whitesp, dref[0]);
                    convertExpression(whitesp, dref[1]);
                    whitesp.push(Instruments::Arithmetic::add);
                }
            }
            else { throw OperatorException(); }


            if (exps.id() == Embedded::Function::IDassign) {
                whitesp.push(Instruments::Stack::duplicate);
                convertExpression(whitesp, exps[1]);
                whitesp.push(Instruments::Heap::store);

                whitesp.push(Instruments::Heap::retrieve);
                return whitesp;
            }
            else {
                whitesp.push(Instruments::Stack::duplicate);
                whitesp.push(Instruments::Stack::duplicate);
                whitesp.push(Instruments::Heap::retrieve);
                convertExpression(whitesp, exps[1]);
                switch (exps.id()) {
                case Embedded::Function::IDassignadd:
                    whitesp.push(Instruments::Arithmetic::add); break;
                case Embedded::Function::IDassignsub:
                    whitesp.push(Instruments::Arithmetic::sub); break;
                case Embedded::Function::IDassignmul:
                    whitesp.push(Instruments::Arithmetic::mul); break;
                case Embedded::Function::IDassigndiv:
                    whitesp.push(Instruments::Arithmetic::div); break;
                case Embedded::Function::IDassignmod:
                    whitesp.push(Instruments::Arithmetic::mod); break;
                }
                whitesp.push(Instruments::Heap::store);
                whitesp.push(Instruments::Heap::retrieve);
                return whitesp;
            }

        }
        case Embedded::Function::IDdereference: {
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        case Embedded::Function::IDindexer: {
            // add + dereference
            convertExpression(whitesp, exps[0]);
            convertExpression(whitesp, exps[1]);
            whitesp.push(Instruments::Arithmetic::add);
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        case Embedded::Function::IDputi: {
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Stack::duplicate);
            whitesp.push(Instruments::IO::putnumber);
            return whitesp;
        }
        case Embedded::Function::IDputc: {
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Stack::duplicate);
            whitesp.push(Instruments::IO::putchar);
            return whitesp;
        }
        case Embedded::Function::IDgeti: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, Alignment::TempPtr);
            whitesp.push(Instruments::IO::getnumber);
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, Alignment::TempPtr);
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        case Embedded::Function::IDgetc: {
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, Alignment::TempPtr);
            whitesp.push(Instruments::IO::getchar);
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, Alignment::TempPtr);
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        case Embedded::Function::IDgetiv: {
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Stack::duplicate);
            whitesp.push(Instruments::IO::getnumber);
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        case Embedded::Function::IDgetcv: {
            convertExpression(whitesp, exps[0]);
            whitesp.push(Instruments::Stack::duplicate);
            whitesp.push(Instruments::IO::getchar);
            whitesp.push(Instruments::Heap::retrieve);
            return whitesp;
        }
        default: {
            throw OperatorException();
        }
        }
    }


    WhiteSpace& convertExpression(WhiteSpace& whitesp, const Expression& exps) {

        if (typeis<Operation>(exps)) {

            const Operation& op = static_cast<const Operation&>(exps);

            if (op.id() < 0) {
                convertEmbeddedExpression(whitesp, op);
            }
            else {
                for (int i = 0; i < op.argSize(); ++i)
                    convertExpression(whitesp, *op.args(i));

                whitesp.push(Instruments::Flow::call);
                pushInteger(whitesp, solveLabel(op.id()));

            }
            return whitesp;
        }
        try {
            convertValue(whitesp, dynamic_cast<const Factor&>(exps));
            return whitesp;
        }
        catch (const bad_cast& _) {}

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
        integer label = solveLabel(func.funcLabel);

        whitesp.push(Instruments::Flow::jump);
        pushInteger(whitesp, label + 1);

        whitesp.push(Instruments::Flow::label);
        pushInteger(whitesp, label);

        convertLocalAllocate(whitesp, func);

        if (!func.argAddrs.empty()) {
            // 復帰用スタック値をTEMP0に退避
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, Alignment::TempPtr);
            whitesp.push(Instruments::Stack::swap);
            whitesp.push(Instruments::Heap::store);

            // args(差分を使って出力コード最適化)
            int bk = int(func.argAddrs.size()) - 1;
            // アドレス基準値をTEMP1に保存
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, Alignment::TempPtr + 1);
            convertCalculateLocalVariablePtr(whitesp, func.argAddrs[bk]);
            whitesp.push(Instruments::Heap::store);
            for (int i = bk; 0 <= i; --i) {
                // アドレス基準値を取り出す
                whitesp.push(Instruments::Stack::push);
                pushInteger(whitesp, Alignment::TempPtr + 1);
                whitesp.push(Instruments::Heap::retrieve);
                // 本来求めたいconvertCalculateLocalVariablePtr(whitesp, func.argAddrs[i]); を計算
                if (func.argAddrs[i] != func.argAddrs[bk]) {
                    whitesp.push(Instruments::Stack::push);
                    pushInteger(whitesp, func.argAddrs[i] - func.argAddrs[bk]);
                    whitesp.push(Instruments::Arithmetic::add);
                }
                // 引数値を変数にセット
                whitesp.push(Instruments::Stack::swap);
                whitesp.push(Instruments::Heap::store);
            }

            // 復帰用スタック値をTEMP0からスタックに戻す
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, Alignment::TempPtr);
            whitesp.push(Instruments::Heap::retrieve);
        }

        convertScope(whitesp, dynamic_cast<const StatementScope&>(func));


        convertLocalDeallocate(whitesp);

        // default return value(0)
        whitesp.push(Instruments::Stack::push);
        pushInteger(whitesp, 0);

        whitesp.push(Instruments::Flow::retun);

        whitesp.push(Instruments::Flow::label);
        pushInteger(whitesp, label + 1);
        return whitesp;
    }


    WhiteSpace& convertWhile(WhiteSpace& whitesp, const StatementWhile& whilestat) {
        integer label = solveLabel(whilestat.label);

        whitesp.push(Instruments::Flow::label);
        pushInteger(whitesp, label);

        convertExpression(whitesp, *(whilestat.cond));
        whitesp.push(Instruments::Flow::zerojump);
        pushInteger(whitesp, label + 1);

        convertOpenScope(whitesp, whilestat);

        whitesp.push(Instruments::Flow::jump); // loop
        pushInteger(whitesp, label);
        whitesp.push(Instruments::Flow::label);
        pushInteger(whitesp, label + 1);

        return whitesp;
    }


    // condition部分
    WhiteSpace& convertIf_condition(WhiteSpace& whitesp, const StatementIf& ifstat, integer label) {
        const Expression& expr = *(ifstat.cond);

        // optimization
        if (typeis<Operation>(expr)) {
            const Operation& op = static_cast<const Operation&>(expr);
            switch (op.id()) {
            case Embedded::Function::IDnotequal: {
                convertExpression(whitesp, op[0]);
                convertExpression(whitesp, op[1]);
                whitesp.push(Instruments::Arithmetic::sub);
                whitesp.push(Instruments::Flow::zerojump);
                pushInteger(whitesp, label + 1);
                return whitesp;
            }
            case Embedded::Function::IDgreatereq: {
                convertExpression(whitesp, op[0]);
                convertExpression(whitesp, op[1]);
                whitesp.push(Instruments::Arithmetic::sub);
                whitesp.push(Instruments::Flow::negativejump);
                pushInteger(whitesp, label + 1);
                return whitesp;
            }
            case Embedded::Function::IDlesseq: {
                convertExpression(whitesp, op[0]);
                convertExpression(whitesp, op[1]);
                whitesp.push(Instruments::Stack::swap);
                whitesp.push(Instruments::Arithmetic::sub);
                whitesp.push(Instruments::Flow::negativejump);
                pushInteger(whitesp, label + 1);
                return whitesp;
            }
            }
        }
        convertExpression(whitesp, expr);
        whitesp.push(Instruments::Flow::zerojump);
        pushInteger(whitesp, label + 1);
        return whitesp;
    }


    // elsif, else も処理する
    WhiteSpace& convertIf(WhiteSpace& whitesp, const StatementIf& ifstat) {
        integer label = solveLabel(ifstat.label);

        whitesp.push(Instruments::Flow::label);
        pushInteger(whitesp, label);

        if (ifstat.cond) {
            convertIf_condition(whitesp, ifstat, label);
        }

        convertOpenScope(whitesp, ifstat);

        if (ifstat.elsif) {
            whitesp.push(Instruments::Flow::jump);
            pushInteger(whitesp, solveLabel(ifstat.getLabelLast()) + 1);
        }

        whitesp.push(Instruments::Flow::label);
        pushInteger(whitesp, label + 1);

        if (ifstat.elsif)
            convertIf(whitesp, *(ifstat.elsif));

        return whitesp;
    }


    WhiteSpace& convertReturn(WhiteSpace& whitesp, const StatementReturn& stat) {

        if (stat.retVal) {
            convertExpression(whitesp, *stat.retVal);
            whitesp.push(Instruments::Stack::swap);
            convertLocalDeallocate(whitesp);
        }
        else {
            convertLocalDeallocate(whitesp);
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, 0);
        }

        whitesp.push(Instruments::Flow::retun);
        return whitesp;
    }


    WhiteSpace& convertLetInit(WhiteSpace& whitesp, const StatementLetInit& stat) {

        if (stat.assignments.empty()) return whitesp;

        integer baseaddr = stat.assignments[0].first;

        convertCalculateLocalVariablePtr(whitesp, baseaddr);
        int cnt = stat.assignments.size();
        for (const auto& a : stat.assignments) {
            if (cnt-- != 1) whitesp.push(Instruments::Stack::duplicate);
            whitesp.push(Instruments::Stack::push);
            pushInteger(whitesp, a.first - baseaddr);
            whitesp.push(Instruments::Arithmetic::add);
            convertExpression(whitesp, *a.second);
            whitesp.push(Instruments::Heap::store);
        }
        return whitesp;
    }


    WhiteSpace& convertStatement(WhiteSpace& whitesp, const Statement& stat) {
        if (typeis<StatementFunction>(stat))
            return convertFunction(whitesp, static_cast<const StatementFunction&>(stat));
        if (typeis<StatementScope>(stat)) // 未実装()
            return convertScope(whitesp, static_cast<const StatementScope&>(stat));
        if (typeis<StatementIf>(stat))
            return convertIf(whitesp, static_cast<const StatementIf&>(stat));
        if (typeis<StatementWhile>(stat))
            return convertWhile(whitesp, static_cast<const StatementWhile&>(stat));
        if (typeis<StatementReturn>(stat))
            return convertReturn(whitesp, static_cast<const StatementReturn&>(stat));
        if (typeis<StatementLetInit>(stat))
            return convertLetInit(whitesp, static_cast<const StatementLetInit&>(stat));
        try {
            convertExpression(whitesp, dynamic_cast<const Expression&>(stat));
            whitesp.push(Instruments::Stack::discard);
            return whitesp;
        }
        catch (const bad_cast& _) {}

        throw GenerationException();
    }


    //


    void attachEmbeddedHeader(WhiteSpace& code, const StatementScope& globalScope) {

        // initialize memory

        code.push(Instruments::Stack::push); //
        pushInteger(code, Alignment::LocalHeapBegin);
        code.push(Instruments::Stack::push);
        pushInteger(code, Alignment::GlobalPtr);
        code.push(Instruments::Heap::store);

        code.push(Instruments::Stack::push); //
        pushInteger(code, Alignment::LocalHeapEnd);
        code.push(Instruments::Stack::push);
        pushInteger(code, Alignment::GlobalPtr + globalScope.nameTable->localHeapSize());
        code.push(Instruments::Heap::store);


        // embedded utilities
        code.push(Instruments::Flow::jump);
        pushInteger(code, Alignment::LabelUserCodeBegin);

        // [jumped][unjumped][value] zerojump
        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorZero);
        code.push(Instruments::Flow::zerojump);
        pushInteger(code, Alignment::LabelComparatorZero2);
        code.push(Instruments::Stack::swap);
        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorZero2);
        code.push(Instruments::Stack::discard);
        code.push(Instruments::Flow::retun);

        // [jumped][unjumped][value] negativejump
        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorNegative);
        code.push(Instruments::Flow::negativejump);
        pushInteger(code, Alignment::LabelComparatorNegative2);
        code.push(Instruments::Stack::swap);
        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorNegative2);
        code.push(Instruments::Stack::discard);
        code.push(Instruments::Flow::retun);

        // [value1][value2] and
        // 言語仕様上重実装
        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorAnd); // [v1][v2]

        code.push(Instruments::Flow::zerojump);
        pushInteger(code, Alignment::LabelComparatorAnd2); // [v1]
        code.push(Instruments::Stack::duplicate); // Dammy
        code.push(Instruments::Flow::zerojump);
        pushInteger(code, Alignment::LabelComparatorAnd2); // [v1]

        code.push(Instruments::Stack::discard); // [j][u]
        code.push(Instruments::Stack::push);
        pushInteger(code, 1);
        code.push(Instruments::Flow::retun);

        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorAnd2);
        code.push(Instruments::Stack::discard);
        code.push(Instruments::Stack::push);
        pushInteger(code, 0);

        code.push(Instruments::Flow::retun);

        // [value1][value2] or
        // 言語仕様上重実装
        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorOr); // [v1][v2]

        code.push(Instruments::Flow::zerojump);
        pushInteger(code, Alignment::LabelComparatorOr2); // [v1]
        code.push(Instruments::Stack::discard);
        code.push(Instruments::Stack::push);
        pushInteger(code, 1);
        code.push(Instruments::Flow::retun);

        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorOr2);

        code.push(Instruments::Flow::zerojump);
        pushInteger(code, Alignment::LabelComparatorOr3);
        code.push(Instruments::Stack::push);
        pushInteger(code, 1);
        code.push(Instruments::Flow::retun);

        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelComparatorOr3);
        code.push(Instruments::Stack::push);
        pushInteger(code, 0);
        code.push(Instruments::Flow::retun);

        // terminal
        code.push(Instruments::Flow::label);
        pushInteger(code, Alignment::LabelUserCodeBegin);
    }


    void attachEmbeddedFooter(WhiteSpace& code, const StatementScope& globalScope) {

        auto& mainEntry = globalScope.nameTable->getLocal("main");
        if (!typeis<NameEntryFunction>(mainEntry))
            throw GenerationException();

        code.push(Instruments::Flow::call);
        pushInteger(code, solveLabel(mainEntry.address()));
        code.push(Instruments::Flow::exit);
    }

}



int main(int argc, char** argv) {
    using namespace WS;
    using namespace Parser;
    using namespace Compiler;
    using namespace Builder;

    // embedded definition

    reservedNameTable.defineEmbeddedFunction("__puti", Embedded::Function::IDputi, 1);
    reservedNameTable.defineEmbeddedFunction("__putc", Embedded::Function::IDputc, 1);
    reservedNameTable.defineEmbeddedFunction("__geti", Embedded::Function::IDgeti, 0);
    reservedNameTable.defineEmbeddedFunction("__getc", Embedded::Function::IDgetc, 0);
    reservedNameTable.defineEmbeddedFunction("__getiv", Embedded::Function::IDgetiv, 1);
    reservedNameTable.defineEmbeddedFunction("__getcv", Embedded::Function::IDgetcv, 1);

    // embedded definition

    // analysis

    TokenStream tokenStream(parseToTokens(cin));

    auto dmy = make_shared<NameTable>();
    StatementScope globalScope = move(*getStatementsScope(tokenStream, dmy, true));

    if (!globalScope.nameTable->includeLocal("main"))
        throw GenerationException();

    WhiteSpace code;

    attachEmbeddedHeader(code, globalScope);
    convertScope(code, globalScope);
    attachEmbeddedFooter(code, globalScope);

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
