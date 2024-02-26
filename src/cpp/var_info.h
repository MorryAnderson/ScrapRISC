#ifndef VARINFO_H
#define VARINFO_H


#include "global_include.h"


class VarInfo {
  public:
    VarInfo(const ModulePtr module, QString n, int l, int r, int d=1);
    VarInfo(const ModulePtr module, QString n, int d=1);
    inline QString Name() const {return name_;}
    QString FullName() const;  // with possible range selection

    inline void SetDuplication(int n){duplication_ = n;}

    inline const ModulePtr& GetModule() const {return module_;}
    inline const NodeGroupPtr& GetAllNodes() const {return nodes_;}
    NodePtr GetNode(int i) const;
    int SignalWidth() const;
    int PortWidth() const;

    bool CheckValidation();
    inline bool Checked() const {return nodes_ != nullptr;}

  private:
    QString name_;
    bool ranged_;
    int left_range_;
    int right_range_;
    int duplication_;
    ModulePtr module_;
    NodeGroupPtr nodes_;
};

#endif // VARINFO_H
