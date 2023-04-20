TString sNNTitle = "\\sqrt{s_\\mathrm{NN}}";

#include "Classes/Axis.h"
#include "Classes/Node.h"
#include "Classes/Marker.h"
#include "Classes/Box.h"
#include "Classes/TextBox.h"
#include "Classes/ErrorBar.h"
#include "Classes/Graph.h"
#include "Classes/PgfCanvas.h"
#include "Classes/TexFile.h"

namespace SplittingFigureTools{
  Marker BesIILambdaMarkerStyle, BesIILamBarMarkerStyle, BesIILamBarInnerMarkerStyle;
  Marker BesILambdaMarkerStyle, BesILamBarMarkerStyle;
  Marker AliceLambdaMarkerStyle, AliceLamBarMarkerStyle;
  Marker BesISplittingMarkerStyle, BesIISplittingMarkerStyle, AliceSplittingMarkerStyle;
  double XShift;
  PgfCanvas * can;
  double BesISizeFactor;
  TString LambdaFillColor, LamBarFillColor, LamBarInnerFillColor, GrayColor;

  void SetMarkerStyles(){
    XShift = .5;
    BesISizeFactor = 0.7;
    LambdaFillColor = "{rgb:red,51;green,102;blue,255}";
    LamBarFillColor = "{rgb:red,204;green,16;blue,0}";
    LamBarInnerFillColor = "white";
    GrayColor = "{rgb:black,1;white,3}";

    BesIILambdaMarkerStyle.Shape = "star";
    BesIILambdaMarkerStyle.FillColor = LambdaFillColor;
    BesIILamBarMarkerStyle.Shape = "star";
    BesIILamBarMarkerStyle.FillColor = LamBarFillColor;
    BesIILamBarInnerMarkerStyle.Shape = "star";
    BesIILamBarInnerMarkerStyle.Size = BesIILamBarMarkerStyle.Size*0.25;
    BesIILamBarInnerMarkerStyle.OutlineWidthScale = 0;
    BesIILamBarInnerMarkerStyle.OutlineColor = LamBarInnerFillColor;
    BesIILamBarInnerMarkerStyle.FillColor = LamBarInnerFillColor;

    BesILambdaMarkerStyle = BesIILambdaMarkerStyle;
    BesILambdaMarkerStyle.FillColor = GrayColor;
    BesILambdaMarkerStyle.Size*=BesISizeFactor;
    BesILamBarMarkerStyle = BesIILamBarMarkerStyle;
    BesILamBarMarkerStyle.FillColor = "white";
    BesILamBarMarkerStyle.Size*=BesISizeFactor;
    
    AliceLambdaMarkerStyle.Shape = "rectangle";
    AliceLambdaMarkerStyle.FillColor = GrayColor;
    AliceLambdaMarkerStyle.Size = BesILambdaMarkerStyle.Size;
    AliceLamBarMarkerStyle.Shape = "rectangle";
    AliceLamBarMarkerStyle.FillColor = "white";
    AliceLamBarMarkerStyle.Size = BesILamBarMarkerStyle.Size;

    BesISplittingMarkerStyle.Shape = "star,star points=4, star point ratio=2.618034";
    BesISplittingMarkerStyle.FillColor = GrayColor;
    BesIISplittingMarkerStyle.Shape = BesISplittingMarkerStyle.Shape;
    BesIISplittingMarkerStyle.FillColor = "pink";
    AliceSplittingMarkerStyle.Shape = "diamond";
    AliceSplittingMarkerStyle.FillColor = GrayColor;
  }

vector<vector<double>> Splitting(vector<vector<double>> LambdaPoints, vector<vector<double>> LamBarPoints){
  vector<vector<double>> Differences;
  for( int iEnergy = 0; iEnergy<LamBarPoints.size(); iEnergy++ ){
    if( LambdaPoints[iEnergy][0]==3 ) break;
    vector<double> Diff;
    Diff.push_back(LambdaPoints[iEnergy][0]);
    Diff.push_back(LamBarPoints[iEnergy][1]-LambdaPoints[iEnergy][1]);
    Diff.push_back(sqrt(pow(LamBarPoints[iEnergy][2],2)+pow(LambdaPoints[iEnergy][2],2)));
    Diff.push_back(sqrt(pow(LamBarPoints[iEnergy][3],2)+pow(LambdaPoints[iEnergy][3],2)));
    Diff.push_back(sqrt(pow(LamBarPoints[iEnergy][4],2)+pow(LambdaPoints[iEnergy][4],2)));
    Differences.push_back(Diff);
  }
  return Differences;
} // Splitting

