double EnergyShift = 0.03;

pair<double, double> OffsetsOnLogScale(double Value, double RelativeOffset){
  pair<double, double> Offsets(RelativeOffset*exp(2.*log(Value)-log(RelativeOffset*Value+Value)),Value*RelativeOffset);
  return Offsets;
} // OffsetsOnLogScale

struct Axis{
    double Min, Max;
    int NumMinorTicks;
    vector<double> CustomMajorTicks, CustomMinorTicks;
    TString Title, TitleSize;
    bool IsLog;
    Axis(){
        NumMinorTicks = 4;
        Min = 0;
        Max = 1;
        TitleSize = "\\large";
        IsLog = false;
    }
    virtual ~Axis(){};
}; // Axis

struct Node{
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

struct Marker : public Node{
  public:
    double Size, OutlineWidthScale;
    TString Shape;
    double StarRatio;

    Marker(double x = 0, double y = 0){
        AnchorX = x;
        AnchorY = y;
        StarRatio = 2.618034;
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
      if( Shape=="star" ) Options.push_back(Form("star point ratio=%f",StarRatio));
      return Node::NodeText();
    }
};

struct Box : public Node{
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

    TString NodeText(){
      Options.push_back(Form("minimum width=%fmm",Width));
      Options.push_back(Form("minimum height=%fmm",Height));
      return Node::NodeText();
    }
};

struct TextBox : public Node{
  TextBox(double x = 0, double y = 0){
    AnchorX = x;
    AnchorY = y;
    IncludeDraw = false;
  }
  void DrawBorder(bool yn){
    IncludeDraw = yn;
  }
};

struct ErrorBar{
    double Width;
    TString Color;

    ErrorBar(){
        Width = 0.3; // mm
        Color = "black";
    }
    virtual ~ErrorBar(){}
}; // ErrorBar

class Graph : public TGraphAsymmErrors{
private:

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

    Graph(){
        DrawXError = true;
        DrawYError = true;
        DrawLines = false;
        OnlyDrawLines = false;
        LineWidth = 0.2; // mm
        LineColor = "blue";
        XShiftDistance = 0;
        YShiftDistance = 0;
    }    
    virtual ~Graph(){}

    void AddMarkerStyle(Marker MarkerStyle){
      MarkerStyles.push_back(MarkerStyle);
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
}; // Graph

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

    PgfCanvas(int NX = 1, int NY = 1):NumDivisionsX(NX),NumDivisionsY(NY){
        Width = 100/(double)NX;
        Height = 0.9*Width;
        XAxes.resize(NX);
        YAxes.resize(NY);
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

    void AddZoomInset(PgfCanvas &can){
    }
}; // PgfCanvas

class TexFile{
private:
    int FigureCounter = 0;

public:
    TString FileName;
    ofstream File;

    TexFile(TString Name):FileName(Name){
        File.open(Form("Output/%s/%s.tex",Name.Data(),Name.Data()));
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
        File<<"\\definecolor{LambdaFillColor}{RGB}{51,102,255}\n";
        File<<"\\definecolor{LamBarFillColor}{RGB}{204,16,0}\n";
        File<<"\\definecolor{LambdaInnerFillColor}{RGB}{51,102,255}\n";
        File<<"\\definecolor{LamBarInnerFillColor}{RGB}{255,255,255}\n";
        File<<"\\newcommand{\\PerfectStarRadiusRatio}{2.618034}";
        File<<"\\newcommand{\\XLabelOffsetY}{-0.07}";
        File<<"\\newcommand{\\YLabelOffsetX}{-0.161}";
        File<<"\\newcommand{\\sNN}{\\sqrt{s_\\mathrm{NN}}}";
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
                Axis XAxis = Canvas.XAxes[ColumnX];
                AddPictureLine("\\begin{axis}[");

                AddAxisOption(Form("width=%fmm",Canvas.Width));
                AddAxisOption(Form("height=%fmm",Canvas.Height));

                AddAxisOption(Form("xmin=%f",XAxis.Min));
                AddAxisOption(Form("xmax=%f",XAxis.Max));
                AddAxisOption(Form("ymin=%f",YAxis.Min));
                AddAxisOption(Form("ymax=%f",YAxis.Max));
                AddAxisOption(Form("x label style={at={(1,\\XLabelOffsetY)},anchor=north east}"));
                AddAxisOption(Form("y label style={at={(\\YLabelOffsetX,1)},anchor=north east}"));
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

                for( Node * node:Canvas.Nodes[ColumnX][ColumnY] ) AddPictureLine(node->NodeText());

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

                    for( Marker grMark:gr.MarkerNodes() ) AddPictureLine(grMark.NodeText());

                }

                AddPictureLine("\\end{axis}");
            } // ColumnX
        } // ColumnY
        File<<"\\end{tikzpicture}\n\n";
    } // AddCanvas

    void AddPictureLine(TString Line){
        File<<"\t"<<Line.Data()<<"\n";
    }
    void AddAxisOption(TString Option){
        File<<"\t\t"<<Option.Data()<<",\n";
    }
    TString CanvasName(int ColumnX, int ColumnY){
        return Form("X%iY%i",ColumnX,ColumnY);
    }
}; // TexFile

