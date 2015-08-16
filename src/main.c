#include <pebble.h>
//===================================================================================================
// ライブラリヘッダ
//===================================================================================================
time_t gTime;
#define pLOCTIME (gTime=time(NULL), localtime(&gTime))
//===================================================================================================
// 関数プロトタイプ
//===================================================================================================
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void OnTick(struct tm *tick_time, TimeUnits units_changed);
static void OnDraw_PLayDial(Layer *layer, GContext *ctx);
static void OnDraw_PLayHand(Layer *layer, GContext *ctx);
static void OnBattery(BatteryChargeState charge_state);
static void OnBlueTooth(bool connected);

//===================================================================================================
// グローバル定数/変数
//===================================================================================================
//--- ウィンドウ/レイヤー
Window    *gWinMain;
Layer     *gPLayDial;
TextLayer *gTLayDate; char gTLayDateText[32];
TextLayer *gTLayTime; char gTLayTimeText[32];
TextLayer *gTLayBatt; char gTLayBattText[32];
TextLayer *gTLayBlue; char gTLayBlueText[32];
Layer     *gPLayHand;
//--- アナログ時計の描画オブジェクト
#define NUM_CLOCK_TICKS 11
static const struct GPathInfo ANALOG_BG_POINTS[] = {
  { 4, (GPoint []) { { 68,   0}, { 71,   0}, { 71,  12}, { 68,  12} } },
  { 4, (GPoint []) { { 72,   0}, { 75,   0}, { 75,  12}, { 72,  12} } },
  { 4, (GPoint []) { {112,  10}, {114,  12}, {108,  23}, {106,  21} } },
  { 4, (GPoint []) { {132,  47}, {144,  40}, {144,  44}, {135,  49} } },
  { 4, (GPoint []) { {135, 118}, {144, 123}, {144, 126}, {132, 120} } },
  { 4, (GPoint []) { {108, 144}, {114, 154}, {112, 157}, {108, 147} } },
  { 4, (GPoint []) { { 70, 155}, { 73, 155}, { 73, 167}, { 70, 167} } },
  { 4, (GPoint []) { { 32,  10}, { 30,  12}, { 36,  23}, { 36,  21} } },
  { 4, (GPoint []) { { 12,  47}, { -1,  40}, { -1,  44}, {  9,  49} } },
  { 4, (GPoint []) { {  9, 118}, { -1, 123}, { -1, 126}, { 12, 120} } },
  { 4, (GPoint []) { { 36, 144}, { 30, 154}, { 32, 157}, { 38, 147} } }
};
static const GPathInfo HOUR_HAND_POINTS = { 4, (GPoint[]) { {-8, 5}, {0, 10}, {8, 5}, {0, -40} } };
static const GPathInfo MINUTE_HAND_POINTS = { 4, (GPoint[]) { {-6, 7}, {0, 12},{6, 7}, {0, -70} } };
static GPath *gpDialScales[NUM_CLOCK_TICKS];
static GPath *gpHourArrow, *gpMinArrow;
//--- BlueToorhの前回接続状態
static bool gPrevConnect = true;
//===================================================================================================
// アプリメイン
//===================================================================================================
int main(void) {
  //=============
  // アプリ初期化
  //=============
  //--- メインウィンドウ作成とハンドラ登録
  gWinMain = window_create();
  window_set_window_handlers(gWinMain, (WindowHandlers) {.load = main_window_load, .unload = main_window_unload} );
  window_set_background_color(gWinMain, GColorCyan);
  window_stack_push(gWinMain, true);
  //===============
  // イベントループ
  //===============
  app_event_loop();
  //=============
  // アプリ後処理
  //=============
  window_destroy(gWinMain);
}

