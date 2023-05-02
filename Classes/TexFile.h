#ifndef TEXFILE_HPP
#define TEXFILE_HPP

#include "Canvas.h"
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

  void AddCanvas(Canvas Canvas){
    File<<"%%%%%%%%%%%%%%%%%%%%%%%% figure "<<FigureCounter++<<" below %%%%%%%%%%%%%%%%%%%%%%%%\n";
    File<<"\\begin{tikzpicture}\n";
    for( TString Line:Canvas.LatexLines() ) AddLine(Line);
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