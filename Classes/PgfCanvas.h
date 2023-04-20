#ifndef PGFCANVAS_HPP
#define PGFCANVAS_HPP

#include "Graph.h"
#include "Axis.h"
#include "Node.h"

class PgfCanvas{
private:

public:
  int ActivePadX, ActivePadY;
  int NumDivisionsX, NumDivisionsY;
  double Width, Height; // mm
  vector<Axis> XAxes, YAxes;
  vector<Graph> Graphs[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]
  vector<TString> AdditionalNodes[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]
  vector<Node*> Nodes[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]
  vector<vector<bool>> DrawZeroLinesVector;
  double XLabelOffsetY, YLabelOffsetX;

  PgfCanvas(int NX = 1, int NY = 1):NumDivisionsX(NX),NumDivisionsY(NY){
    Width = 100/(double)NX;
    Height = 0.9*Width;
    XAxes.resize(NX);
    YAxes.resize(NY);
    XLabelOffsetY = -0.07;
    YLabelOffsetX = -0.161;
    for( int iX=0; iX<NX; iX++ ) DrawZeroLinesVector.push_back(vector<bool>(NY));
    cd();
  }
  virtual ~PgfCanvas(){}

  void cd(int X = 0, int Y = 0){
    if( X>=NumDivisionsX || Y>=NumDivisionsY ){
      cout<<"Error: This pad doesn't exist!"<<endl;
      gApplication->Terminate();
    }
    ActivePadX = X;
    ActivePadY = Y;
  }

  Axis& ActiveXAxis(){ return XAxes[ActivePadX]; }
  Axis& ActiveYAxis(){ return YAxes[ActivePadY]; }

  void AddGraph(Graph gr){
    Graphs[ActivePadX][ActivePadY].push_back(gr);
  }

  void SetXTitle(TString Title){
    XAxes[ActivePadX].Title = Title;
  }

  void SetYTitle(TString Title){
    YAxes[ActivePadY].Title = Title;
  }

  void SetXYTitles(TString XTitle, TString YTitle){
    for( Axis &axis:XAxes ) axis.Title = XTitle;
    for( Axis &axis:YAxes ) axis.Title = YTitle;
  }

  void SetXRange(double Min, double Max){
    XAxes[ActivePadX].Min = Min;
    XAxes[ActivePadX].Max = Max;
  }

  void SetYRange(double Min, double Max){
    YAxes[ActivePadY].Min = Min;
    YAxes[ActivePadY].Max = Max;
  }

  void SetXRanges(double Min, double Max){
    for( Axis &axis:XAxes ){
      axis.Min = Min;
      axis.Max = Max;
    }
  }

  void SetYRanges(double Min, double Max){
    for( Axis &axis:YAxes ){
      axis.Min = Min;
      axis.Max = Max;
    }
  }

  void SetLogX(){
    for( Axis &axis:XAxes ) axis.IsLog=true;
  }

  void DrawZeroLines(){
    for( int iX=0; iX<NumDivisionsX; iX++ ){
      for( int iY=0; iY<NumDivisionsY; iY++ ){
        DrawZeroLinesVector[iX][iY] = true;
      }
    }
  }

  void DrawZeroLine(){
    DrawZeroLinesVector[ActivePadX][ActivePadY] = true;
  }

  void AddNode(TString Text){
    AdditionalNodes[ActivePadX][ActivePadY].push_back(Text);
  }

  void AddNode(Node * node){
    Nodes[ActivePadX][ActivePadY].push_back(node);
  }
};
#endif