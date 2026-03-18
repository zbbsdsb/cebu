#include <cebu/simplicial_complex.h>

int main() {
    cebu::SimplicialComplex sc;
    sc.add_vertex(0);
    sc.add_vertex(1);
    sc.add_edge(0, 1);
    return 0;
}