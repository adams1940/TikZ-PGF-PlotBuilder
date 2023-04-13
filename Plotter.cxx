struct Axis{
    double Min, Max;
    int NumMinorTicks;
    vector<double> CustomMajorTicks, CustomMinorTicks;
    TString Title, TitleSize;
    Axis(){
        NumMinorTicks = 4;
        Min = 0;
        Max = 1;
        TitleSize = "\\large";
    }
    virtual ~Axis(){};
}; // Axis

struct Node{
  TString OutlineColor, FillColor;
  vector<TString> Options;
  // double x,y;
  
  // Node(double x = 0, double y = 0):x(x),y(y){}
  Node(){}
  virtual ~Node(){}

  TString NodeText(double x, double y){
    if( OutlineColor!="" ) Options.push_back(Form("color=%s",OutlineColor.Data()));
    if( FillColor!="" ) Options.push_back(Form("fill=%s",FillColor.Data()));
    TString OptionsWithCommas;
    for( TString Option:Options ) OptionsWithCommas.Append(Form("%s, ",Option.Data()));
    return Form("\\node[%sdraw] at (axis cs: %f,%f){};",OptionsWithCommas.Data(),x,y);
  }
};

struct Marker:Node{
  double Size, OutlineWidthScale;
  TString Shape;
  double StarRatio = 2.618034;

  Marker(){
      OutlineColor="black";
      FillColor = "blue";
      Size = 3; // mm
      OutlineWidthScale = 0.05;
      Shape = "circle";
  }
  virtual ~Marker(){}

  TString NodeText(double x, double y){
    // if new, Marker-specific members or Options are introduced, then do Options.push_back() and return Node::NodeText();
    Options.push_back("inner sep=0pt");
    Options.push_back(Form("minimum size=%fmm",Size));
    Options.push_back(Form("line width=%fmm",OutlineWidthScale*Size));
    Options.push_back(Shape.Data());
    if( Shape=="star" ) Options.push_back(Form("star point ratio=%f",StarRatio));
    return Node::NodeText(x,y);
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
    Marker MarkerStyle;
    ErrorBar ErrorBarStyle;
    bool DrawXError, DrawYError;
    bool DrawLines;
    bool OnlyDrawLines;
    double LineWidth;
    TString LineColor;

    Graph(){
        DrawXError = true;
        DrawYError = true;
        DrawLines = false;
        OnlyDrawLines = false;
        LineWidth = 0.2; // mm
        LineColor = "blue";
    }    
    TString MarkerNode(int iPoint){
        return MarkerStyle.NodeText(this->GetPointX(iPoint),this->GetPointY(iPoint));
    }
}; // Graph

class PgfCanvas{
private:
    int ActivePadX, ActivePadY;

public:
    int NumDivisionsX, NumDivisionsY;
    double Width, Height; // mm
    vector<Axis> XAxes, YAxes;
    vector<Graph> Graphs[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]
    vector<TString> AdditionalNodes[10][10]; // Ideally would be [NumDivisionsX][NumDivisionsY]
    vector<vector<bool>> DrawZeroLinesVector;
    vector<vector<TString>> Texts;
    vector<vector<pair<double,double>>> TextPositions;

