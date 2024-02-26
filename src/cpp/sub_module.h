#ifndef SUBMODULE_H
#define SUBMODULE_H


#include "global_include.h"
#include "gate.h"


class SubModule {
  public:
    SubModule(QString);
    SubModule(QString, const Table&, const Table&);
    inline QString Name() const {return name_;}
    bool AddInput(QString, const NodeGroupPtr);
    bool AddOutput(QString, const NodeGroupPtr);
    void SetInputs(const Table&);
    void SetOutputs(const Table&);
    inline Table& GetAllInputs() {return inputs_;}
    inline Table& GetAllOutputs() {return outputs_;}
    inline NodeGroupPtr GetInput(QString name){return inputs_.value(name);}
    inline NodeGroupPtr GetOutput(QString name){return outputs_.value(name);}
    bool NotPlaced() const;
    inline bool IsPlaced() const {return !NotPlaced();}
    inline void PlaceAt(int x,int y, int z) {pos_.x = x; pos_.y = y; pos_.z = z;}
    inline void Move(int dx, int dy, int dz) {pos_.x += dx; pos_.y += dy; pos_.z += dz;}
    inline int GetX() const {return pos_.x;}
    inline int GetY() const {return pos_.y;}
    inline int GetZ() const {return pos_.z;}

  private:
    QString name_;
    Table inputs_;
    Table outputs_;
    Gate::Pos pos_;
};


#endif // SUBMODULE_H
