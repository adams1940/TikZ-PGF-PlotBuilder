#ifndef PGFCANVAS_HPP
#define PGFCANVAS_HPP

#include "Graph.h"
#include "Axis.h"
#include "Node.h"
#include "Pad.h"

class PgfCanvas{
private:

public:
  int NumDivisionsX, NumDivisionsY;
  int ActiveColX, ActiveRowY;
  double Width, Height; // mm
  vector<Axis> XAxes, YAxes;
  vector<Graph> Graphs[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]
  vector<TString> AdditionalNodes[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]
  vector<Node*> Nodes[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]
  vector<vector<bool>> DrawZeroLinesVector;
  double XLabelOffsetY, YLabelOffsetX;
  vector<Pad> Pads[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]

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

  void cd(int X = 0, int Y = 0, int I = 0){
    if( X>=NumDivisionsX || Y>=NumDivisionsY ){
      cout<<"Error: This pad doesn't exist!"<<endl;
      gApplication->Terminate();
    }
    ActiveColX = X;
    ActiveRowY = Y;
  }

  void AddPad(Pad &pad){ Pads[ActiveColX][ActiveRowY].push_back(pad); }

  Axis& ActiveXAxis(){ return XAxes[ActiveColX]; }
  Axis& ActiveYAxis(){ return YAxes[ActiveRowY]; }

  void AddGraph(Graph gr){
    Graphs[ActiveColX][ActiveRowY].push_back(gr);
  }

  void SetXTitle(TString Title){
    XAxes[ActiveColX].Title = Title;
  }

  void SetYTitle(TString Title){
    YAxes[ActiveRowY].Title = Title;
  }

  void SetXYTitles(TString XTitle, TString YTitle){
    for( Axis &axis:XAxes ) axis.Title = XTitle;
    for( Axis &axis:YAxes ) axis.Title = YTitle;
  }

  void SetXRange(double Min, double Max){
    XAxes[ActiveColX].Min = Min;
    XAxes[ActiveColX].Max = Max;
  }

  void SetYRange(double Min, double Max){
    YAxes[ActiveRowY].Min = Min;
    YAxes[ActiveRowY].Max = Max;
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
    DrawZeroLinesVector[ActiveColX][ActiveRowY] = true;
  }

  void AddNode(TString Text){
    AdditionalNodes[ActiveColX][ActiveRowY].push_back(Text);
  }

  void AddNode(Node * node){
    Nodes[ActiveColX][ActiveRowY].push_back(node);
  }

  TString CanvasName(int ColX, int RowY, int iPad){ return Form("ColX%i_RowY%i_Pad%i",ColX,RowY,iPad); }

  TString AxisOption(TString Text ){
    return Form("\t\t%s,",Text.Data());
  }
  TString PictureLine(TString Text ){
    return Form("\t%s",Text.Data());
  }