    PgfCanvas(int NX = 1, int NY = 1):NumDivisionsX(NX),NumDivisionsY(NY){
        Width = 100/(double)NX;
        Height = 0.9*Width;
        XAxes.resize(NX);
        YAxes.resize(NY);
        for( int iX=0; iX<NX; iX++ ) DrawZeroLinesVector.push_back(vector<bool>(NY));
        for( int iX=0; iX<NX; iX++ ) Texts.push_back(vector<TString>(NY));
        for( int iX=0; iX<NX; iX++ ) TextPositions.push_back(vector<pair<double,double>>(NY));
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

    void SetXYTitles(TString XTitle, TString YTitle){
      for( Axis &axis:XAxes ) axis.Title = XTitle;
      for( Axis &axis:YAxes ) axis.Title = YTitle;
    }

    void SetXRange(double Min, double Max){
      for( Axis &axis:XAxes ){
        axis.Min = Min;
        axis.Max = Max;
      }
    }

    void SetYRange(double Min, double Max){
      for( Axis &axis:YAxes ){
        axis.Min = Min;
        axis.Max = Max;
      }
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

    void AddText(TString Text, double x, double y){
      Texts[ActivePadX][ActivePadY] = Text;
      TextPositions[ActivePadX][ActivePadY] = pair<double,double>(x,y);
    }

    void AddNode(TString Text){
      AdditionalNodes[ActivePadX][ActivePadY].push_back(Text);
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

                for( TString Node:Canvas.AdditionalNodes[ColumnX][ColumnY] ) AddPictureLine(Node.Data());

                if( Canvas.Texts[ColumnX][ColumnY]!="" ) AddPictureLine(Form("\\node [anchor=center, align=center] at (axis cs: %f,%f){%s};",Canvas.TextPositions[ColumnX][ColumnY].first,Canvas.TextPositions[ColumnX][ColumnY].second,Canvas.Texts[ColumnX][ColumnY].Data()));

                for( Graph gr:Canvas.Graphs[ColumnX][ColumnY] ){
                    if( gr.DrawLines ){
                        for( int iPoint=0; iPoint<gr.GetN()-1; iPoint++ ){
                            AddPictureLine(Form("\\draw[line width = %fmm, %s](%f,%f)--(%f,%f);",gr.LineWidth,gr.LineColor.Data(),gr.GetPointX(iPoint),gr.GetPointY(iPoint),gr.GetPointX(iPoint+1),gr.GetPointY(iPoint+1)));
                        }
                    }
                    if( gr.OnlyDrawLines ) continue;
                    AddPictureLine(Form("\\addplot[scatter, only marks, forget plot, no markers, error bars/.cd, error mark = none, error bar style = {line width=%fmm,solid}, x dir = %s, x explicit, y dir = %s, y explicit]",gr.ErrorBarStyle.Width,gr.DrawXError?"both":"none",gr.DrawYError?"both":"none"));
                    AddPictureLine("\t table[x index = 0, x error minus index = 1, x error plus index = 2, y index = 3, y error minus index = 4, y error plus index = 5]{");
                    for( int iPoint=0; iPoint<gr.GetN(); iPoint++ ){
                        AddPictureLine(Form("\t\t%f %f %f %f %f %f",gr.GetPointX(iPoint),gr.GetErrorXlow(iPoint),gr.GetErrorXhigh(iPoint), gr.GetPointY(iPoint),gr.GetErrorYlow(iPoint),gr.GetErrorYhigh(iPoint)));
                    }
                    AddPictureLine("\t };");

                    for( int iPoint=0; iPoint<gr.GetN(); iPoint++ ){
                        AddPictureLine(gr.MarkerNode(iPoint));
                    }
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

void DrawLambdaStatPoints(Graph &gr, PgfCanvas &can){
    if( gr.GetN()>0 ) gr.MovePoints(-gr.GetErrorX(0)/10.,0);
    gr.MarkerStyle.Shape = "star";
    gr.MarkerStyle.FillColor = "LambdaFillColor";
    can.AddGraph(gr);
}

void DrawLamBarStatPoints(Graph &gr, PgfCanvas &can){
    if( gr.GetN()>0 ) gr.MovePoints(gr.GetErrorX(0)/10.,0);
    gr.MarkerStyle.Shape = "star";
    gr.MarkerStyle.FillColor = "LamBarFillColor";

}

void DrawData(Graph &StatGraph, Graph &SystGraph, PgfCanvas &can, bool IsLambda){
  int ProtonCharge;
  TString ParentName;
  if( IsLambda ){ ProtonCharge = 1; ParentName = "Lambda"; }
  else { ProtonCharge = -1; ParentName = "LamBar"; };

  if( StatGraph.GetN()>0 ){
    double XShift = 0.1;
    StatGraph.MovePoints(-ProtonCharge*XShift*StatGraph.GetErrorX(0),0);
    SystGraph.MovePoints(-ProtonCharge*XShift*StatGraph.GetErrorX(0),0);
  }

  StatGraph.MarkerStyle.Shape = "star";
  StatGraph.MarkerStyle.FillColor = Form("%sFillColor",ParentName.Data());

  can.AddGraph(StatGraph);

  double SystBoxWidthFactor=0.4;
  for( int iPoint=0; iPoint<SystGraph.GetN(); iPoint++ ){
    can.AddNode(Form("\\draw[thick] (axis cs: %f,%f) rectangle (axis cs: %f,%f);",SystGraph.GetPointX(iPoint)-SystBoxWidthFactor*StatGraph.GetErrorX(0),SystGraph.GetPointY(iPoint)-SystGraph.GetErrorY(iPoint),SystGraph.GetPointX(iPoint)+SystBoxWidthFactor*StatGraph.GetErrorX(0),SystGraph.GetPointY(iPoint)+SystGraph.GetErrorY(iPoint)));
  }

  Graph grInner(StatGraph);
  if( !IsLambda ){
    for( int Point=0; Point<grInner.GetN(); Point++ ){
      grInner.SetPointError(Point,0,0,0,0);
    }
    grInner.MarkerStyle.Shape = "star";
    grInner.MarkerStyle.Size = StatGraph.MarkerStyle.Size*0.25;
    grInner.MarkerStyle.OutlineWidthScale = 0;
    grInner.MarkerStyle.OutlineColor = "LamBarInnerFillColor";
    grInner.MarkerStyle.FillColor = "LamBarInnerFillColor";
    can.AddGraph(grInner);
  }
} // DrawData

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

void DrawInfoText(PgfCanvas &can, TString Energy, TString PtRapidityCentrality, double x, double y){
  can.AddText(Form("STAR Au+Au, $\\sNN=%s$~GeV\\\\%s\\\\$\\alpha_\\Lambda=0.732$",Energy.Data(),PtRapidityCentrality.Data()),x,y);
}

void DrawLambdaPointLegend(PgfCanvas &can, double x, double y){
  can.AddNode(Form("\\node[draw, shape=rectangle, minimum width=1cm, minimum height=1cm, anchor=center,fill={rgb:black,1;white,3}] at (%f,%f) {};",x,y));
  can.AddNode(Form("\\node[anchor=center,align = center, xshift=-2.5mm, yshift=2.5mm] at (axis cs: %f,%f){$\\Lambda$};",x,y));
  can.AddNode(Form("\\node[anchor=center,align = center, xshift= 2.5mm, yshift=2.5mm] at (axis cs: %f,%f){$\\bar{\\Lambda}$};",x,y));
  can.AddNode(Form("\\node[color=black, fill=LambdaFillColor, line width=0.150000mm, star, minimum size=3.000000mm, inner sep=0pt, star point ratio = \\PerfectStarRadiusRatio, xshift=-2.5mm, yshift=-2.5mm, draw] at (axis cs: %f,%f){};",x,y));
  can.AddNode(Form("\\node[color=black, fill=LamBarFillColor, line width=0.150000mm, star, minimum size=3.000000mm, inner sep=0pt, star point ratio = \\PerfectStarRadiusRatio, xshift= 2.5mm, yshift=-2.5mm, draw] at (axis cs: %f,%f){};",x,y));
	can.AddNode(Form("\\node[color=LamBarInnerFillColor, fill=LamBarInnerFillColor, line width=0.000000mm, star, minimum size=0.750000mm, inner sep=0pt, star point ratio = \\PerfectStarRadiusRatio, xshift= 2.5mm, yshift=-2.5mm, draw] at (axis cs: %f,%f){};",x,y));
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

    gSystem->Exec(Form("mkdir -p Output/%s",OutputFileCommitHash.Data()));
    TexFile MyTexFile(OutputFileCommitHash);

    PgfCanvas CentralityCanvas(1,2);
    CentralityCanvas.SetXYTitles("\\mathrm{Centrality}~(\\%)",PolarizationTitle);
    CentralityCanvas.SetXRange(-5,85);
    CentralityCanvas.SetYRange(-0.65,3.65);
    CentralityCanvas.DrawZeroLines();
    CentralityCanvas.cd(0,0);
    DrawData(LambdaStatGraph19GeVCentrality,LambdaSystGraph19GeVCentrality,CentralityCanvas,true);
    DrawData(LamBarStatGraph19GeVCentrality,LamBarSystGraph19GeVCentrality,CentralityCanvas,false);
    DrawInfoText(CentralityCanvas,"19.6","$p_{\\mathrm{T}}>0.5$~GeV/$c$, $|y|<1$",22,2.8);
    CentralityCanvas.cd(0,1);
    DrawData(LambdaStatGraph27GeVCentrality,LambdaSystGraph27GeVCentrality,CentralityCanvas,true);
    DrawData(LamBarStatGraph27GeVCentrality,LamBarSystGraph27GeVCentrality,CentralityCanvas,false);
    DrawLambdaPointLegend(CentralityCanvas,15,1.5);
    DrawInfoText(CentralityCanvas,"27","$p_{\\mathrm{T}}>0.5$~GeV/$c$, $|y|<1$",22,2.8);
    MyTexFile.AddCanvas(CentralityCanvas);

    PgfCanvas PtCanvas(1,2);
    PtCanvas.SetXYTitles("p_{\\mathrm{T}}~(\\mathrm{GeV}/c)",PolarizationTitle);
    PtCanvas.SetXRange(0.3,3.7);
    PtCanvas.SetYRange(-0.22,1.82);
    PtCanvas.DrawZeroLines();
    PtCanvas.cd(0,0);
    DrawData(LambdaStatGraph19GeVPt,LambdaSystGraph19GeVPt,PtCanvas,true);
    DrawData(LamBarStatGraph19GeVPt,LamBarSystGraph19GeVPt,PtCanvas,false);
    DrawInfoText(PtCanvas,"19.6","20-50\\% Centrality, $|y|<1$",1.4,0.05);
    DrawLambdaPointLegend(PtCanvas,3.35,1.5);
    PtCanvas.cd(0,1);
    DrawData(LambdaStatGraph27GeVPt,LambdaSystGraph27GeVPt,PtCanvas,true);
    DrawData(LamBarStatGraph27GeVPt,LamBarSystGraph27GeVPt,PtCanvas,false);
    DrawInfoText(PtCanvas,"27","20-50\\% Centrality, $|y|<1$",1.4,0.05);
    MyTexFile.AddCanvas(PtCanvas);

    PgfCanvas RapidityCanvas(1,2);
    RapidityCanvas.SetXYTitles("y",PolarizationTitle);
    RapidityCanvas.SetXRange(-1.7,1.7);
    RapidityCanvas.SetYRange(-0.22,1.82);
    RapidityCanvas.DrawZeroLines();
    RapidityCanvas.cd(0,0);
    DrawData(LambdaStatGraph19GeVRapidity,LambdaSystGraph19GeVRapidity,RapidityCanvas,true);
    DrawData(LamBarStatGraph19GeVRapidity,LamBarSystGraph19GeVRapidity,RapidityCanvas,false);
    DrawInfoText(RapidityCanvas,"19.6","20-50\\% Centrality, $p_{\\mathrm{T}}>0.5$~GeV/$c$",0,.05);
    DrawLambdaPointLegend(RapidityCanvas,1.5,1.65);
    RapidityCanvas.cd(0,1);
    DrawData(LambdaStatGraph27GeVRapidity,LambdaSystGraph27GeVRapidity,RapidityCanvas,true);
    DrawData(LamBarStatGraph27GeVRapidity,LamBarSystGraph27GeVRapidity,RapidityCanvas,false);
    DrawInfoText(RapidityCanvas,"27","20-50\\% Centrality, $p_{\\mathrm{T}}>0.5$~GeV/$c$",0,0.05);
    MyTexFile.AddCanvas(RapidityCanvas);

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
    ResolutionCanvas.SetXRange(-5,85);
    ResolutionCanvas.SetYRange(0,0.65);
    Resolution19GeV.MarkerStyle.Shape = "circle";
    Resolution19GeV.MarkerStyle.Size = 2;
    Resolution19GeV.MarkerStyle.FillColor = "blue";
    Resolution27GeV.MarkerStyle.Shape = "circle";
    Resolution27GeV.MarkerStyle.Size = 2;
    Resolution27GeV.MarkerStyle.FillColor = "red";
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