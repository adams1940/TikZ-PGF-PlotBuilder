#ifndef BOX_HPP
#define BOX_HPP

#include "Node.h"

class Box : public Node{
private:
  TString Shape;
public:
  double Width, Height;
  Box(double x = 0, double y = 0){
    AnchorX = x;
    AnchorY = y;
    Shape="rectangle";
    FillColor="white";
    Width = 10; //mm
    Height = 10;
  }
  virtual ~Box(){}

  TString LatexLine(){
    Options.push_back(Form("minimum width=%fmm",Width));
    Options.push_back(Form("minimum height=%fmm",Height));
    return Node::LatexLine();
  }
};
#endif