  vector<TString> LatexLines(){
    vector<TString> Lines;
    for( int ColX = 0; ColX<NumDivisionsX; ColX++ ){
      for( int RowY = 0; RowY<NumDivisionsY; RowY++ ){
        for( int iPad = 0; iPad<Pads[ColX][RowY].size(); iPad++ ){
          Pad pad = Pads[ColX][RowY][iPad];

          Lines.push_back(PictureLine("\\begin{axis}["));

          Lines.push_back(AxisOption(Form("\t\twidth=%fmm",pad.Width)));
          Lines.push_back(AxisOption(Form("\t\theight=%fmm",pad.Height)));
          Lines.push_back(AxisOption(Form("\t\txmin=%f",pad.XAxis.Min)));
          Lines.push_back(AxisOption(Form("\t\txmax=%f",pad.XAxis.Max)));
          Lines.push_back(AxisOption(Form("\t\tymin=%f",pad.YAxis.Min)));
          Lines.push_back(AxisOption(Form("\t\tymax=%f",pad.YAxis.Max)));
          Lines.push_back(AxisOption(Form("\t\tx label style={at={(1,%f)},anchor=north east}",pad.XLabelOffsetY)));
          Lines.push_back(AxisOption(Form("\t\ty label style={at={(%f,1)},anchor=north east}",pad.YLabelOffsetX)));
          if( pad.XAxis.IsLog ) Lines.push_back(AxisOption("\t\txmode=log"));
          if( pad.YAxis.IsLog ) Lines.push_back(AxisOption("\t\tymode=log"));

          Lines.push_back(AxisOption(Form("\t\tname=%s",CanvasName(ColX,RowY,iPad).Data())));
          if( !(ColX==0 && RowY==0) && iPad==0 ){
            if( ColX==0 ){
              Lines.push_back(AxisOption(Form("\t\tat=(%s.south)",CanvasName(ColX,RowY-1,0).Data())));
              Lines.push_back(AxisOption("\t\tanchor=north"));
            }
            else{
              Lines.push_back(AxisOption(Form("\t\tat=(%s.east)",CanvasName(ColX-1,RowY,0).Data())));
              Lines.push_back(AxisOption("\t\tanchor=west"));
            }
          }

          if( RowY<NumDivisionsY-1 ) Lines.push_back(AxisOption("\t\txticklabels={}"));
          else Lines.push_back(AxisOption(Form("\t\txlabel={%s\\(%s\\)}",pad.XAxis.TitleSize.Data(),pad.XAxis.Title.Data())));
          if( ColX>0 ) Lines.push_back(AxisOption("\t\tyticklabels={}"));
          else Lines.push_back(AxisOption(Form("\t\tylabel={%s\\(%s\\)}",pad.YAxis.TitleSize.Data(),pad.YAxis.Title.Data())));

          if( pad.XAxis.CustomMajorTicks.size()>0 ){
            TString CustomMajorTicksString = "{";
            { // Put this method in a custom vector class?
              for( int i=0; i<pad.XAxis.CustomMajorTicks.size()-1; i++ ) CustomMajorTicksString.Append(Form("%f, ",pad.XAxis.CustomMajorTicks[i]));
              CustomMajorTicksString.Append(Form("%f}",pad.XAxis.CustomMajorTicks[pad.XAxis.CustomMajorTicks.size()-1]));
            }
            Lines.push_back(AxisOption(Form("\t\txtick=%s",CustomMajorTicksString.Data())));
          }
          if( pad.YAxis.CustomMajorTicks.size()>0 ){
            TString CustomMajorTicksString = "{";
            { // Put this method in a custom vector class?
              for( int i=0; i<pad.YAxis.CustomMajorTicks.size()-1; i++ ) CustomMajorTicksString.Append(Form("%f, ",pad.YAxis.CustomMajorTicks[i]));
              CustomMajorTicksString.Append(Form("%f}",pad.YAxis.CustomMajorTicks[pad.YAxis.CustomMajorTicks.size()-1]));
            }
            Lines.push_back(AxisOption(Form("\t\tytick=%s",CustomMajorTicksString.Data())));
          }
          if( pad.XAxis.CustomMinorTicks.size()>0 ){
            TString CustomMinorTicksString = "{";
            { // Put this method in a custom vector class?
              for( int i=0; i<pad.XAxis.CustomMinorTicks.size()-1; i++ ) CustomMinorTicksString.Append(Form("%f, ",pad.XAxis.CustomMinorTicks[i]));
              CustomMinorTicksString.Append(Form("%f}",pad.XAxis.CustomMinorTicks[pad.XAxis.CustomMinorTicks.size()-1]));
            }
            Lines.push_back(AxisOption(Form("\t\tminor xtick=%s",CustomMinorTicksString.Data())));
          }
          Lines.push_back(AxisOption(Form("\t\tminor x tick num = %i",pad.XAxis.NumMinorTicks)));
          if( pad.YAxis.CustomMinorTicks.size()>0 ){
            TString CustomMinorTicksString = "{";
            { // Put this method in a custom vector class?
              for( int i=0; i<pad.YAxis.CustomMinorTicks.size()-1; i++ ) CustomMinorTicksString.Append(Form("%f, ",pad.YAxis.CustomMinorTicks[i]));
              CustomMinorTicksString.Append(Form("%f}",pad.YAxis.CustomMinorTicks[pad.YAxis.CustomMinorTicks.size()-1]));
            }
            Lines.push_back(AxisOption(Form("\t\tminor ytick=%s",CustomMinorTicksString.Data())));
          }
          Lines.push_back(AxisOption(Form("\t\tminor y tick num = %i",pad.YAxis.NumMinorTicks)));

          Lines.push_back("\t]");

          if( pad.DrawZeroLine ) Lines.push_back(PictureLine(Form("\\addplot[color=gray, forget plot, /tikz/densely dotted, ] coordinates{(%f,0)(%f,0)};",pad.XAxis.Min,pad.XAxis.Max)));

          for( Node * node:pad.Nodes ) Lines.push_back(PictureLine(node->LatexLine()));

          for( Graph gr:pad.Graphs ){
            if( gr.DrawLines ){
              for( int iPoint=0; iPoint<gr.GetN()-1; iPoint++ ){
                Lines.push_back(PictureLine(Form("\\draw[line width = %fmm, %s](%f,%f)--(%f,%f);",gr.LineWidth,gr.LineColor.Data(),gr.GetPointX(iPoint),gr.GetPointY(iPoint),gr.GetPointX(iPoint+1),gr.GetPointY(iPoint+1))));
              }
            }
            if( gr.OnlyDrawLines ) continue;
            Lines.push_back(PictureLine(Form("\\addplot[shift={(%fmm,%fmm)}, scatter, only marks, forget plot, no markers, error bars/.cd, error mark = none, error bar style = {line width=%fmm,solid}, x dir = %s, x explicit, y dir = %s, y explicit]",gr.XShiftDistance,gr.YShiftDistance,gr.ErrorBarStyle.Width,gr.DrawXError?"both":"none",gr.DrawYError?"both":"none")));
            Lines.push_back(PictureLine("\ttable[x index = 0, x error minus index = 1, x error plus index = 2, y index = 3, y error minus index = 4, y error plus index = 5]{"));
            for( int iPoint=0; iPoint<gr.GetN(); iPoint++ ){
              Lines.push_back(PictureLine(Form("\t\t%f %f %f %f %f %f",gr.GetPointX(iPoint),gr.GetErrorXlow(iPoint),gr.GetErrorXhigh(iPoint), gr.GetPointY(iPoint),gr.GetErrorYlow(iPoint),gr.GetErrorYhigh(iPoint))));
            }
            Lines.push_back(PictureLine("\t};"));

            for( TString LatexLine:gr.SystematicErrorBoxLatexLines(&pad.XAxis) ) Lines.push_back(PictureLine(LatexLine));
            for( Marker grMark:gr.MarkerNodes() ) Lines.push_back(PictureLine(grMark.LatexLine()));

          } // for graphs

          Lines.push_back("\t\\end{axis}");
        } // iPad
      } // RowY
    } // ColY

    return Lines;
  }
};
#endif