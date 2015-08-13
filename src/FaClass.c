#include "FaClass.h"
//==============================
// ウィンドウクラス
//==============================

//==============================
// テキストレイヤークラス
//==============================
void TextLayer_Init(TextLayer_class* Obj, int16_t TextBufSize) {
  Obj->Layer = NULL;
  Obj->Rect = GRect(0, 0, 0, 0);
  Obj->Font = fonts_get_system_font(FONT_KEY_GOTHIC_18); //FONT_KEY_BITHAM_42_BOLD
  Obj->BGColor = GColorWhite;
  Obj->FGColor = GColorBlack;
  Obj->HAlign = GTextAlignmentCenter;
  Obj->TextBufSize = TextBufSize;
  Obj->Text = (char*)malloc(TextBufSize);
  Obj->Text[0] = '\0';
}
void TextLayer_Create(TextLayer_class* Obj, int16_t X, int16_t Y, int16_t Width, int16_t Height) {
  Obj->Rect = GRect(X, Y, Width, Height);
  TextLayer      *Layer = Obj->Layer = text_layer_create(Obj->Rect);
  text_layer_set_background_color(Layer, Obj->BGColor);
  text_layer_set_text_color(Layer, Obj->FGColor);
  text_layer_set_text_alignment(Layer, Obj->HAlign);
  text_layer_set_font(Layer, Obj->Font);
}
void TextLayer_SetText(TextLayer_class* Obj, const char* Text) {
  snprintf(Obj->Text, Obj->TextBufSize, "%s", Text);
  text_layer_set_text(Obj->Layer, Obj->Text);
}
void TextLayer_Destroy(TextLayer_class* Obj) {
  if (Obj->Layer == NULL) return;
  text_layer_destroy(Obj->Layer);
  free(Obj->Text);
}
