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

    namespace Instruments {

    }

    struct WhiteSpace {
        vector<Chr> raw;

        WhiteSpace() :raw() {}

        inline void push(Chr c) {
            raw.push_back(c);
        }
        inline void push(initializer_list<Chr> cl) {
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
            for (auto c : raw)
                os << (
                    c == Chr::SP ? char(32) :
                    c == Chr::TB ? char(9) :
                    c == Chr::LF ? char(10) : char(0)
                    );
        }
    };
    inline ostream& operator <<(ostream& os, WhiteSpace& ws) { ws.print(os); return os; }


    // 


    WhiteSpace convert_integer(integer val) {
        WhiteSpace stk;

        // 雑
        for (int i = sizeof(integer) * 8 - 1; 0 <= i; --i)
            stk.push((val >> i) & 1);

        stk.push(Chr::LF);
        return stk;
    }
}


//


int main() {
    using namespace WS;
    integer x;
    cin >> x;
    WhiteSpace code;
    code.push({ Chr::SP, Chr::SP });
    code.push(convert_integer(x));
    code.push({ Chr::TB, Chr::LF,  Chr::SP, Chr::TB });
    code.push({ Chr::LF, Chr::LF, Chr::LF });
    cout << code << flush;
    return 0;
}

