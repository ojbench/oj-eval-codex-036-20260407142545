#include <bits/stdc++.h>
using namespace std;

struct dynamic_bitset {
    static constexpr size_t B = 64;
    vector<uint64_t> w; // words
    size_t n = 0;       // number of valid bits

    dynamic_bitset() = default;
    ~dynamic_bitset() = default;
    dynamic_bitset(const dynamic_bitset&) = default;
    dynamic_bitset& operator=(const dynamic_bitset&) = default;

    dynamic_bitset(size_t sz) { resize_zero(sz); }

    dynamic_bitset(const string &str) {
        n = 0;
        w.clear();
        w.reserve((str.size() + B - 1) / B);
        for (char c : str) {
            push_back(c == '1');
        }
    }

    static size_t words_for(size_t sz){ return (sz + B - 1) / B; }

    void resize_zero(size_t sz){
        n = sz;
        w.assign(words_for(n), 0ULL);
        trim_unused_bits();
    }

    void trim_unused_bits(){
        if (w.empty()) return;
        size_t valid = n % B;
        if (valid == 0) return;
        uint64_t mask = (valid == 64) ? ~0ULL : ((1ULL << valid) - 1ULL);
        w.back() &= mask;
    }

    bool operator[](size_t i) const {
        if (i >= n) return false; // defensive; judge may not query out-of-range
        return (w[i / B] >> (i % B)) & 1ULL;
    }

    dynamic_bitset& set(size_t i, bool val = true){
        if (i >= n) return *this; // ignore invalid set as per typical vector-like undefined; keep safe
        if (val) w[i / B] |= (1ULL << (i % B));
        else     w[i / B] &= ~(1ULL << (i % B));
        return *this;
    }

    dynamic_bitset& push_back(bool val){
        if (n % B == 0) w.push_back(0ULL);
        if (val) w[n / B] |= (1ULL << (n % B));
        ++n;
        return *this;
    }

    bool none() const {
        for (size_t i = 0; i < w.size(); ++i) if (w[i]) return false;
        return true;
    }

    bool all() const {
        if (n == 0) return true;
        for (size_t i = 0; i + 1 < w.size(); ++i) if (~w[i]) return false;
        // check last word only valid bits
        size_t valid = n % B;
        uint64_t mask = (valid ? ((1ULL << valid) - 1ULL) : ~0ULL);
        return (w.back() & mask) == mask;
    }

    size_t size() const { return n; }

    dynamic_bitset& operator|=(const dynamic_bitset& other){
        size_t m = min(n, other.n);
        size_t mw = m / B; // full words covered by m bits
        for (size_t i = 0; i < mw; ++i) w[i] |= other.w[i];
        size_t rem = m % B;
        if (rem) {
            uint64_t mask = (1ULL << rem) - 1ULL;
            w[mw] = (w[mw] & ~mask) | ((w[mw] | other.w[mw]) & mask);
        }
        return *this;
    }

    dynamic_bitset& operator&=(const dynamic_bitset& other){
        size_t m = min(n, other.n);
        size_t full = words_for(n);
        size_t ow = words_for(other.n);
        size_t mw = m / B;
        for (size_t i = 0; i < mw; ++i) w[i] &= (i < ow ? other.w[i] : 0ULL);
        size_t rem = m % B;
        if (rem) {
            uint64_t mask = (1ULL << rem) - 1ULL;
            uint64_t a = w[mw] & mask;
            uint64_t b = (mw < ow ? other.w[mw] : 0ULL) & mask;
            w[mw] = (w[mw] & ~mask) | ((a & b) & mask);
        }
        // bits beyond m remain unchanged
        return *this;
    }

    dynamic_bitset& operator^=(const dynamic_bitset& other){
        size_t m = min(n, other.n);
        size_t mw = m / B;
        for (size_t i = 0; i < mw; ++i) w[i] ^= other.w[i];
        size_t rem = m % B;
        if (rem) {
            uint64_t mask = (1ULL << rem) - 1ULL;
            w[mw] ^= (other.w[mw] & mask);
        }
        return *this;
    }

    dynamic_bitset& operator<<=(size_t k){
        if (k == 0 || n == 0) return *this;
        // increase length by k, fill lower k with 0
        size_t old_n = n;
        size_t new_n = n + k;
        vector<uint64_t> oldw = w;
        w.assign(words_for(new_n), 0ULL);

        size_t word_shift = k / B;
        size_t bit_shift  = k % B;
        for (size_t i = 0; i < words_for(old_n); ++i) {
            size_t j = i + word_shift;
            w[j] |= oldw[i] << bit_shift;
            if (bit_shift && j + 1 < w.size()) w[j + 1] |= oldw[i] >> (B - bit_shift);
        }
        n = new_n;
        trim_unused_bits();
        return *this;
    }

