#ifndef MARKER_HPP
#define MARKER_HPP

#include "Node.h"

class Marker : public Node{
private:

public:
  double Size, OutlineWidthScale;
  TString Shape;
  double StarPointRatio;

  Marker(double x = 0, double y = 0){
    AnchorX = x;
    AnchorY = y;
    StarPointRatio = 2.618034;
    OutlineColor="black";
    FillColor = "blue";
    Size = 3; // mm
    OutlineWidthScale = 0.05;
    Shape = "circle";
  }
  virtual ~Marker(){}

  TString NodeText(){
    Options.push_back("inner sep=0pt");
    Options.push_back(Form("minimum size=%fmm",Size));
    Options.push_back(Form("line width=%fmm",OutlineWidthScale*Size));
    Options.push_back(Shape.Data());
    if( Shape=="star" ) Options.push_back(Form("star point ratio=%f",StarPointRatio));
    return Node::NodeText();
  }
};
#endif