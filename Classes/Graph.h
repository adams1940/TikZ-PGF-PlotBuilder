#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "Marker.h"
#include "ErrorBar.h"
#include "Axis.h"

class Graph : public TGraphAsymmErrors{
private:
  pair<double, double> OffsetsOnLogScale(double Value, double RelativeOffset){
    pair<double, double> Offsets(RelativeOffset*exp(2.*log(Value)-log(RelativeOffset*Value+Value)),Value*RelativeOffset);
    return Offsets;
  } // OffsetsOnLogScale

public:
  vector<Marker> MarkerStyles;
  ErrorBar ErrorBarStyle;
  bool DrawXError, DrawYError;
  bool DrawLines;
  bool OnlyDrawLines;
  double LineWidth;
  TString LineColor;
  double XShiftDistance, YShiftDistance;
  TGraphAsymmErrors SystematicErrorGraph;
  double SystematicErrorBoxWidth;
  double LogScaleSystematicErrorBoxWidth;

  Graph(){
    DrawXError = true;
    DrawYError = true;
    DrawLines = false;
    OnlyDrawLines = false;
    LineWidth = 0.2; // mm
    LineColor = "blue";
    XShiftDistance = 0;
    YShiftDistance = 0;
    SystematicErrorBoxWidth = 0.05; // As a fraction of the axis width
    LogScaleSystematicErrorBoxWidth = 0.25; // As... something related to the fraction of the axis width
  }    
  virtual ~Graph(){}

  void AddMarkerStyle(Marker MarkerStyle){
    MarkerStyles.push_back(MarkerStyle);
  }

  vector<TString> SystematicErrorBoxLatexLines(Axis * XAxis = NULL){ // only need to pass XAxis pointer if you want to change error box width... This could be done in some clever way with lengths in mm
    vector<TString> LatexLines;
    for( int iPoint=0; iPoint<SystematicErrorGraph.GetN(); iPoint++ ){
      double MinX, MaxX;
      double X = SystematicErrorGraph.GetPointX(iPoint), Y = SystematicErrorGraph.GetPointY(iPoint);
      if( XAxis ){
        if( !(XAxis->IsLog) ){
          double AxisWidth = XAxis->Max-XAxis->Min;
          MinX = X-.5*SystematicErrorBoxWidth*AxisWidth;
          MaxX = X+.5*SystematicErrorBoxWidth*AxisWidth;
        }
        else{
          MinX = X-OffsetsOnLogScale(X,.5*LogScaleSystematicErrorBoxWidth).first;
          MaxX = X+OffsetsOnLogScale(X,.5*LogScaleSystematicErrorBoxWidth).second;
        }
      } // if( XAxis )
      else{
        MinX = X-SystematicErrorGraph.GetErrorXlow(iPoint);
        MaxX = X+SystematicErrorGraph.GetErrorXhigh(iPoint);
      }
      LatexLines.push_back(Form("\\draw[thick, xshift=%fmm, yshift=%fmm] (axis cs: %f,%f) rectangle (axis cs: %f,%f);",XShiftDistance,YShiftDistance,MinX,Y-SystematicErrorGraph.GetErrorYlow(iPoint),MaxX,Y+SystematicErrorGraph.GetErrorYhigh(iPoint)));
    } // iPoint
    return LatexLines;
  }

  vector<Marker> MarkerNodes(){
    vector<Marker> Nodes;
    for( Marker MarkerStyle:MarkerStyles ){
      for( int iPoint=0; iPoint<this->GetN(); iPoint++ ){
        if( XShiftDistance!=0 ) MarkerStyle.ShiftX = XShiftDistance;
        if( YShiftDistance!=0 ) MarkerStyle.ShiftY = YShiftDistance;
        MarkerStyle.SetAnchorPosition(this->GetPointX(iPoint),this->GetPointY(iPoint));
        Nodes.push_back(MarkerStyle);
      }
    }
    return Nodes;
  }
};
#endif