  void SetStatAndSystGraphs(Graph &StatisticalErrorGraph, Graph &SystematicErrorGraph, vector<vector<double>> MeasuredPolarizationDataPoints){
    const int NumDataPoints = (const int)MeasuredPolarizationDataPoints.size();
    for( int iDataPoint = 0; iDataPoint<NumDataPoints; iDataPoint++ ){
      double Energy = MeasuredPolarizationDataPoints[iDataPoint][0];
      double Polarization = MeasuredPolarizationDataPoints[iDataPoint][1];
      double StatErr = MeasuredPolarizationDataPoints[iDataPoint][2];
      double SystErrLow = MeasuredPolarizationDataPoints[iDataPoint][3];
      double SystErrHgh = MeasuredPolarizationDataPoints[iDataPoint][4];
      double LeftSystErr = 0;
      double RghtSystErr = 0;
      double StatXErr = 0.;
      StatisticalErrorGraph.SetPoint(iDataPoint,Energy,Polarization);
      StatisticalErrorGraph.SetPointError(iDataPoint,StatXErr,StatXErr,StatErr,StatErr);
      SystematicErrorGraph.SetPoint(iDataPoint,Energy,Polarization);
      SystematicErrorGraph.SetPointError(iDataPoint,LeftSystErr,RghtSystErr,SystErrLow,SystErrHgh);
    } // iDataPoint
  }

  void SetCanvas(PgfCanvas &c){
    can = &c;
  }

  void DrawData(Graph &StatGraph, Graph &SystGraph){
    StatGraph.SystematicErrorGraph = SystGraph;
    can->AddGraph(StatGraph);
  }

  void DrawBesISplitting(vector<vector<double>> LambdaPoints, vector<vector<double>> LamBarPoints){
    vector<vector<double>> SplittingPoints = Splitting(LambdaPoints,LamBarPoints);
    Graph StatGraph, SystGraph;
    SetStatAndSystGraphs(StatGraph,SystGraph,SplittingPoints);
    StatGraph.AddMarkerStyle(BesISplittingMarkerStyle);
    DrawData(StatGraph,SystGraph);
  }

  void DrawBesIISplitting(vector<vector<double>> LambdaPoints, vector<vector<double>> LamBarPoints){
    vector<vector<double>> SplittingPoints = Splitting(LambdaPoints,LamBarPoints);
    Graph StatGraph, SystGraph;
    SetStatAndSystGraphs(StatGraph,SystGraph,SplittingPoints);
    StatGraph.AddMarkerStyle(BesIISplittingMarkerStyle);
    DrawData(StatGraph,SystGraph);
  }

