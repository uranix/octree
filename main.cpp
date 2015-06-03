#include "octree.h"

#include <memory>
#include <vector>

struct CellData {
    double p;
    double s;
};

struct FaceData {
    double W;
    double B1, B2;
};

int main() {
    typedef Cell<CellData, FaceData, 2> CellType;
    typedef CellType::FaceType FaceType;

    auto c = std::unique_ptr<CellType>(new CellType());
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++) {
            c->faces[i][j] = new FaceType();
            c->faces[i][j]->side[1-j] = c.get();
        }

    c->refine();
    c->children[1]->refine();
    return 0;
}
