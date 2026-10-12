#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>

std::size_t longest_unique_subarray(const std::vector<int>& values) {
    std::unordered_map<std::size_t, std::size_t> st;

    std::size_t n = values.size();
    std::size_t l = 0;
    std::size_t r = 0;
    std::size_t len = 0;

    while (r < n) {
        auto it = st.find(values[r]);
        if (it != st.end() && it->second >= l) {
            l = it.second + 1;
        }
        
        st[values[r]] = r;
        len = std::max(len, r - l + 1);
        ++r;
    }

    return len;
}

int main(void) {

}