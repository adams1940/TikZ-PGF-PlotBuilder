#ifndef TEXFILE_HPP
#define TEXFILE_HPP

#include "PgfCanvas.h"
#include "Graph.h"
#include "Axis.h"
#include "Node.h"
#include "Marker.h"
#include "ErrorBar.h"

class TexFile{
private:
  int FigureCounter = 0;

public:
  TString FileName;
  ofstream File;

  TexFile(TString Name):FileName(Name){
    gSystem->Exec(Form("mkdir -p Output/%s",Name.Data()));
    File.open(Form("Output/%s/%s.tex",Name.Data(),Name.Data()));
    File<<"\\batchmode\n";
    File<<"\\documentclass[class=article,10pt,border=1pt]{standalone}\n";
    File<<"\\usepackage[utf8]{inputenc}\n";
    File<<"\\usepackage{bm}\n";
    File<<"\\usepackage{pgf, pgfplots, pgfplotstable}\n";
    File<<"\\pgfplotsset{width=12cm,height=10cm,compat=1.16}\n";
    File<<"\\usepgfmodule{oo}\n";
    File<<"\\usepgflibrary{shapes}\n";
    File<<"\\usepackage{catchfile}\n";
    File<<"\\usepackage{tikz}\n";
    File<<"\\usetikzlibrary{patterns}\n";
    File<<"\\usetikzlibrary{shadows}\n";
    File<<"\\usepackage{amsmath}\n";
    File<<"\\usetikzlibrary{external}\n";
    File<<"\\tikzexternalize\n";
    File<<"\\usepackage{calc}\n";
    File<<"\n";
    File<<"\\begin{document}\n";
    File<<"\n";
  } // TexFile 
    
  virtual ~TexFile(){
    File<<"\\end{document}";
    File.close();
    gSystem->Exec(Form("cd Output/%s; pdflatex -shell-escape %s; cd ../../",FileName.Data(),FileName.Data()));
  } // ~TexFile

