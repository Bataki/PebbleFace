#pragma once
#include <pebble.h>
//==============================
// ウィンドウクラス
//==============================
typedef struct {
  Window*      Window;
} Window_class;
//==============================
// テキストレイヤークラス
//==============================
typedef struct {
  TextLayer*     Layer;
  GRect          Rect;
  GFont          Font;
  GColor         BGColor;
  GColor         FGColor;
  GTextAlignment HAlign;
  int16_t        TextBufSize;
  char*          Text;
} TextLayer_class;

void TextLayer_Init(TextLayer_class* Obj, int16_t TextBufSize);
void TextLayer_Create(TextLayer_class* Obj, int16_t X, int16_t Y, int16_t Width, int16_t Height);
void TextLayer_SetText(TextLayer_class* Obj, const char* Text);
void TextLayer_Destroy(TextLayer_class* Obj);