namespace SplittingFigureTools{
  Marker BesIILambdaMarkerStyle, BesIILamBarMarkerStyle, BesIILamBarInnerMarkerStyle;
  Marker BesILambdaMarkerStyle, BesILamBarMarkerStyle;
  Marker AliceLambdaMarkerStyle, AliceLamBarMarkerStyle;
  double XShift;
  PgfCanvas * can;
  double BesISizeFactor;

  void SetMarkerStyles(){
    XShift = 1;
    BesISizeFactor = 0.7;
    BesIILambdaMarkerStyle.Shape = "star";
    BesIILambdaMarkerStyle.FillColor = "LambdaFillColor";
    BesIILamBarMarkerStyle.Shape = "star";
    BesIILamBarMarkerStyle.FillColor = "LamBarFillColor";
    BesIILamBarInnerMarkerStyle.Shape = "star";
    BesIILamBarInnerMarkerStyle.Size = BesIILamBarMarkerStyle.Size*0.25;
    BesIILamBarInnerMarkerStyle.OutlineWidthScale = 0;
    BesIILamBarInnerMarkerStyle.OutlineColor = "LamBarInnerFillColor";
    BesIILamBarInnerMarkerStyle.FillColor = "LamBarInnerFillColor";

    BesILambdaMarkerStyle = BesIILambdaMarkerStyle;
    BesILambdaMarkerStyle.FillColor = "{rgb:black,1;white,3}";
    BesILambdaMarkerStyle.Size*=BesISizeFactor;
    BesILamBarMarkerStyle = BesIILamBarMarkerStyle;
    BesILamBarMarkerStyle.FillColor = "white";
    BesILamBarMarkerStyle.Size*=BesISizeFactor;
    
    AliceLambdaMarkerStyle.Shape = "rectangle";
    AliceLambdaMarkerStyle.FillColor = "{rgb:black,1;white,3}";
    AliceLambdaMarkerStyle.Size = BesILambdaMarkerStyle.Size;
    AliceLamBarMarkerStyle.Shape = "rectangle";
    AliceLamBarMarkerStyle.FillColor = "white";
    AliceLamBarMarkerStyle.Size = BesILamBarMarkerStyle.Size;
  }

  void SetCanvas(PgfCanvas &c){
    can = &c;
  }

  void DrawData(Graph &StatGraph, Graph &SystGraph, bool IsLambda, bool IsNewResult){

    can->AddGraph(StatGraph);

    double SystBoxWidthFactor=0.4;
    for( int iPoint=0; iPoint<SystGraph.GetN(); iPoint++ ){
      can->AddNode(Form("\\draw[thick] (axis cs: %f,%f) rectangle (axis cs: %f,%f);",SystGraph.GetPointX(iPoint)-SystBoxWidthFactor*StatGraph.GetErrorX(0),SystGraph.GetPointY(iPoint)-SystGraph.GetErrorY(iPoint),SystGraph.GetPointX(iPoint)+SystBoxWidthFactor*StatGraph.GetErrorX(0),SystGraph.GetPointY(iPoint)+SystGraph.GetErrorY(iPoint)));
    }
  } // DrawData

  void DrawData(Graph &StatGraph, Graph &SystGraph){
    can->AddGraph(StatGraph);
    double SystBoxWidthFactor=0.4;
    for( int iPoint=0; iPoint<SystGraph.GetN(); iPoint++ ){
      can->AddNode(Form("\\draw[thick] (axis cs: %f,%f) rectangle (axis cs: %f,%f);",SystGraph.GetPointX(iPoint)-SystBoxWidthFactor*StatGraph.GetErrorX(0),SystGraph.GetPointY(iPoint)-SystGraph.GetErrorY(iPoint),SystGraph.GetPointX(iPoint)+SystBoxWidthFactor*StatGraph.GetErrorX(0),SystGraph.GetPointY(iPoint)+SystGraph.GetErrorY(iPoint)));
    }
  }

  void DrawBesILambda(Graph &StatGraph, Graph &SystGraph){
    StatGraph.AddMarkerStyle(BesILambdaMarkerStyle);
    StatGraph.XShiftDistance = -XShift;
    DrawData(StatGraph,SystGraph);
  }
  void DrawBesILamBar(Graph &StatGraph, Graph &SystGraph){
    StatGraph.AddMarkerStyle(BesILamBarMarkerStyle);
    StatGraph.XShiftDistance = XShift;
    DrawData(StatGraph,SystGraph);
  }
  void DrawBesIILambda(Graph &StatGraph, Graph &SystGraph){
    StatGraph.AddMarkerStyle(BesIILambdaMarkerStyle);
    StatGraph.XShiftDistance = -XShift;
    DrawData(StatGraph,SystGraph);
  }
  void DrawBesIILamBar(Graph &StatGraph, Graph &SystGraph){
    StatGraph.AddMarkerStyle(BesIILamBarMarkerStyle);
    StatGraph.AddMarkerStyle(BesIILamBarInnerMarkerStyle);
    StatGraph.XShiftDistance = XShift;
    DrawData(StatGraph,SystGraph);
  }
  void DrawAliceLambda(Graph &StatGraph, Graph &SystGraph){
    StatGraph.AddMarkerStyle(AliceLambdaMarkerStyle);
    StatGraph.XShiftDistance = -XShift;
    DrawData(StatGraph,SystGraph);
  }
  void DrawAliceLamBar(Graph &StatGraph, Graph &SystGraph){
    StatGraph.AddMarkerStyle(AliceLamBarMarkerStyle);
    StatGraph.XShiftDistance = XShift;
    DrawData(StatGraph,SystGraph);
  }

