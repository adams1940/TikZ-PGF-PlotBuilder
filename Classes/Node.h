#ifndef NODE_HPP
#define NODE_HPP

class Node{
private:

public:
  TString OutlineColor, FillColor;
  vector<TString> Options;
  TString Anchor, Align;
  double AnchorX, AnchorY; // axis cs
  double ShiftX, ShiftY; // mm
  TString Text;
  bool IncludeDraw;
  
  Node(double x = 0, double y = 0):AnchorX(x),AnchorY(y){
    Anchor = "center";
    Align = "center";
    IncludeDraw = "true";
    ShiftX = 0;
    ShiftY = 0;
  }
  virtual ~Node(){}

  void SetAnchorPosition(double x, double y){
    AnchorX = x;
    AnchorY = y;
  }

  void Shift(double x, double y){
    ShiftX = x;
    ShiftY = y;
  }

  virtual TString NodeText(){
    if( OutlineColor!="" ) Options.push_back(Form("color=%s",OutlineColor.Data()));
    if( FillColor!="" ) Options.push_back(Form("fill=%s",FillColor.Data()));
    if( ShiftX!=0 ) Options.push_back(Form("xshift=%fmm",ShiftX));
    if( ShiftY!=0 ) Options.push_back(Form("yshift=%fmm",ShiftY));
    Options.push_back(Form("anchor=%s",Anchor.Data()));
    Options.push_back(Form("align=%s",Align.Data()));
    TString OptionsWithCommas;
    for( TString Option:Options ) OptionsWithCommas.Append(Form("%s, ",Option.Data()));
    return Form("\\node[%s%s] at (axis cs: %f,%f){%s};",OptionsWithCommas.Data(),IncludeDraw?"draw":"",AnchorX,AnchorY,Text.Data());
  }
};
#endif