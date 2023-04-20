#ifndef ERRORBAR_HPP
#define ERRORBAR_HPP

class ErrorBar{
private:

public:
  double Width;
  TString Color;

  ErrorBar(){
    Width = 0.3; // mm
    Color = "black";
  }
  virtual ~ErrorBar(){}
};
#endif