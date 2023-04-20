#ifndef AXIS_HPP
#define AXIS_HPP

class Axis{
private:

public:
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
};
#endif