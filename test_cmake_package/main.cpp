#include <cebu/simplicial_complex.h>

int main() {
    cebu::SimplicialComplex sc;
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    sc.add_edge(v1, v2);
    return 0;
}