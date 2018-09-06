#pragma GCC optimize ("O3")
#include "bits/stdc++.h"


using namespace std;
using integer = int64_t;


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

    // isdigit

    template<typename T> T getNumber(istream& is) {
        T var = 0; T sign = 1;
        int cc = is.peek();
        for (; (cc < '0' || '9' < cc) && cc != istream::traits_type::eof(); is.get(), cc = is.peek())
            if (cc == '-') sign = -1;
        for (; '0' <= cc && cc <= '9'; is.get(), cc = is.peek())
            var = (var << 3) + (var << 1) + cc - '0';
        return var * sign;
    }


    ExpressionStatement getStatement(istream& is, int level) {

        unique_ptr<ExpressionStatement> root;
        unique_ptr<ExpressionStatement>* curr = &root;

        int stat = 0;
        while (!is.eof()) {
            int cc = is.peek();

            if (isspace(cc)) {
                is.get(); continue;
            }

            switch (stat) {
            case 0:
            case 2: {
                if (isdigit(cc) || cc == '-') {
                    assert(!(*curr));
                    if (level < 3) {
                        curr->reset(new ExpressionStatement(getStatement(is, level + 1)));
                        stat = 1;
                    }
                    else if (level == 3) {
                        if (isdigit(cc) || cc == '-') {
                            integer val = getNumber<integer>(is);
                            return ExpressionStatement(OperationMode::Integer, val);
                        }
                    }
                }
                break;
            }
            case 1: {
                if (cc == '+' || cc == '-') {

                    if (level < 1) {
                        throw ParseException("internal error?");
                    }
                    else if (level == 1) {
                        auto new_ex = new ExpressionStatement(
                            cc == '-' ? OperationMode::Subtract : OperationMode::Add);
                        (*new_ex).args(0) = move(root);
                        root.reset(new_ex);
                        curr = &(*new_ex).args(1);
                        is.get();
                        stat = 2;
                    }
                    else if (level > 1) {
                        return move(*root);
                    }
                }
                else if (cc == '*' || cc == '/' || cc == '%') {
                    if (level < 2) {
                        throw ParseException("internal error?");
                    }
                    else if (level == 2) {
                        auto new_ex = new ExpressionStatement(
                            cc == '/' ? OperationMode::Divide :
                            cc == '%' ? OperationMode::Modulo :
                            OperationMode::Multiply);
                        (*new_ex).args(0) = move(root);
                        root.reset(new_ex);
                        curr = &(*new_ex).args(1);
                        is.get();
                        stat = 2;
                    }
                    else if (level > 2) {
                        return move(*root);
                    }
                    
                }
                break;
            }
            }
        }
        if (stat == 1) return move(*root);
        else throw ParseException();
    }


    ExpressionStatement getStatement(istream& is) {
        return getStatement(is, 1);
    }
}


//

namespace Compiler {
    using namespace WS;
    using namespace Parser;


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

    ExpressionStatement st = getStatement(cin);

    WhiteSpace code;
    convert_statement(code, st);
    code.push(Instruments::IO::putnumber);
    code.push(Instruments::Flow::exit);

    cout << code << flush;
    return 0;
}

