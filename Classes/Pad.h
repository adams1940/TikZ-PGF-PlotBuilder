#ifndef PAD_HPP
#define PAD_HPP

#include "Axis.h"
#include "Node.h"
#include "Graph.h"
#include "TextBox.h"
#include "Legend.h"

class Pad{
private:

public:
  Axis XAxis, YAxis;
  vector<Graph> Graphs;
  vector<Node *> Nodes; // By storing pointers, we can call functions overloaded by derived classes. But it's a tradeoff...
  bool DrawZeroLine;
  double XLabelOffsetY, YLabelOffsetX;
  double Width, Height; // mm
  double SubPadX, SubPadY;

  Pad(){
    XLabelOffsetY = -0.07;
    YLabelOffsetX = -0.161;
    DrawZeroLine = false;
    Width = 100;
    Height = 0.9*Width;
    SubPadX = 0;
    SubPadY = 0;
  }
  virtual ~Pad(){}

  void AddGraph(Graph gr){ Graphs.push_back(gr); }
  void SetLogX(){ XAxis.IsLog = true; }
  void SetLogY(){ YAxis.IsLog = true; }
  void SetXTitle(TString Title){ XAxis.Title = Title; }
  void SetYTitle(TString Title){ YAxis.Title = Title; }
  void SetXYTitle(TString XTitle, TString YTitle){ 
    SetXTitle(XTitle);
    SetYTitle(YTitle);
  }
  void SetXRange(double Min, double Max){
    XAxis.Min = Min;
    XAxis.Max = Max;
  }

  void SetYRange(double Min, double Max){
    YAxis.Min = Min;
    YAxis.Max = Max;
  }

  void SetPlottingStyle(Pad &pad){
    XAxis = pad.XAxis;
    YAxis = pad.YAxis;
    DrawZeroLine = pad.DrawZeroLine;
  }

  void AddNode(Node * node){ Nodes.push_back(node); }

  void AddLegend(Legend &legend){
    for( Node * node:legend.Nodes() ) Nodes.push_back(node);
  }

};
#endif