  void DrawInfoText(TString Energy, TString PtRapidityCentrality, double x, double y){
    TextBox * text = new TextBox(x,y);
    text->Text = Form("STAR Au+Au, $\\sNN=%s$~GeV\\\\%s\\\\$\\alpha_\\Lambda=0.732$",Energy.Data(),PtRapidityCentrality.Data());
    can->AddNode(text);
  }

  void DrawLambdaPointLegend(double x, double y){
    double XShift = 2.5, TitleYShift = 0, MarkerYShift = -2.5;
    Box * Background = new Box(x,y); 
    can->AddNode(Background);
    TextBox * LambdaTitle = new TextBox(x,y);
    LambdaTitle->Text = "$\\Lambda$";
    LambdaTitle->Shift(-XShift,TitleYShift);
    LambdaTitle->Anchor = "south";
    can->AddNode(LambdaTitle);
    TextBox * LamBarTitle = new TextBox(x,y);
    LamBarTitle->Text = "$\\bar{\\Lambda}$";
    LamBarTitle->Shift(XShift,TitleYShift);
    LamBarTitle->Anchor = "south";
    can->AddNode(LamBarTitle);
    Marker * LambdaLegendMarker = new Marker();
    *LambdaLegendMarker = BesIILambdaMarkerStyle;
    LambdaLegendMarker->SetAnchorPosition(x,y);
    LambdaLegendMarker->Shift(-XShift,MarkerYShift);
    can->AddNode(LambdaLegendMarker);
    Marker * LamBarLegendMarker = new Marker();
    *LamBarLegendMarker = BesIILamBarMarkerStyle;
    LamBarLegendMarker->SetAnchorPosition(x,y);
    LamBarLegendMarker->Shift(XShift,MarkerYShift);
    can->AddNode(LamBarLegendMarker);
    Marker * LamBarInnerLegendMarker = new Marker();
    *LamBarInnerLegendMarker = BesIILamBarInnerMarkerStyle;
    LamBarInnerLegendMarker->SetAnchorPosition(x,y);
    LamBarInnerLegendMarker->Shift(XShift,MarkerYShift);
    can->AddNode(LamBarInnerLegendMarker);
  }

  void SetStatAndSystGraphs(Graph &StatisticalErrorGraph, Graph &SystematicErrorGraph, vector<vector<double>> MeasuredPolarizationDataPoints, int IsLambda, bool SetLogX = true, bool OffsetX = true){
    const int NumDataPoints = (const int)MeasuredPolarizationDataPoints.size();
    double SystErrX = SetLogX?0.12:2; // this is the "width" of the systematic error box in energy (just for visibility)
    double StatErrX = 0.;
    for( int iDataPoint = 0; iDataPoint<NumDataPoints; iDataPoint++ ){
      double Energy = MeasuredPolarizationDataPoints[iDataPoint][0];
      if( OffsetX && IsLambda ) Energy -= OffsetsOnLogScale(Energy,EnergyShift).first;
      if( OffsetX && !IsLambda ) Energy += OffsetsOnLogScale(Energy,EnergyShift).second;
      double Polarization = MeasuredPolarizationDataPoints[iDataPoint][1];
      double StatErr = MeasuredPolarizationDataPoints[iDataPoint][2];
      double SystErrLow = MeasuredPolarizationDataPoints[iDataPoint][3];
      double SystErrHgh = MeasuredPolarizationDataPoints[iDataPoint][4];
      double LeftSystErr = (MeasuredPolarizationDataPoints[iDataPoint][3]==0. && MeasuredPolarizationDataPoints[iDataPoint][4]==0.)?0.:!SetLogX?SystErrX:OffsetsOnLogScale(Energy,SystErrX).first;
      double RghtSystErr = (MeasuredPolarizationDataPoints[iDataPoint][3]==0. && MeasuredPolarizationDataPoints[iDataPoint][4]==0.)?0.:!SetLogX?SystErrX:OffsetsOnLogScale(Energy,SystErrX).second;
      double StatXErr = 0.;
      StatisticalErrorGraph.SetPoint(iDataPoint,Energy,Polarization);
      StatisticalErrorGraph.SetPointError(iDataPoint,StatXErr,StatXErr,StatErr,StatErr);
      SystematicErrorGraph.SetPoint(iDataPoint,Energy,Polarization);
      SystematicErrorGraph.SetPointError(iDataPoint,LeftSystErr,RghtSystErr,SystErrLow,SystErrHgh);
    } // iDataPoint
  }
}; // SplittingFigureTools

struct Legend{
  TString ColumnLabels[10], RowLabels[10];
  TString Title;
  Graph Graphs[10][10];
  int NumColumns, NumRows;
  double ColumnShifts[10], RowShifts[10];

  Legend(){}
  virtual ~Legend(){}

