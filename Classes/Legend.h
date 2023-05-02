#ifndef LEGEND_HPP
#define LEGEND_HPP

#include "Box.h"
#include "Node.h"

class Legend{
private:
  vector<double> XShifts, YShifts;
  TextBox Title;
public:
  int NumColX, NumRowY;
  double CenterX, CenterY;
  double BorderWidth, BorderHeight;
  vector<Marker> Markers; // Only storing this so we can copy Graph::MarkerStyles and fill Nodes with the addresses
  vector<TextBox> TextBoxes;
  Box Border;

  Legend(int NX, int NY):NumColX(NX),NumRowY(NY){
    CenterX = 0;
    CenterY = 0;
    BorderWidth = 0;
    BorderHeight = 0;
    Border.FillColor = "{rgb:black,1;white,25}";
  }
  virtual ~Legend(){};

  void SetXShifts(vector<double> shifts){
    if( shifts.size()!=NumColX ){
      cout<<"Legend::SetXShifts passed vector of incorrected size."<<endl;
      return;
    }
    XShifts = shifts;
  }
  void SetYShifts(vector<double> shifts){
    if( shifts.size()!=NumRowY ){
      cout<<"Legend::SetYShifts passed vector of incorrected size."<<endl;
      return;
    }
    YShifts = shifts;
  }

  void SetCenter(double x, double y){
    CenterX = x;
    CenterY = y;
    Border.SetAnchorPosition(x,y);
  }

  void SetBorderSize(double w, double h){
    BorderWidth = w;
    BorderHeight = h;
    Border.Width = w;
    Border.Height = h;
  }

  void SetFillColor(TString Color){
    Border.FillColor = Color;
  }

  bool ColXIsValid(int ColX){
    return ColX>=0 && ColX<NumColX;
  }
  bool RowYIsValid(int RowY){
    return RowY>=0 && RowY<NumRowY;
  }
  bool ColAndRowAreValid(int ColX, int RowY){
    return ColXIsValid(ColX) && RowYIsValid(RowY);
  }

  void AddText(int ColX, int RowY, TString Text, TString Anchor){
    if( !ColAndRowAreValid(ColX,RowY) ){
      cout<<"Legend::AddText given invalid Column or Row"<<endl;
      return;
    }
    TextBox text;
    text.SetAnchorPosition(CenterX,CenterY);
    text.Shift(XShifts[ColX],YShifts[RowY]);
    text.Anchor = Anchor;
    text.Text = Text;
    TextBoxes.push_back(text);
  }

  void AddMarker(int ColX, int RowY, Graph graph){
    if( !ColAndRowAreValid(ColX,RowY) ){
      cout<<"Legend::AddMarker given invalid Column or Row"<<endl;
      return;
    }
    for( Marker mark:graph.MarkerStyles ){
      mark.SetAnchorPosition(CenterX,CenterY);
      mark.Shift(XShifts[ColX],YShifts[RowY]);
      Markers.push_back(mark);
    }
  }

  void SetTitle(TString Text){
    Title.SetAnchorPosition(CenterX,CenterY);
    Title.Text = Text;
  }
  void SetTitleShift(double x, double y){
    Title.Shift(x,y);
  }

  vector<Node *> Nodes(){
    vector<Node *> nodes;
    nodes.push_back(&Border);
    nodes.push_back(&Title);
    for( Node &tb:TextBoxes ) nodes.push_back(&tb);
    for( Node &mark:Markers ) nodes.push_back(&mark);
    return nodes;
  }

};

#endif