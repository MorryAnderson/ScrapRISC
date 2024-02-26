#ifndef MATRIX_H
#define MATRIX_H


#include "global_include.h"
#include "gate.h"


class Matrix {

  public:
    typedef struct  {
        quint16 id;
        quint8  mode;
        Gate::Orient orient;
        Gate::Pos pos;
        quint32 color;
    } GateInfo;
    typedef QVector<GateInfo> GateInfoLib;

    typedef QVector<int> WiringInfo;
    typedef QVector<WiringInfo> WiringInfoLib;
    typedef WiringInfo SubInfo;
    typedef WiringInfoLib SubInfoLib;

    typedef struct {
        char dir;
        quint8 width;
        quint8 name_len;
        QString name;
        WiringInfoLib connection;
    } PortInfo;
    typedef QVector<PortInfo> PortInfoLib;

    typedef struct {
      QString output_name;
      int output_index;
      QString input_name;
      int input_index;
    } ThroughoutInfo;
    typedef QVector<ThroughoutInfo> ThroughoutInfoLib;

  public:
    Matrix();
    ~Matrix();

    void SetTop(ModulePtr top);
    QString GetTopName() const;
    inline int Width() const {return width_;}
    inline int Height() const {return height_;}
    inline int NumGates() const {return gates_.size();}
    inline int NumPorts() const {return inputs_.size() + outputs_.size();}
    void CalcSize();
    bool CheckFanout(); // call this after GenerateInfo();
    bool CheckConnection();  // call this after GenerateInfo();
    bool CheckOverlapping();  // call this after GenerateInfo();
    bool CheckAsymmetric();  // call this after GenerateInfo();

    QString NextSubName() const;
    bool SubsNotEmpty() const;
    bool ConnectSub(ModulePtr module);

    void DisplaceAllGates();
    void ManifestHierarchy();
    void ManifestPlane(int x_len, int y_len);
    void ManifestCube(int x_len, int y_len);
    void ReBrush();

    bool GenerateInfo();

    void PrintNumInfo() const;
    void PrintGateInfo() const;
    void PrintWiringInfo() const;
    void PrintModuleInfo() const;
    inline const GateInfoLib& GetGateInfo() const {return gate_info_;}
    inline const WiringInfoLib& GetWiringInfo() const {return wiring_info_;}
    inline const PortInfoLib& GetPortInfo() const {return port_info_;}
    inline const ThroughoutInfoLib& GetThroughoutInfo() const {return throughout_info_;}

    void ClearStatistics();
    void Statistics();
    inline int NumGateType(Gate::Mode mode) const {return num_gate_type_[(int)mode];}
    inline int NumWirings() const {return num_wirings_;}
    QString Mode2Str(int mode) const;

  private:
    void GenerateGateInfo();
    void GeneratePortInfo(bool is_output);
    void GenerateThroughoutInfo();
    SubInfo GetAllSubsOf(int index);
    void MoveModules(SubInfo indexes, int x, int y);
    int GetSubsBoundX(int min, SubInfo indexes);

  private:
    static const quint32 DEFAULT_COLOR = 0xdf7f01;
    static const quint32 INPUT_COLOR   = 0x00ee00;
    static const quint32 OUTPUT_COLOR  = 0x0000ee;
    static const quint32 DEBUG_COLOR   = 0xee00ee;

  private:
    ModulePtr top_;
    int width_;
    int height_;
    Nodes wires_;
    Nodes gates_;
    Table inputs_;
    Table outputs_;
    Subs subs_;
    Modules modules_;

  private:
    GateInfoLib gate_info_;
    WiringInfoLib wiring_info_;
    PortInfoLib port_info_;
    ThroughoutInfoLib throughout_info_;
    SubInfoLib sub_info_;

    int num_gate_type_[6];
    int num_wirings_;

    typedef struct {
        int num_instance;
        int num_gate;
        int width;
        int height;
    } ModuleInfo;

    QMap<QString, ModuleInfo> module_info_;

  public:
    void DebugGateInfo() const;
    void DebugNodeInfo() const;
};


#endif // MATRIX_H