  void AddCanvas(PgfCanvas Canvas){
    File<<"%%%%%%%%%%%%%%%%%%%%%%%% figure "<<FigureCounter++<<" below %%%%%%%%%%%%%%%%%%%%%%%%\n";
    File<<"\\begin{tikzpicture}\n";
    for( int ColumnY=0; ColumnY<Canvas.NumDivisionsY; ColumnY++ ){
      Axis YAxis = Canvas.YAxes[ColumnY];
      for( int ColumnX=0; ColumnX<Canvas.NumDivisionsX; ColumnX++ ){
        for( TString Line:Canvas.LatexLines() ) AddLine(Line);
        if( Canvas.LatexLines().size()>0 ) continue;
        Axis XAxis = Canvas.XAxes[ColumnX];
        AddPictureLine("\\begin{axis}[");

        AddAxisOption(Form("width=%fmm",Canvas.Width));
        AddAxisOption(Form("height=%fmm",Canvas.Height));

        AddAxisOption(Form("xmin=%f",XAxis.Min));
        AddAxisOption(Form("xmax=%f",XAxis.Max));
        AddAxisOption(Form("ymin=%f",YAxis.Min));
        AddAxisOption(Form("ymax=%f",YAxis.Max));
        AddAxisOption(Form("x label style={at={(1,%f)},anchor=north east}",Canvas.XLabelOffsetY));
        AddAxisOption(Form("y label style={at={(%f,1)},anchor=north east}",Canvas.YLabelOffsetX));
        if( XAxis.IsLog ) AddAxisOption("xmode=log");
        if( YAxis.IsLog ) AddAxisOption("ymode=log");

        AddAxisOption(Form("name=%s",CanvasName(ColumnX,ColumnY).Data()));
        if( !(ColumnX==0 && ColumnY==0) ){
          if( ColumnX==0 ){
            AddAxisOption(Form("at=(%s.south)",CanvasName(ColumnX,ColumnY-1).Data()));
            AddAxisOption("anchor=north");
          }
          else{
            AddAxisOption(Form("at=(%s.east)",CanvasName(ColumnX-1,ColumnY).Data()));
            AddAxisOption("anchor=west");
          }
        }

        if( ColumnY<Canvas.NumDivisionsY-1 ) AddAxisOption("xticklabels={}");
        else AddAxisOption(Form("xlabel={%s\\(%s\\)}",XAxis.TitleSize.Data(),XAxis.Title.Data()));
        if( ColumnX>0 ) AddAxisOption("yticklabels={}");
        else AddAxisOption(Form("ylabel={%s\\(%s\\)}",YAxis.TitleSize.Data(),YAxis.Title.Data()));

        if( XAxis.CustomMajorTicks.size()>0 ){
          TString CustomMajorTicksString = "{";
          { // Put this method in a custom vector class?
            for( int i=0; i<XAxis.CustomMajorTicks.size()-1; i++ ) CustomMajorTicksString.Append(Form("%f, ",XAxis.CustomMajorTicks[i]));
            CustomMajorTicksString.Append(Form("%f}",XAxis.CustomMajorTicks[XAxis.CustomMajorTicks.size()-1]));
          }
          AddAxisOption(Form("xtick=%s",CustomMajorTicksString.Data()));
        }
        if( YAxis.CustomMajorTicks.size()>0 ){
          TString CustomMajorTicksString = "{";
          { // Put this method in a custom vector class?
            for( int i=0; i<YAxis.CustomMajorTicks.size()-1; i++ ) CustomMajorTicksString.Append(Form("%f, ",YAxis.CustomMajorTicks[i]));
            CustomMajorTicksString.Append(Form("%f}",YAxis.CustomMajorTicks[YAxis.CustomMajorTicks.size()-1]));
          }
          AddAxisOption(Form("ytick=%s",CustomMajorTicksString.Data()));
        }
        if( XAxis.CustomMinorTicks.size()>0 ){
          TString CustomMinorTicksString = "{";
          { // Put this method in a custom vector class?
            for( int i=0; i<XAxis.CustomMinorTicks.size()-1; i++ ) CustomMinorTicksString.Append(Form("%f, ",XAxis.CustomMinorTicks[i]));
            CustomMinorTicksString.Append(Form("%f}",XAxis.CustomMinorTicks[XAxis.CustomMinorTicks.size()-1]));
          }
          AddAxisOption(Form("minor xtick=%s",CustomMinorTicksString.Data()));
        }
        AddAxisOption(Form("minor x tick num = %i",XAxis.NumMinorTicks));
        if( YAxis.CustomMinorTicks.size()>0 ){
          TString CustomMinorTicksString = "{";
          { // Put this method in a custom vector class?
            for( int i=0; i<YAxis.CustomMinorTicks.size()-1; i++ ) CustomMinorTicksString.Append(Form("%f, ",YAxis.CustomMinorTicks[i]));
            CustomMinorTicksString.Append(Form("%f}",YAxis.CustomMinorTicks[YAxis.CustomMinorTicks.size()-1]));
          }
          AddAxisOption(Form("minor ytick=%s",CustomMinorTicksString.Data()));
        }
        AddAxisOption(Form("minor y tick num = %i",YAxis.NumMinorTicks));

        AddPictureLine("]");

        if( Canvas.DrawZeroLinesVector[ColumnX][ColumnY] ) AddPictureLine(Form("\\addplot[color=gray, forget plot, /tikz/densely dotted, ] coordinates{(%f,0)(%f,0)};",Canvas.XAxes[ColumnX].Min,Canvas.XAxes[ColumnX].Max));

        for( Node * node:Canvas.Nodes[ColumnX][ColumnY] ) AddPictureLine(node->LatexLine());

        for( TString Node:Canvas.AdditionalNodes[ColumnX][ColumnY] ) AddPictureLine(Node.Data());

        for( Graph gr:Canvas.Graphs[ColumnX][ColumnY] ){
          if( gr.DrawLines ){
            for( int iPoint=0; iPoint<gr.GetN()-1; iPoint++ ){
              AddPictureLine(Form("\\draw[line width = %fmm, %s](%f,%f)--(%f,%f);",gr.LineWidth,gr.LineColor.Data(),gr.GetPointX(iPoint),gr.GetPointY(iPoint),gr.GetPointX(iPoint+1),gr.GetPointY(iPoint+1)));
            }
          }
          if( gr.OnlyDrawLines ) continue;
          AddPictureLine(Form("\\addplot[shift={(%fmm,%fmm)}, scatter, only marks, forget plot, no markers, error bars/.cd, error mark = none, error bar style = {line width=%fmm,solid}, x dir = %s, x explicit, y dir = %s, y explicit]",gr.XShiftDistance,gr.YShiftDistance,gr.ErrorBarStyle.Width,gr.DrawXError?"both":"none",gr.DrawYError?"both":"none"));
          AddPictureLine("\t table[x index = 0, x error minus index = 1, x error plus index = 2, y index = 3, y error minus index = 4, y error plus index = 5]{");
          for( int iPoint=0; iPoint<gr.GetN(); iPoint++ ){
            AddPictureLine(Form("\t\t%f %f %f %f %f %f",gr.GetPointX(iPoint),gr.GetErrorXlow(iPoint),gr.GetErrorXhigh(iPoint), gr.GetPointY(iPoint),gr.GetErrorYlow(iPoint),gr.GetErrorYhigh(iPoint)));
          }
          AddPictureLine("\t };");

          for( TString LatexLine:gr.SystematicErrorBoxLatexLines(&XAxis) ) AddPictureLine(LatexLine);
          for( Marker grMark:gr.MarkerNodes() ) AddPictureLine(grMark.LatexLine());
        }

        AddPictureLine("\\end{axis}");
      } // ColumnX
    } // ColumnY
    File<<"\\end{tikzpicture}\n\n";
  } // AddCanvas

  void AddLine(TString Line){
    File<<Line.Data()<<"\n";
  }
  void AddPictureLine(TString Line){
    File<<"\t"<<Line.Data()<<"\n";
  }
  void AddAxisOption(TString Option){
    File<<"\t\t"<<Option.Data()<<",\n";
  }
  TString CanvasName(int ColumnX, int ColumnY){
    return Form("X%iY%i",ColumnX,ColumnY);
  }
};
#endif