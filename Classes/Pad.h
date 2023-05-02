#ifndef PAD_HPP
#define PAD_HPP

#include "Axis.h"
#include "Node.h"
#include "Graph.h"
#include "TextBox.h"

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

  void DrawLegendMarkerAt(Graph &graph, double x, double ShiftX, double y, double ShiftY){ // can only do this once per graph :/
    for( Marker &Style:graph.MarkerStyles ){
      Style.SetAnchorPosition(x,y);
      Style.ShiftX = ShiftX;
      Style.ShiftY = ShiftY;
      AddNode(&Style);
    }
  }

  TextBox * LegendText(TString Text, double x, double ShiftX, double y, double ShiftY){
    TextBox * text = new TextBox(x,y);
    text->Text = Text;
    text->ShiftX = ShiftX;
    text->ShiftY = ShiftY;
    return text;
  }

  void DrawLegendTitleAt(TString Text, double x, double ShiftX, double y, double ShiftY){
    TextBox * text = LegendText(Text,x,ShiftX,y,ShiftY);
    AddNode(text);
  }

  void DrawLegendTextAt(TString Text, double x, double ShiftX, double y, double ShiftY){
    TextBox * text = LegendText(Text,x,ShiftX,y,ShiftY);
    text->Anchor = "west";
    AddNode(text);
  }

};
#endif