//===================================================================================================
// メインウィンドウ
//===================================================================================================
//--- ロード
static void main_window_load(Window *window) {
  Layer*     RootLayer = window_get_root_layer(window); //Size(x144,y154), Area(x0,y0)-(x143,y153)
  GRect      RootLayerBounds = layer_get_bounds(RootLayer); //GRect(PosX, PosY, SizeX, SizeY)
  GPoint     RootLayerCenter = grect_center_point(&RootLayerBounds);
  //--- アナログ時計文字盤レイヤー
  gPLayDial = layer_create(RootLayerBounds);
  layer_set_update_proc(gPLayDial, OnDraw_PLayDial);
  for (int Element = 0; Element < NUM_CLOCK_TICKS; Element++) {
    gpDialScales[Element] = gpath_create(&ANALOG_BG_POINTS[Element]);
  }
  //--- バッテリーレイヤー
  gTLayBatt = text_layer_create(GRect(0, 10, 144, 28));
  text_layer_set_background_color(gTLayBatt, GColorClear);
  text_layer_set_text_color(gTLayBatt, GColorMidnightGreen);
  text_layer_set_text_alignment(gTLayBatt, GTextAlignmentCenter);
  text_layer_set_font(gTLayBatt, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  strcpy(gTLayBattText, "Checking..."); text_layer_set_text(gTLayBatt, gTLayBattText);
  //--- 日付レイヤー
  gTLayDate = text_layer_create(GRect(0, 30, 144, 28));
  text_layer_set_background_color(gTLayDate, GColorClear);
  text_layer_set_text_color(gTLayDate, GColorDukeBlue);
  text_layer_set_text_alignment(gTLayDate, GTextAlignmentCenter);
  text_layer_set_font(gTLayDate, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  strcpy(gTLayDateText, "--.-- ---"); text_layer_set_text(gTLayDate, gTLayDateText);
  //--- BlueToorhレイヤー
  gTLayBlue = text_layer_create(GRect(0, 60, 144, 30));
  text_layer_set_background_color(gTLayBlue, GColorClear);
  text_layer_set_text_color(gTLayBlue, GColorBlack);
  text_layer_set_text_alignment(gTLayBlue, GTextAlignmentCenter);
  text_layer_set_font(gTLayBlue, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  strcpy(gTLayBlueText, "Checking..."); text_layer_set_text(gTLayBlue, gTLayBlueText);
  //---デジタル時刻レイヤー
  gTLayTime = text_layer_create(GRect(0, 95, 144, 48));
  text_layer_set_background_color(gTLayTime, GColorClear);
  text_layer_set_text_color(gTLayTime, GColorDukeBlue);
  text_layer_set_text_alignment(gTLayTime, GTextAlignmentCenter);
  text_layer_set_font(gTLayTime, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text(gTLayTime, "--:--");
  strcpy(gTLayTimeText, "--:--"); text_layer_set_text(gTLayTime, gTLayTimeText);
  //--- アナログ時計針レイヤー
  gPLayHand = layer_create(RootLayerBounds);
  layer_set_update_proc(gPLayHand, OnDraw_PLayHand);
  //--- レイヤー追加
  layer_add_child(RootLayer, gPLayDial);
  layer_add_child(RootLayer, text_layer_get_layer(gTLayDate));
  layer_add_child(RootLayer, text_layer_get_layer(gTLayTime));
  layer_add_child(RootLayer, text_layer_get_layer(gTLayBatt));
  layer_add_child(RootLayer, text_layer_get_layer(gTLayBlue));
  layer_add_child(RootLayer, gPLayHand);
  //--- 描画オブジェクト作成
  gpMinArrow  = gpath_create(&MINUTE_HAND_POINTS); gpath_move_to(gpMinArrow, RootLayerCenter);
  gpHourArrow = gpath_create(&HOUR_HAND_POINTS);   gpath_move_to(gpHourArrow, RootLayerCenter);
  //--- 初期表示
  OnTick(pLOCTIME, MINUTE_UNIT);
  OnBattery(battery_state_service_peek());
  OnBlueTooth(bluetooth_connection_service_peek());
  //--- イベントサービス起動
  tick_timer_service_subscribe(MINUTE_UNIT, OnTick);
  battery_state_service_subscribe(OnBattery);
  bluetooth_connection_service_subscribe(OnBlueTooth);
}

//--- アンロード
static void main_window_unload(Window *window) {
  //--- イベントサービス削除
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  tick_timer_service_unsubscribe();
  //--- 描画オブジェクトの削除
  gpath_destroy(gpMinArrow);
  gpath_destroy(gpHourArrow);
  //--- レイヤー削除
  text_layer_destroy(gTLayBlue);
  text_layer_destroy(gTLayBatt);
  text_layer_destroy(gTLayTime);
  text_layer_destroy(gTLayDate);
}

//==============================
// 文字盤更新
//==============================
static void OnDraw_PLayDial(Layer *layer, GContext *ctx) {
  GRect   Bounds = layer_get_bounds(layer);
  GPoint  Center = grect_center_point(&Bounds);
  //---
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 5);
  graphics_draw_circle(ctx, Center, 83);
  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    gpath_draw_filled(ctx, gpDialScales[i]);
  }
}

//==============================
// 時刻更新
//==============================
//--- 時刻更新ハンドラー
static void OnTick(struct tm *tick_time, TimeUnits units_changed) {
  strftime(gTLayDateText, sizeof(gTLayDateText), "%m.%d %a", tick_time); text_layer_set_text(gTLayDate, gTLayDateText);
  strftime(gTLayTimeText, sizeof(gTLayTimeText), ((clock_is_24h_style()) ? "%H:%M" : "%I:%M"), tick_time); text_layer_set_text(gTLayTime, gTLayTimeText);
  layer_mark_dirty(window_get_root_layer(gWinMain));
}
//--- 時計針更新
static void OnDraw_PLayHand(Layer *layer, GContext *ctx) {
  GRect LayerBounds = layer_get_bounds(layer);
  struct tm *LocTime = pLOCTIME;
  int    Hour = LocTime->tm_hour;
  int    Min  = LocTime->tm_min;
  //--- 色のセット
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);
  //--- 時
  gpath_rotate_to(gpHourArrow, (TRIG_MAX_ANGLE * (((Hour % 12) * 6) + (Min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, gpHourArrow);
  gpath_draw_outline(ctx, gpHourArrow);
  //--- 分
  gpath_rotate_to(gpMinArrow, TRIG_MAX_ANGLE * Min / 60);
  gpath_draw_filled(ctx, gpMinArrow);
  gpath_draw_outline(ctx, gpMinArrow);
  //--- ピン
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(LayerBounds.size.w / 2 - 1, LayerBounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

//==============================
// バッテリーイベントハンドラ
//==============================
static void OnBattery(BatteryChargeState charge_state) {
  snprintf(gTLayBattText, sizeof(gTLayBattText), charge_state.is_charging ? "BAT: Charging" : "BAT: %d%%", charge_state.charge_percent);
  text_layer_set_text(gTLayBatt, gTLayBattText);
}

//==============================
// BlueToothイベントハンドラ
//==============================
static void OnBlueTooth(bool connected) {
  strcpy(gTLayBlueText, connected ? "" : "BT:XXX"); text_layer_set_text(gTLayBlue, gTLayBlueText);
  do {
    //--- 途切れた時
    if ((gPrevConnect) && (!connected)) {
      window_set_background_color(gWinMain, GColorOrange);
      for (int VibeCount=1; VibeCount<=5; VibeCount++) {vibes_short_pulse(); psleep(800);}
      break;
    }
    //--- つながった時
    if ((!gPrevConnect) && (connected)) {
      window_set_background_color(gWinMain, GColorCyan);
      vibes_short_pulse();
      break;
    }
  } while (false);
  gPrevConnect = connected;
}

//===================================================================================================
// ライブラリ
//===================================================================================================
