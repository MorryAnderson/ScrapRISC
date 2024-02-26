#ifndef MODULE_H
#define MODULE_H


#include "global_include.h"
#include "gate.h"
#include <QMap>


class Module {
  public:
    Module(QString name);
    Module(QString n, const Nodes& w, const Nodes& g, const Table& v, const Table& i, const Table& o);
    ~Module();

    ModulePtr Clone();
    inline void Rename(QString name){name_ = name;}
    inline QString Name() const {return name_;}

    bool VariableExist(QString) const;
    bool SubExist(QString) const;

    inline Nodes& AllWires(){return wires_;};
    inline Nodes& AllGates(){return gates_;}
    inline const Nodes GetAllNodes() const {return wires_ + gates_;}
    inline NodeGroupPtr GetInput(QString name) const {return inputs_.value(name);}
    inline NodeGroupPtr GetOutput(QString name) const {return outputs_.value(name);}
    inline const Table& GetAllInputs() const {return inputs_;}
    inline const Table& GetAllOutputs() const {return outputs_;}
    NodeGroupPtr GetVariable(QString) const;
    SubPtr GetSubByName(QString) const;  // only the last one
    SubPtr GetSubByIndex(int) const;
    inline const Subs& GetAllSubs() const {return subs_;}

    inline int NumGates() const {return gates_.size();}
    inline int NumSignals() const {return num_signals_;}
    inline int NumInputs() const {return inputs_.size();}
    inline int NumOutputs() const {return outputs_.size();}
    inline int NumSubs() const {return subs_.size();}
    inline int NumDebugs() const {return debugs_.size();}

    bool NotPlaced() const;
    inline bool IsPlaced() const {return !NotPlaced();}
    void PlaceAt(int x,int y, int z);
    void Move(int dx, int dy, int dz);
    void CalcSize();
    inline int GetX() const {return pos_.x;}
    inline int GetY() const {return pos_.y;}
    inline int GetZ() const {return pos_.z;}
    inline int GetWidth() const {return width_;}
    inline int GetHeight() const {return height_;}
    inline void SetWidth(int w){width_ = w;}
    inline void SetHeight(int h){height_ = h;}

    SignalPtr CreateSignal(QString name, int width, Gate::Mode mode);
    SignalPtr CreateSignal(QString name, int width, QString mode);
    TunnelPtr CreateTunnel(QString name, int width);
    TunnelPtr CreateInput(QString name, int width);
    TunnelPtr CreateOutput(QString name, int width);
    SubPtr    CreateSub(QString name, const Table& in, const Table& out);

    SubPtr    CreateSubCached(QString name);
    void      AddCachedSubPortIn(QString port_name, const NodeGroupPtr);
    void      AddCachedSubPortOut(QString port_name, const NodeGroupPtr);

    bool CreateDebug(QString name);

//    bool Connect(QString src_name, QString dst_name, int lsb = 0);
    bool Connect(VarInfo src, VarInfo dst, int lsb = 0);
    void RemoveUnconnected01();

    void Manifest(int x_len, int y_len);
    inline void ForceManifested() {is_manifested_ = true;}
    inline bool IsManifested() const {return is_manifested_;}
    void SetFixed(bool fixed = true);

    void PrintInfo() const;
    void PrintConnectionInfo() const;
    void PrintGateInfo() const;
    void SearchThroughout() const;

  public:
    static void ManifestPort(const Table&, bool is_output, int x_offset, int *x_len = nullptr);
    static void ManifestDebugs(const Groups debugs, int x, int y);
    void ManifestGates(const Table& vars, int x, int y, int height);  // height is len in Y axis (not Z)

  private:
    void CloneNodes(ModulePtr);
    void CloneVariables(ModulePtr);
    void UpdataNodes(NodePtr);
    void UpdatePorts(ModulePtr);
    void UpdateSubs(ModulePtr);
    void UpdateTable(Table&);
    void CutBoundary(ModulePtr);

  private:
    static const int MODULE_Y_SIZE = 16;

    QString name_;
    Nodes wires_;
    Nodes gates_;
    Table variables_;
    Table inputs_;
    Table outputs_;
    Subs subs_;
    Groups debugs_;

    Table cached_sub_port_connection_in_;
    Table cached_sub_port_connection_out_;


    int width_;  // along x
    int height_;  // along y
    Gate::Pos pos_;

    int num_signals_;
    bool is_manifested_;
    bool gnd_is_connected_to_sub_;
    bool vcc_is_connected_to_sub_;
};


#endif // MODULE_H