  void DrawAliceSplitting(vector<vector<double>> LambdaPoints, vector<vector<double>> LamBarPoints){
    vector<vector<double>> SplittingPoints = Splitting(LambdaPoints,LamBarPoints);
    Graph StatGraph, SystGraph;
    SetStatAndSystGraphs(StatGraph,SystGraph,SplittingPoints);
    StatGraph.AddMarkerStyle(AliceSplittingMarkerStyle);
    DrawData(StatGraph,SystGraph);
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
    text->Text = Form("STAR Au+Au, $%s=%s$~GeV\\\\%s\\\\$\\alpha_\\Lambda=0.732$",sNNTitle.Data(),Energy.Data(),PtRapidityCentrality.Data());
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
}; // SplittingFigureTools

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
      19.6, // 044666513c03b24ef87a6361ff6394455d9acde8
      0.9152,
      0.0503,
      0.0172,
      0.0172,
    },
    {
      27,
      0.7168,
      0.0553,
      0.0143,
      0.0143,
    },
  };
  vector<double> NineteenGeVEnergies; for( int i=0; i<NineteenGeVLambdaPolarizationGraphPoints.size(); i++ ) NineteenGeVEnergies.push_back(NineteenGeVLambdaPolarizationGraphPoints[i][0]);
  vector<vector<double>> NineteenGeVLamBarPolarizationGraphPoints = {
    {
      19.6, // 5cae15284111395d6a12676b1db189ad894d4e4c
      0.8976,
      0.1170,
      0.0171,
      0.0171,
    },
    {
      27,
      0.8257,
      0.1046,
      0.0163,
      0.0163,
    },
  };
  vector<vector<double>> StarLambdaPolarizationGraphPoints = {
    {7.7,   AlphaChangeFactor*2.039,  AlphaChangeFactor*0.628,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {11.5,  AlphaChangeFactor*1.344,  AlphaChangeFactor*0.396,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {14.5,  AlphaChangeFactor*1.321,  AlphaChangeFactor*0.482,  AlphaChangeFactor*0.3,    AlphaChangeFactor*0.0   },
    {19.6,  AlphaChangeFactor*0.950,  AlphaChangeFactor*0.305,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {27.0,  AlphaChangeFactor*1.047,  AlphaChangeFactor*0.282,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {39.0,  AlphaChangeFactor*0.506,  AlphaChangeFactor*0.424,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {62.4,  AlphaChangeFactor*1.334,  AlphaChangeFactor*1.167,  AlphaChangeFactor*0.2,    AlphaChangeFactor*0.0   },
    {200.0, AlphaChangeFactor*0.277,  AlphaChangeFactor*0.040,  AlphaChangeFactor*0.049,  AlphaChangeFactor*0.039 },
    {3.,    4.90823,                  0.81389,                  0.15485,                  0.15485                   },  // from joseph@joseph-Latitude-5590:~/Documents/Coding/OSUResearch/LambdaPolarizationAnalyses/Local/3GeV/Output/Histograms/1630bfc58b466c639cf8e643b4e9f5aafc6a11d5.PolarizationHistograms.root
  };
  vector<vector<double>> StarLamBarPolarizationGraphPoints = {
    {7.7,   AlphaChangeFactor*8.669,  AlphaChangeFactor*3.569,  AlphaChangeFactor*1.00,   AlphaChangeFactor*0.0   },
    {11.5,  AlphaChangeFactor*1.802,  AlphaChangeFactor*1.261,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.0   },
    {14.5,  AlphaChangeFactor*2.276,  AlphaChangeFactor*1.210,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.4   },
    {19.6,  AlphaChangeFactor*1.515,  AlphaChangeFactor*0.610,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.0   },
    {27.0,  AlphaChangeFactor*1.245,  AlphaChangeFactor*0.471,  AlphaChangeFactor*0.15,   AlphaChangeFactor*0.0   },
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
    SplittingFigureTools::SetStatAndSystGraphs(StarBesILambdaStat,StarBesILambdaSyst,StarLambdaPolarizationGraphPoints);
    Graph StarBesILamBarStat, StarBesILamBarSyst;
    SplittingFigureTools::SetStatAndSystGraphs(StarBesILamBarStat,StarBesILamBarSyst,StarLamBarPolarizationGraphPoints);
    Graph StarBesIILambdaStat, StarBesIILambdaSyst;
    SplittingFigureTools::SetStatAndSystGraphs(StarBesIILambdaStat,StarBesIILambdaSyst,NineteenGeVLambdaPolarizationGraphPoints);
    Graph StarBesIILamBarStat, StarBesIILamBarSyst;
    SplittingFigureTools::SetStatAndSystGraphs(StarBesIILamBarStat,StarBesIILamBarSyst,NineteenGeVLamBarPolarizationGraphPoints);
    Graph AliceLambdaStat, AliceLambdaSyst;
    SplittingFigureTools::SetStatAndSystGraphs(AliceLambdaStat,AliceLambdaSyst,AlicLambdaPolarizationGraphPoints);
    Graph AliceLamBarStat, AliceLamBarSyst;
    SplittingFigureTools::SetStatAndSystGraphs(AliceLamBarStat,AliceLamBarSyst,AlicLamBarPolarizationGraphPoints);

    PgfCanvas EnergyCanvas(1,2);
      PgfCanvas EnergyCanvasTopPanelZoom(1,1);
      EnergyCanvasTopPanelZoom.SetXRange(15,35);
      EnergyCanvasTopPanelZoom.SetYRange(0,3);
      SplittingFigureTools::SetCanvas(EnergyCanvasTopPanelZoom);
      SplittingFigureTools::DrawBesILambda(StarBesILambdaStat,StarBesILambdaSyst);
      SplittingFigureTools::DrawBesILamBar(StarBesILamBarStat,StarBesILamBarSyst);
      SplittingFigureTools::DrawBesIILambda(StarBesIILambdaStat,StarBesIILambdaSyst);
      SplittingFigureTools::DrawBesIILamBar(StarBesIILamBarStat,StarBesIILamBarSyst);
      // EnergyCanvas.AddZoomInset(EnergyCanvasTopPanelZoom);
      TexFile ZoomInsetFile("temp_ZoomInset_Top");
      ZoomInsetFile.AddCanvas(EnergyCanvasTopPanelZoom);
    EnergyCanvas.DrawZeroLines();
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
    EnergyCanvas.SetXTitle(sNNTitle.Data());
    EnergyCanvas.SetYTitle("P_{\\bar{\\Lambda}}-P_{\\Lambda}~(\\%)");
    SplittingFigureTools::DrawBesISplitting(StarLambdaPolarizationGraphPoints,StarLamBarPolarizationGraphPoints);
    SplittingFigureTools::DrawBesIISplitting(NineteenGeVLambdaPolarizationGraphPoints,NineteenGeVLamBarPolarizationGraphPoints);
    SplittingFigureTools::DrawAliceSplitting(AlicLambdaPolarizationGraphPoints,AlicLamBarPolarizationGraphPoints);
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
    ResolutionCanvas.AddNode(Form("\\node[anchor=center,align = center, yshift=%s] at (axis cs: %f,%f){$%s$};",TitleYShift.Data(),ResolutionLegendX,ResolutionLegendY,sNNTitle.Data()));
	  ResolutionCanvas.AddNode(Form("\\node[color=black, fill=red, line width=0.100000mm, circle, minimum size=2.000000mm, inner sep=0pt, xshift = %s, yshift = %s, draw] at (axis cs: %f,%f){};",LegendMarkerXShift.Data(),LegendFirstRowYShift.Data(),ResolutionLegendX,ResolutionLegendY));
	  ResolutionCanvas.AddNode(Form("\\node[color=black, fill=blue, line width=0.100000mm, circle, minimum size=2.000000mm, inner sep=0pt, xshift = %s, yshift = %s, draw] at (axis cs: %f,%f){};",LegendMarkerXShift.Data(),LegendSecndRowYShift.Data(),ResolutionLegendX,ResolutionLegendY));
    ResolutionCanvas.AddNode(Form("\\node[anchor=west,align = center, xshift=%s, yshift=%s] at (axis cs: %f,%f){19.6~GeV};",LegendLabelXShift.Data(),LegendFirstRowYShift.Data(),ResolutionLegendX,ResolutionLegendY));
    ResolutionCanvas.AddNode(Form("\\node[anchor=west,align = center, xshift=%s, yshift=%s] at (axis cs: %f,%f){27~GeV};",LegendLabelXShift.Data(),LegendSecndRowYShift.Data(),ResolutionLegendX,ResolutionLegendY));
    MyTexFile.AddCanvas(ResolutionCanvas);
}