#ifndef __OCTREE_H__
#define __OCTREE_H__

#include <cstdlib>
#include <stdexcept>
#include <cassert>
#include <iostream>

struct Data {
    void gather() { }
    void scatter() { }
};

template<class DT>
struct Storage {
    DT data;
};

template<class T, int nc>
struct Hierarchy {
    T *parent;
    T *children[nc];
    int childno;
    Hierarchy(T *parent, int childno) : parent(parent), childno(childno) {
        for (int i = 0; i < nc; i++)
            children[i] = NULL;
    }
    bool isRoot() { return parent == NULL; }
    bool isLeaf() { return children[0] == NULL; }
    void makeChildren() {
        for (int i = 0; i < nc; i++) {
            assert(children[i] == NULL);
            children[i] = new T(static_cast<T *>(this), i);
        }
    }
    void killChildren() {
        for (int i = 0; i < nc; i++) {
            delete children[i];
            children[i] = NULL;
        }
    }
};

template<class CellData, class FaceData, int dim>
struct Cell;

template<class CellData, class FaceData, int dim>
struct Face : public Storage<FaceData>, public Hierarchy<Face<CellData, FaceData, dim>, 1 << dim> {
    typedef Cell<CellData, FaceData, dim+1> CellType;
    CellType *side[2];
    explicit Face(Face *parent = NULL, int childno = -1)
        : Hierarchy<Face<CellData, FaceData, dim>, 1 << dim>(parent, childno)
    {
        side[0] = side[1] = NULL;
    }
    bool orphan() {
        return (side[0] == NULL) && (side[1] == NULL);
    }
    void release(CellType *cell, int d, int s) {
        std::cout << "cell = " << cell << " d = " << d << " s = " << s << std::endl;
        assert(cell == side[1-s]);
        cell->faces[d][s] = NULL;
        side[1-s] = NULL;
        if (orphan()) {
            if (this->parent)
                this->parent->children[this->childno] = NULL;
            delete this;
        }
    }
    void setCell(CellType *cell, int s) {
        side[s] = cell;
    }
};

template<class CellData, class FaceData,int dim>
struct Cell : public Storage<CellData>, public Hierarchy<Cell<CellData, FaceData, dim>, 1 << dim> {
    typedef Face<CellData, FaceData, dim-1> FaceType;
    FaceType *faces[dim][2];
    explicit Cell(Cell *parent = NULL, int childno = -1)
        : Hierarchy<Cell<CellData, FaceData, dim>, 1 << dim>(parent, childno)
    {
        for (int i = 0; i < dim; i++)
            for (int j = 0; j < 2; j++)
                faces[i][j] = NULL;
    }
    void refine() {
        if (!this->isLeaf())
            throw std::logic_error("Attept to refine a node that is not a leaf");
        this->makeChildren();
    }
    void coarsen() {
        if (this->isLeaf())
            return;
        this->killChildren();
    }
    ~Cell() {
        std::cout << "B " << __PRETTY_FUNCTION__ << " this = " << this << " parent = " << this->parent << std::endl;
        this->killChildren();
        for (int d = 0; d < dim; d++)
            for (int s = 0; s < 2; s++)
                faces[d][s]->release(this, d, s);
        std::cout << "E " << __PRETTY_FUNCTION__ << " this = " << this << " parent = " << this->parent << std::endl;
    }
};

#endif
