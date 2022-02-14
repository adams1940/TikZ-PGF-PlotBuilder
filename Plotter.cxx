struct Axis{
    double Min, Max;
    int NumMinorTicks;
    vector<double> CustomMajorTicks, CustomMinorTicks;
    TString Title, TitleSize;
    Axis(){
        NumMinorTicks = 1;
        Min = 0;
        Max = 1;
        TitleSize = "\\large";
    }
    virtual ~Axis(){};
}; // Axis

class PgfCanvas{
private:
    int ActivePadX, ActivePadY;

public:
    int NumDivisionsX, NumDivisionsY;
    double Width, Height; // mm
    vector<Axis> XAxes, YAxes;
    vector<TGraphAsymmErrors> Graphs;

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

    void AddGraph(TGraphAsymmErrors gr){
        Graphs.push_back(gr);
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
        File<<"\\documentclass[10pt]{article}\n";
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
        File<<"\\begin{center}\n";
        File<<"\n";
    } // TexFile 
    
    virtual ~TexFile(){
        File<<"\\end{center}\n";
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



                //for( int iGraph=0; iGraph<Canvas.Graphs.size(); iGraph++ ){
                for( TGraphAsymmErrors gr:Canvas.Graphs ){
                    AddPictureLine("\\addplot[scatter, only marks, error bars/.cd, error mark = none, error bar style = {line width=0.4mm,solid}, y dir = both, y explicit, x dir = both, x explicit]");
                    AddPictureLine("\t table[x index = 0, x error minus index = 1, x error plus index = 2, y index = 3, y error minus index = 4, y error plus index = 5]{");
                    for( int iPoint=0; iPoint<gr.GetN(); iPoint++ ){
                        AddPictureLine(Form("\t\t%f %f %f %f %f %f",gr.GetPointX(iPoint),gr.GetErrorXlow(iPoint),gr.GetErrorXhigh(iPoint), gr.GetPointY(iPoint),gr.GetErrorYlow(iPoint),gr.GetErrorYhigh(iPoint)));
                    }
                    AddPictureLine("\t };");
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

void Plotter(TString OutputFileCommitHash = "test"){
    gSystem->Exec(Form("mkdir -p Output/%s",OutputFileCommitHash.Data()));
    TexFile MyTexFile(OutputFileCommitHash);
    PgfCanvas MyCanvas;
    MyCanvas.ActiveXAxis().Title = "x";
    MyCanvas.ActiveYAxis().Title = "y";
    TGraphAsymmErrors gr;
    for( int i=0 ;i<10; i++ ){
        TRandom3 ran(0);
        double min = (double)i/10., max = (double)(i+1)/10.;
        gr.SetPoint(i,ran.Uniform(min,max),ran.Uniform(min,max));
        gr.SetPointError(i,ran.Uniform(0,0.05),ran.Uniform(0,0.05),ran.Uniform(0,0.05),ran.Uniform(0,0.05));
    }
    MyCanvas.AddGraph(gr);

    MyTexFile.AddCanvas(MyCanvas);
}