#include <tr1/functional>
int main() {
    std::tr1::function<int (int)> foo =
            [] (int n) { return n+1; };
    return foo(5);
}
