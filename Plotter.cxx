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

struct Marker{
    TString OutlineColor, FillColor;
    double Size, OutlineWidthScale;
    TString Shape;
    vector<TString> AdditionalNodeOptions;

    Marker(){
        OutlineColor="black";
        FillColor = "blue";
        Size = 3; // mm
        OutlineWidthScale = 0.05;
        Shape = "circle";
    }
    virtual ~Marker(){}

    TString Node(double x, double y){
        TString Options = Form("color=%s, fill=%s, line width=%fmm, %s, minimum size=%fmm, inner sep=0pt",OutlineColor.Data(),FillColor.Data(),OutlineWidthScale*Size,Shape.Data(),Size);
        if( Shape=="star" ) Options.Append(", star point ratio = \\PerfectStarRadiusRatio");
        for( TString Option:AdditionalNodeOptions ) Options.Append(Form(", %s",Option.Data()));
        return Form("\\node[%s, draw] at (axis cs: %f,%f){};",Options.Data(),x,y);
    }
}; // Marker

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
        return MarkerStyle.Node(this->GetPointX(iPoint),this->GetPointY(iPoint));
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

    PgfCanvas(int NX = 1, int NY = 1):NumDivisionsX(NX),NumDivisionsY(NY){
        Width = 83/(double)NX;
        Height = Width;
        XAxes.resize(NX);
        YAxes.resize(NY);
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

void DrawLambdaPoints(Graph &gr, PgfCanvas &can){
    gr.MarkerStyle.Shape = "star";
    gr.MarkerStyle.FillColor = "LambdaFillColor";
    can.AddGraph(gr);
}
void DrawLamBarPoints(Graph &gr, PgfCanvas &can){
    gr.MarkerStyle.Shape = "star";
    gr.MarkerStyle.FillColor = "LamBarFillColor";

    Graph grInner(gr);
    for( int Point=0; Point<grInner.GetN(); Point++ ){
      grInner.SetPointError(Point,0,0,0,0);
    }
    grInner.MarkerStyle.Shape = "star";
    grInner.MarkerStyle.Size = gr.MarkerStyle.Size*0.25;
    grInner.MarkerStyle.OutlineWidthScale = 0;
    grInner.MarkerStyle.OutlineColor = "LamBarInnerFillColor";
    grInner.MarkerStyle.FillColor = "LamBarInnerFillColor";
    can.AddGraph(gr);
    can.AddGraph(grInner);
}
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
    // TString LambdaCommitHash19GeV = "044666513c03b24ef87a6361ff6394455d9acde8";
    // TString LamBarCommitHash19GeV = "5cae15284111395d6a12676b1db189ad894d4e4c";
    // TString LambdaCommitHash27GeV = "cc4f4f6e0dbd646f7013dd693694d11c9c326238";
    // TString LamBarCommitHash27GeV = "079840ff12164ccccfc39ab4bb72ef3fbf2f8f88";

    TFile LambdaFile19GeVCentrality(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/19GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LambdaCommitHash19GeVCentrality.Data(),LambdaCommitHash19GeVCentrality.Data())); TFile LamBarFile19GeVCentrality(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/19GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LamBarCommitHash19GeVCentrality.Data(),LamBarCommitHash19GeVCentrality.Data()));
    Graph LambdaStatGraph19GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LambdaFile19GeVCentrality.Get("Centrality_Polarization_StandardMethod")) );
    Graph LamBarStatGraph19GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LamBarFile19GeVCentrality.Get("Centrality_Polarization_StandardMethod")) );
    TFile LambdaFile27GeVCentrality(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/27GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LambdaCommitHash27GeVCentrality.Data(),LambdaCommitHash27GeVCentrality.Data())); TFile LamBarFile27GeVCentrality(Form("/home/joseph/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/27GeV/Output/Histograms/%s/%s.PolarizationHistograms.root",LamBarCommitHash27GeVCentrality.Data(),LamBarCommitHash27GeVCentrality.Data()));
    Graph LambdaStatGraph27GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LambdaFile27GeVCentrality.Get("Centrality_Polarization_StandardMethod")) );
    Graph LamBarStatGraph27GeVCentrality = ConvertTh1ToGraph( *((TH1D*)LamBarFile27GeVCentrality.Get("Centrality_Polarization_StandardMethod")) );

    TString PolarizationTitle = "P_{H} (\\%)";

    gSystem->Exec(Form("mkdir -p Output/%s",OutputFileCommitHash.Data()));
    TexFile MyTexFile(OutputFileCommitHash);
    PgfCanvas MyCanvas(1,2);
    MyCanvas.SetXYTitles("\\mathrm{Centrality} (\\%)",PolarizationTitle);
    MyCanvas.SetXRange(-5,85);
    MyCanvas.SetYRange(-0.65,3.65);
    MyCanvas.cd(0,0);
    DrawLambdaPoints(LambdaStatGraph19GeVCentrality,MyCanvas);
    DrawLamBarPoints(LamBarStatGraph19GeVCentrality,MyCanvas);
    MyCanvas.cd(0,1);
    DrawLambdaPoints(LambdaStatGraph27GeVCentrality,MyCanvas);
    DrawLamBarPoints(LamBarStatGraph27GeVCentrality,MyCanvas);

    MyTexFile.AddCanvas(MyCanvas);
}