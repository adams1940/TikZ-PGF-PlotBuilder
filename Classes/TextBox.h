#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP

#include "Node.h"

class TextBox : public Node{
private:

public:
  TextBox(double x = 0, double y = 0){
    AnchorX = x;
    AnchorY = y;
    IncludeDraw = false;
  }
  void DrawBorder(bool yn){
    IncludeDraw = yn;
  }
};
#endif