  void SetColumnLabel(int iColumn, TString Label){
    ColumnLabels[iColumn] = Label;
  }
  void SetRowLabel(int iRow, TString Label){
    RowLabels[iRow] = Label;
  }
  void SetColumnShiftX(int iColumn, double shift){
    ColumnShifts[iColumn] = shift;
  }
  void SetRowShiftX(int iRow, double shift){
    RowShifts[iRow] = shift;
  }
  void AddEntry(int iColumn, int iRow, Graph &gr){
    Graphs[iColumn][iRow] = gr;
  }
}; // Legend

  Graph ConvertTh1ToGraph(const TH1 &Hist){
      int NumBins = Hist.GetNbinsX();
      double BinWidth = Hist.GetBinWidth(0);
      double XErrorOverXRange = BinWidth/(Hist.GetBinLowEdge(NumBins+1)-Hist.GetBinLowEdge(1));
      int PointCounter = 0;
      Graph gr;
      gr.SetName(Form("%s_gr",Hist.GetName()));
      for( int Bin=1; Bin<=NumBins; Bin++ ){
          double Val = Hist.GetBinContent(Bin), Err = Hist.GetBinError(Bin);
          if( Val==0 && Err==0 ) continue;
          gr.SetPoint(PointCounter,Hist.GetBinCenter(Bin),Val);
          gr.SetPointError(PointCounter,Hist.GetBinWidth(Bin)/2.,Hist.GetBinWidth(Bin)/2.,Err,Err);
          PointCounter++;
      } // Bin
      return gr;
  }