    dynamic_bitset& operator>>=(size_t k){
        if (k == 0) return *this;
        if (k >= n) { n = 0; w.clear(); return *this; }
        size_t old_n = n;
        size_t new_n = n - k;
        vector<uint64_t> oldw = w;
        w.assign(words_for(new_n), 0ULL);
        size_t word_shift = k / B;
        size_t bit_shift  = k % B;
        for (size_t i = 0; i < words_for(new_n); ++i) {
            size_t j = i + word_shift;
            uint64_t part = oldw[j] >> bit_shift;
            if (bit_shift && j + 1 < oldw.size()) part |= oldw[j + 1] << (B - bit_shift);
            w[i] = part;
        }
        n = new_n;
        trim_unused_bits();
        return *this;
    }

    dynamic_bitset& set(){
        if (n == 0) return *this;
        for (auto &x : w) x = ~0ULL;
        trim_unused_bits();
        return *this;
    }

    dynamic_bitset& flip(){
        for (auto &x : w) x = ~x;
        trim_unused_bits();
        return *this;
    }

    dynamic_bitset& reset(){
        for (auto &x : w) x = 0ULL;
        return *this;
    }

    // helper for local tests
    string to_string() const {
        string s;
        s.reserve(n);
        for (size_t i = 0; i < n; ++i) s.push_back((*this)[i] ? '1' : '0');
        return s;
    }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Provide a simple interpreter for local testing.
    // Commands (space separated):
    // new N | from STR | size ID | get ID POS | set ID POS VAL | push ID VAL
    // none ID | all ID | or A B | and A B | xor A B
    // shl ID K | shr ID K | setall ID | flip ID | reset ID | print ID

    vector<dynamic_bitset> bs;
    string cmd;
    while (cin >> cmd){
        if (cmd == "new"){
            size_t n; if (!(cin >> n)) break; bs.emplace_back(n); cout << bs.size()-1 << '\n';
        } else if (cmd == "from"){
            string s; if (!(cin >> s)) break; bs.emplace_back(dynamic_bitset(s)); cout << bs.size()-1 << '\n';
        } else if (cmd == "size"){
            size_t id; if (!(cin >> id)) break; cout << bs[id].size() << '\n';
        } else if (cmd == "get"){
            size_t id, pos; if (!(cin >> id >> pos)) break; cout << (bs[id][pos] ? 1 : 0) << '\n';
        } else if (cmd == "set"){
            size_t id, pos; int v; if (!(cin >> id >> pos >> v)) break; bs[id].set(pos, v!=0);
        } else if (cmd == "push"){
            size_t id; int v; if (!(cin >> id >> v)) break; bs[id].push_back(v!=0);
        } else if (cmd == "none"){
            size_t id; if (!(cin >> id)) break; cout << (bs[id].none()?1:0) << '\n';
        } else if (cmd == "all"){
            size_t id; if (!(cin >> id)) break; cout << (bs[id].all()?1:0) << '\n';
        } else if (cmd == "or"){
            size_t a,b; if (!(cin >> a >> b)) break; bs[a] |= bs[b];
        } else if (cmd == "and"){
            size_t a,b; if (!(cin >> a >> b)) break; bs[a] &= bs[b];
        } else if (cmd == "xor"){
            size_t a,b; if (!(cin >> a >> b)) break; bs[a] ^= bs[b];
        } else if (cmd == "shl"){
            size_t id,k; if (!(cin >> id >> k)) break; bs[id] <<= k;
        } else if (cmd == "shr"){
            size_t id,k; if (!(cin >> id >> k)) break; bs[id] >>= k;
        } else if (cmd == "setall"){
            size_t id; if (!(cin >> id)) break; bs[id].set();
        } else if (cmd == "flip"){
            size_t id; if (!(cin >> id)) break; bs[id].flip();
        } else if (cmd == "reset"){
            size_t id; if (!(cin >> id)) break; bs[id].reset();
        } else if (cmd == "print"){
            size_t id; if (!(cin >> id)) break; cout << bs[id].to_string() << '\n';
        } else {
            // unknown command -> stop
            break;
        }
    }
    return 0;
}

