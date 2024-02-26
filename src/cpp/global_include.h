#ifndef GLOBAL_INCLUDE_H
#define GLOBAL_INCLUDE_H


#include <QVector>
#include <QString>
#include <QDebug>
#include <memory>

using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;
using std::static_pointer_cast;


class Node;
class Gate;
class Wire;

class NodeGroup;
class Signal;
class Tunnel;

class Module;
class SubModule;
class Matrix;

class VarInfo;

typedef shared_ptr<Node>      NodePtr;
typedef shared_ptr<Gate>      GatePtr;
typedef shared_ptr<Wire>      WirePtr;
typedef shared_ptr<NodeGroup> NodeGroupPtr;
typedef shared_ptr<Signal>    SignalPtr;
typedef shared_ptr<Tunnel>    TunnelPtr;
typedef shared_ptr<Module>    ModulePtr;
typedef shared_ptr<SubModule> SubPtr;
typedef shared_ptr<Matrix>    MatrixPtr;

typedef QVector<NodePtr>      Nodes;
typedef QVector<GatePtr>      Gates;
typedef QVector<WirePtr>      Wires;
typedef QVector<NodeGroupPtr> Groups;
typedef QVector<SignalPtr>    Signals;
typedef QVector<TunnelPtr>    Tunnels;
typedef QVector<SubPtr>       Subs;
typedef QVector<ModulePtr>    Modules;

typedef QMap<QString, NodeGroupPtr> Table;


#endif // GLOBAL_INCLUDE_H
