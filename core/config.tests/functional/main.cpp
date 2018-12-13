#include <functional>
int main() {
    std::function<int (int)> foo =
            [] (int n) { return n+1; };
    return foo(5);
}