void Plotter(TString OutputFileCommitHash = "test"){
    TString LambdaCommitHash19GeVCentrality = "84380533efb06885108ea47a091187d38f1989fe";
    TString LamBarCommitHash19GeVCentrality = "a62202a143af0c296e98e4e8d54cee0f0d583150";
    TString LambdaCommitHash27GeVCentrality = "439a5050cd21fb44dec792bd671aca260296ee8e";
    TString LamBarCommitHash27GeVCentrality = "f522bd60f00be59fcf717b79e0e67f2c6b51618d";
    TString LambdaCommitHash19GeV = "044666513c03b24ef87a6361ff6394455d9acde8";
    TString LamBarCommitHash19GeV = "5cae15284111395d6a12676b1db189ad894d4e4c";
    TString LambdaCommitHash27GeV = "cc4f4f6e0dbd646f7013dd693694d11c9c326238";
    TString LamBarCommitHash27GeV = "079840ff12164ccccfc39ab4bb72ef3fbf2f8f88";

    TString PolarizationTitle = "P_{\\mathrm{H}}~(\\%)";

    TFile LambdaFile19GeVCentrality(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/19GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LambdaCommitHash19GeVCentrality.Data(),LambdaCommitHash19GeVCentrality.Data()));
    TFile LamBarFile19GeVCentrality(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/19GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LamBarCommitHash19GeVCentrality.Data(),LamBarCommitHash19GeVCentrality.Data()));
    TFile LambdaFile27GeVCentrality(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/27GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LambdaCommitHash27GeVCentrality.Data(),LambdaCommitHash27GeVCentrality.Data()));
    TFile LamBarFile27GeVCentrality(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/27GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LamBarCommitHash27GeVCentrality.Data(),LamBarCommitHash27GeVCentrality.Data()));
    TFile LambdaFile19GeV(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/19GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LambdaCommitHash19GeV.Data(),LambdaCommitHash19GeV.Data()));
    TFile LamBarFile19GeV(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/19GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LamBarCommitHash19GeV.Data(),LamBarCommitHash19GeV.Data()));
    TFile LambdaFile27GeV(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/27GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LambdaCommitHash27GeV.Data(),LambdaCommitHash27GeV.Data()));
    TFile LamBarFile27GeV(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/27GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LamBarCommitHash27GeV.Data(),LamBarCommitHash27GeV.Data()));

    Graph LambdaStatGraph19GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LambdaFile19GeVCentrality.Get("Centrality_Polarization_StandardMethod")) );
    Graph LamBarStatGraph19GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LamBarFile19GeVCentrality.Get("Centrality_Polarization_StandardMethod")) );
    Graph LambdaStatGraph27GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LambdaFile27GeVCentrality.Get("Centrality_Polarization_StandardMethod")) );
    Graph LamBarStatGraph27GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LamBarFile27GeVCentrality.Get("Centrality_Polarization_StandardMethod")) );
    Graph LambdaSystGraph19GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LambdaFile19GeVCentrality.Get("Centrality_Polarization_SystematicUncertainty")) );
    Graph LamBarSystGraph19GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LamBarFile19GeVCentrality.Get("Centrality_Polarization_SystematicUncertainty")) );
    Graph LambdaSystGraph27GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LambdaFile27GeVCentrality.Get("Centrality_Polarization_SystematicUncertainty")) );
    Graph LamBarSystGraph27GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LamBarFile27GeVCentrality.Get("Centrality_Polarization_SystematicUncertainty")) );
    Graph LambdaStatGraph19GeVPt = ConvertTh1ToGraph( *((TH1D*)LambdaFile19GeV.Get("Pt_Polarization_StandardMethod")) );
    Graph LamBarStatGraph19GeVPt = ConvertTh1ToGraph( *((TH1D*)LamBarFile19GeV.Get("Pt_Polarization_StandardMethod")) );
    Graph LambdaStatGraph27GeVPt = ConvertTh1ToGraph( *((TH1D*)LambdaFile27GeV.Get("Pt_Polarization_StandardMethod")) );
    Graph LamBarStatGraph27GeVPt = ConvertTh1ToGraph( *((TH1D*)LamBarFile27GeV.Get("Pt_Polarization_StandardMethod")) );
    Graph LambdaSystGraph19GeVPt = ConvertTh1ToGraph( *((TH1D*)LambdaFile19GeV.Get("Pt_Polarization_SystematicUncertainty")) );
    Graph LamBarSystGraph19GeVPt = ConvertTh1ToGraph( *((TH1D*)LamBarFile19GeV.Get("Pt_Polarization_SystematicUncertainty")) );
    Graph LambdaSystGraph27GeVPt = ConvertTh1ToGraph( *((TH1D*)LambdaFile27GeV.Get("Pt_Polarization_SystematicUncertainty")) );
    Graph LamBarSystGraph27GeVPt = ConvertTh1ToGraph( *((TH1D*)LamBarFile27GeV.Get("Pt_Polarization_SystematicUncertainty")) );
    Graph LambdaStatGraph19GeVRapidity = ConvertTh1ToGraph( *((TH1D*)LambdaFile19GeV.Get("Rapidity_Polarization_StandardMethod")) );
    Graph LamBarStatGraph19GeVRapidity = ConvertTh1ToGraph( *((TH1D*)LamBarFile19GeV.Get("Rapidity_Polarization_StandardMethod")) );
    Graph LambdaStatGraph27GeVRapidity = ConvertTh1ToGraph( *((TH1D*)LambdaFile27GeV.Get("ComRapidity_Polarization_StandardMethod")) );
    Graph LamBarStatGraph27GeVRapidity = ConvertTh1ToGraph( *((TH1D*)LamBarFile27GeV.Get("ComRapidity_Polarization_StandardMethod")) );
    Graph LambdaSystGraph19GeVRapidity = ConvertTh1ToGraph( *((TH1D*)LambdaFile19GeV.Get("Rapidity_Polarization_SystematicUncertainty")) );
    Graph LamBarSystGraph19GeVRapidity = ConvertTh1ToGraph( *((TH1D*)LamBarFile19GeV.Get("Rapidity_Polarization_SystematicUncertainty")) );
    Graph LambdaSystGraph27GeVRapidity = ConvertTh1ToGraph( *((TH1D*)LambdaFile27GeV.Get("ComRapidity_Polarization_SystematicUncertainty")) );
    Graph LamBarSystGraph27GeVRapidity = ConvertTh1ToGraph( *((TH1D*)LamBarFile27GeV.Get("ComRapidity_Polarization_SystematicUncertainty")) );

  double AlphaChangeFactor = 0.642/0.732;  // old/new
  vector<vector<double>> NineteenGeVLambdaPolarizationGraphPoints = {
    {
      19.6+OffsetsOnLogScale(19.6,1.*EnergyShift).second, // 044666513c03b24ef87a6361ff6394455d9acde8
      0.9152,
      0.0503,
      0.0172,
      0.0172,
    },
    {
      27+OffsetsOnLogScale(27,1.*EnergyShift).second,
      0.7168,
      0.0553,
      0.0143,
      0.0143,
    },
  };
  vector<double> NineteenGeVEnergies; for( int i=0; i<NineteenGeVLambdaPolarizationGraphPoints.size(); i++ ) NineteenGeVEnergies.push_back(NineteenGeVLambdaPolarizationGraphPoints[i][0]);
  vector<vector<double>> NineteenGeVLamBarPolarizationGraphPoints = {
    {
      19.6+OffsetsOnLogScale(19.6,1.*EnergyShift).second, // 5cae15284111395d6a12676b1db189ad894d4e4c
      0.8976,
      0.1170,
      0.0171,
      0.0171,
    },
    {
      27+OffsetsOnLogScale(27,1.*EnergyShift).second,
      0.8257,
      0.1046,
      0.0163,
      0.0163,
    },
  };
  vector<vector<double>> StarLambdaPolarizationGraphPoints = {
    {3.,    4.90823,                  0.81389,                  0.15485,                  0.15485                   },  // from joseph@joseph-Latitude-5590:~/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/3GeV/Output/Histograms/1630bfc58b466c639cf8e643b4e9f5aafc6a11d5.PolarizationHistograms.root
    {7.7,   AlphaChangeFactor*2.039,  AlphaChangeFactor*0.628,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {11.5,  AlphaChangeFactor*1.344,  AlphaChangeFactor*0.396,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {14.5,  AlphaChangeFactor*1.321,  AlphaChangeFactor*0.482,  AlphaChangeFactor*0.3,    AlphaChangeFactor*0.0   },
    {19.6-OffsetsOnLogScale(19.6,1.*EnergyShift).first,  AlphaChangeFactor*0.950,  AlphaChangeFactor*0.305,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {27.0-OffsetsOnLogScale(27,1.*EnergyShift).first,  AlphaChangeFactor*1.047,  AlphaChangeFactor*0.282,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {39.0,  AlphaChangeFactor*0.506,  AlphaChangeFactor*0.424,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {62.4,  AlphaChangeFactor*1.334,  AlphaChangeFactor*1.167,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {200.0, AlphaChangeFactor*0.277,  AlphaChangeFactor*0.040,  AlphaChangeFactor*0.049,  AlphaChangeFactor*0.039 },
  };
  vector<vector<double>> StarLamBarPolarizationGraphPoints = {
    {7.7,   AlphaChangeFactor*8.669,  AlphaChangeFactor*3.569,  AlphaChangeFactor*1.00,   AlphaChangeFactor*0.0   },
    {11.5,  AlphaChangeFactor*1.802,  AlphaChangeFactor*1.261,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.0   },
    {14.5,  AlphaChangeFactor*2.276,  AlphaChangeFactor*1.210,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.4   },
    {19.6-OffsetsOnLogScale(19.6,1.*EnergyShift).first,  AlphaChangeFactor*1.515,  AlphaChangeFactor*0.610,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.0   },
    {27.0-OffsetsOnLogScale(27,1.*EnergyShift).first,  AlphaChangeFactor*1.245,  AlphaChangeFactor*0.471,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.0   },
    {39.0,  AlphaChangeFactor*0.938,  AlphaChangeFactor*0.615,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.0   },
    {62.4,  AlphaChangeFactor*1.712,  AlphaChangeFactor*1.592,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.0   },
    {200.0, AlphaChangeFactor*0.240,  AlphaChangeFactor*0.045,  AlphaChangeFactor*0.045,  AlphaChangeFactor*0.061 },
  };

  vector<vector<double>> AlicLambdaPolarizationGraphPoints = {
    {2760,  AlphaChangeFactor*-0.0785, AlphaChangeFactor*0.102,  AlphaChangeFactor*0.06,   AlphaChangeFactor*0.06  },
    {5020,  AlphaChangeFactor*0.131, AlphaChangeFactor*0.11,   AlphaChangeFactor*0.06,   AlphaChangeFactor*0.06  },
  };
  vector<double> AlicEnergies; for( int i=0; i<AlicLambdaPolarizationGraphPoints.size(); i++ ) AlicEnergies.push_back(AlicLambdaPolarizationGraphPoints[i][0]);
  vector<vector<double>> AlicLamBarPolarizationGraphPoints = {
    {2760,  AlphaChangeFactor*0.052, AlphaChangeFactor*0.1,    AlphaChangeFactor*0.06,   AlphaChangeFactor*0.06  },
    {5020,  AlphaChangeFactor*-0.14,   AlphaChangeFactor*0.12,   AlphaChangeFactor*0.06,   AlphaChangeFactor*0.06  },
  };

    gSystem->Exec(Form("mkdir -p Output/%s",OutputFileCommitHash.Data()));
    TexFile MyTexFile(OutputFileCommitHash);

    PgfCanvas CentralityCanvas(1,2);
    CentralityCanvas.SetXYTitles("\\mathrm{Centrality}~(\\%)",PolarizationTitle);
    CentralityCanvas.SetXRanges(-5,85);
    CentralityCanvas.SetYRanges(-0.65,3.65);
    CentralityCanvas.DrawZeroLines();
    CentralityCanvas.cd(0,0);
    // SplittingFigure CentralityFigure;
    SplittingFigureTools::SetCanvas(CentralityCanvas);
    SplittingFigureTools::SetMarkerStyles();
    SplittingFigureTools::DrawBesIILambda(LambdaStatGraph19GeVCentrality,LambdaSystGraph19GeVCentrality);
    SplittingFigureTools::DrawBesIILamBar(LamBarStatGraph19GeVCentrality,LamBarSystGraph19GeVCentrality);
    SplittingFigureTools::DrawInfoText("19.6","$p_{\\mathrm{T}}>0.5$~GeV/$c$, $|y|<1$",22,2.8);
    CentralityCanvas.cd(0,1);
    SplittingFigureTools::DrawBesIILambda(LambdaStatGraph27GeVCentrality,LambdaSystGraph27GeVCentrality);
    SplittingFigureTools::DrawBesIILamBar(LamBarStatGraph27GeVCentrality,LamBarSystGraph27GeVCentrality);
    SplittingFigureTools::DrawLambdaPointLegend(15,1.5);
    SplittingFigureTools::DrawInfoText("27","$p_{\\mathrm{T}}>0.5$~GeV/$c$, $|y|<1$",22,2.8);
    MyTexFile.AddCanvas(CentralityCanvas);

    PgfCanvas PtCanvas(1,2);
    PtCanvas.SetXYTitles("p_{\\mathrm{T}}~(\\mathrm{GeV}/c)",PolarizationTitle);
    PtCanvas.SetXRanges(0.3,3.7);
    PtCanvas.SetYRanges(-0.22,1.82);
    PtCanvas.DrawZeroLines();
    PtCanvas.cd(0,0);
    SplittingFigureTools::SetCanvas(PtCanvas);
    SplittingFigureTools::DrawBesIILambda(LambdaStatGraph19GeVPt,LambdaSystGraph19GeVPt);
    SplittingFigureTools::DrawBesIILamBar(LamBarStatGraph19GeVPt,LamBarSystGraph19GeVPt);
    SplittingFigureTools::DrawInfoText("19.6","20-50\\% Centrality, $|y|<1$",1.4,0.05);
    SplittingFigureTools::DrawLambdaPointLegend(3.35,1.5);
    PtCanvas.cd(0,1);
    SplittingFigureTools::DrawBesIILambda(LambdaStatGraph27GeVPt,LambdaSystGraph27GeVPt);
    SplittingFigureTools::DrawBesIILamBar(LamBarStatGraph27GeVPt,LamBarSystGraph27GeVPt);
    SplittingFigureTools::DrawInfoText("27","20-50\\% Centrality, $|y|<1$",1.4,0.05);
    MyTexFile.AddCanvas(PtCanvas);

    PgfCanvas RapidityCanvas(1,2);
    RapidityCanvas.SetXYTitles("y",PolarizationTitle);
    RapidityCanvas.SetXRanges(-1.7,1.7);
    RapidityCanvas.SetYRanges(-0.22,1.82);
    RapidityCanvas.DrawZeroLines();
    RapidityCanvas.cd(0,0);
    SplittingFigureTools::SetCanvas(RapidityCanvas);
    SplittingFigureTools::DrawBesIILambda(LambdaStatGraph19GeVRapidity,LambdaSystGraph19GeVRapidity);
    SplittingFigureTools::DrawBesIILamBar(LamBarStatGraph19GeVRapidity,LamBarSystGraph19GeVRapidity);
    SplittingFigureTools::DrawInfoText("19.6","20-50\\% Centrality, $p_{\\mathrm{T}}>0.5$~GeV/$c$",0,.05);
    SplittingFigureTools::DrawLambdaPointLegend(1.5,1.65);
    RapidityCanvas.cd(0,1);
    SplittingFigureTools::DrawBesIILambda(LambdaStatGraph27GeVRapidity,LambdaSystGraph27GeVRapidity);
    SplittingFigureTools::DrawBesIILamBar(LamBarStatGraph27GeVRapidity,LamBarSystGraph27GeVRapidity);
    SplittingFigureTools::DrawInfoText("27","20-50\\% Centrality, $p_{\\mathrm{T}}>0.5$~GeV/$c$",0,0.05);
    MyTexFile.AddCanvas(RapidityCanvas);

    Graph StarBesILambdaStat, StarBesILambdaSyst;
    SplittingFigureTools::SetStatAndSystGraphs(StarBesILambdaStat,StarBesILambdaSyst,StarLambdaPolarizationGraphPoints,true);
    Graph StarBesILamBarStat, StarBesILamBarSyst;
    SplittingFigureTools::SetStatAndSystGraphs(StarBesILamBarStat,StarBesILamBarSyst,StarLamBarPolarizationGraphPoints,true);
    Graph StarBesIILambdaStat, StarBesIILambdaSyst;
    SplittingFigureTools::SetStatAndSystGraphs(StarBesIILambdaStat,StarBesIILambdaSyst,NineteenGeVLambdaPolarizationGraphPoints,true);
    Graph StarBesIILamBarStat, StarBesIILamBarSyst;
    SplittingFigureTools::SetStatAndSystGraphs(StarBesIILamBarStat,StarBesIILamBarSyst,NineteenGeVLamBarPolarizationGraphPoints,true);
    Graph AliceLambdaStat, AliceLambdaSyst;
    SplittingFigureTools::SetStatAndSystGraphs(AliceLambdaStat,AliceLambdaSyst,AlicLambdaPolarizationGraphPoints,true);
    Graph AliceLamBarStat, AliceLamBarSyst;
    SplittingFigureTools::SetStatAndSystGraphs(AliceLamBarStat,AliceLamBarSyst,AlicLamBarPolarizationGraphPoints,true);

    PgfCanvas EnergyCanvas(1,2);
      PgfCanvas EnergyCanvasTopPanelZoom(1,1);
      EnergyCanvasTopPanelZoom.SetXRange(15,35);
      EnergyCanvasTopPanelZoom.SetYRange(0,3);
      SplittingFigureTools::SetCanvas(EnergyCanvasTopPanelZoom);
      SplittingFigureTools::DrawBesILambda(StarBesILambdaStat,StarBesILambdaSyst);
      SplittingFigureTools::DrawBesILamBar(StarBesILamBarStat,StarBesILamBarSyst);
      SplittingFigureTools::DrawBesIILambda(StarBesIILambdaStat,StarBesIILambdaSyst);
      SplittingFigureTools::DrawBesIILamBar(StarBesIILamBarStat,StarBesIILamBarSyst);
      EnergyCanvas.AddZoomInset(EnergyCanvasTopPanelZoom);
    EnergyCanvas.SetLogX();
    EnergyCanvas.SetXRanges(1.5,9000);
    EnergyCanvas.cd(0,0);
    EnergyCanvas.SetYTitle(PolarizationTitle);
    EnergyCanvas.SetYRange(-1,12);
    SplittingFigureTools::SetCanvas(EnergyCanvas);
    SplittingFigureTools::DrawBesILambda(StarBesILambdaStat,StarBesILambdaSyst);
    SplittingFigureTools::DrawBesILamBar(StarBesILamBarStat,StarBesILamBarSyst);
    SplittingFigureTools::DrawBesIILambda(StarBesIILambdaStat,StarBesIILambdaSyst);
    SplittingFigureTools::DrawBesIILamBar(StarBesIILamBarStat,StarBesIILamBarSyst);
    SplittingFigureTools::DrawAliceLambda(AliceLambdaStat,AliceLambdaSyst);
    SplittingFigureTools::DrawAliceLamBar(AliceLamBarStat,AliceLamBarSyst);
    EnergyCanvas.cd(0,1);
    EnergyCanvas.SetYRange(-1.6,2.4);
    EnergyCanvas.SetXTitle("\\sNN");
    EnergyCanvas.SetYTitle("P_{\\bar{\\Lambda}}-P_{\\Lambda}~(\\%)");
    MyTexFile.AddCanvas(EnergyCanvas);

    double Res19GeV[] = {0.2048, 0.371, 0.4768, 0.5418, 0.5773, 0.5924, 0.5932, 0.5826, 0.563, 0.5343, 0.495, 0.4474, 0.399, 0.3499, 0.3015, 0.2599};
    double Res27GeV[] = {0.1836, 0.3332, 0.4194, 0.4685, 0.4935, 0.5003, 0.4987, 0.4851, 0.4685, 0.4436, 0.4134, 0.3801, 0.3431, 0.3038, 0.2645, 0.2161};
    TFile ResolutionFile19GeV(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/19GeV/Output/Histograms/%s/%s.Psi1Histograms.root",LambdaCommitHash19GeVCentrality.Data(),LambdaCommitHash19GeVCentrality.Data()));
    TFile ResolutionFile27GeV(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/27GeV/Output/Histograms/%s/%s.Psi1Histograms.root",LambdaCommitHash27GeVCentrality.Data(),LambdaCommitHash27GeVCentrality.Data()));
    Graph Resolution19GeV = ConvertTh1ToGraph( *((TH1D*)ResolutionFile19GeV.Get("Centrality_Resolution_FullEpd")) );
    Graph Resolution27GeV = ConvertTh1ToGraph( *((TH1D*)ResolutionFile27GeV.Get("Centrality_Resolution_FullEpd")) );
    for( int i=0; i<Resolution19GeV.GetN(); i++ ) Resolution19GeV.SetPointY(i,Res19GeV[i]);
    for( int i=0; i<Resolution27GeV.GetN(); i++ ) Resolution27GeV.SetPointY(i,Res27GeV[i]);
    PgfCanvas ResolutionCanvas(1,1);
    ResolutionCanvas.SetXYTitles("\\mathrm{Centrality}~(\\%)","R_{\\mathrm{EP}}^{(1)}");
    ResolutionCanvas.SetXRanges(-5,85);
    ResolutionCanvas.SetYRanges(0,0.65);
    Marker Resolution19GeVMarkerStyle, Resolution27GeVMarkerStyle;
    Resolution19GeVMarkerStyle.Shape = "circle";
    Resolution19GeVMarkerStyle.Size = 2;
    Resolution19GeVMarkerStyle.FillColor = "blue";
    Resolution27GeVMarkerStyle.Shape = "circle";
    Resolution27GeVMarkerStyle.Size = 2;
    Resolution27GeVMarkerStyle.FillColor = "red";
    Resolution19GeV.AddMarkerStyle(Resolution19GeVMarkerStyle);
    Resolution27GeV.AddMarkerStyle(Resolution27GeVMarkerStyle);
    ResolutionCanvas.AddGraph(Resolution19GeV);
    ResolutionCanvas.AddGraph(Resolution27GeV);
    double ResolutionLegendX = 25, ResolutionLegendY = 0.25;
    TString TitleYShift = "5mm";
    TString LegendMarkerXShift = "-8mm";
    TString LegendLabelXShift = "-5mm";
    TString LegendFirstRowYShift = "0mm";
    TString LegendSecndRowYShift = "-5mm";
    ResolutionCanvas.AddNode(Form("\\node[draw, shape=rectangle, minimum width=25mm, minimum height=15mm, anchor=center,fill={rgb:black,0;white,3}] at (axis cs: %f,%f) {};",ResolutionLegendX,ResolutionLegendY));
    ResolutionCanvas.AddNode(Form("\\node[anchor=center,align = center, yshift=%s] at (axis cs: %f,%f){$\\sNN$};",TitleYShift.Data(),ResolutionLegendX,ResolutionLegendY));
	  ResolutionCanvas.AddNode(Form("\\node[color=black, fill=red, line width=0.100000mm, circle, minimum size=2.000000mm, inner sep=0pt, xshift = %s, yshift = %s, draw] at (axis cs: %f,%f){};",LegendMarkerXShift.Data(),LegendFirstRowYShift.Data(),ResolutionLegendX,ResolutionLegendY));
	  ResolutionCanvas.AddNode(Form("\\node[color=black, fill=blue, line width=0.100000mm, circle, minimum size=2.000000mm, inner sep=0pt, xshift = %s, yshift = %s, draw] at (axis cs: %f,%f){};",LegendMarkerXShift.Data(),LegendSecndRowYShift.Data(),ResolutionLegendX,ResolutionLegendY));
    ResolutionCanvas.AddNode(Form("\\node[anchor=west,align = center, xshift=%s, yshift=%s] at (axis cs: %f,%f){19.6~GeV};",LegendLabelXShift.Data(),LegendFirstRowYShift.Data(),ResolutionLegendX,ResolutionLegendY));
    ResolutionCanvas.AddNode(Form("\\node[anchor=west,align = center, xshift=%s, yshift=%s] at (axis cs: %f,%f){27~GeV};",LegendLabelXShift.Data(),LegendSecndRowYShift.Data(),ResolutionLegendX,ResolutionLegendY));
    MyTexFile.AddCanvas(ResolutionCanvas);
}