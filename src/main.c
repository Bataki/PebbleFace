#include <pebble.h>
#include "FaClass.h"

static void init();
static void deinit();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void update_time();
static void handle_battery(BatteryChargeState charge_state);
static void handle_bluetooth(bool connected);

//==============================
// グローバル変数
//==============================
//--- メイン
static struct {
  Window *Obj;                          //メインウィンドウ
} sMainWin;
//--- テキスト
TextLayer_class DateText, TimeText, BatteryText, BlueToothText;
//---
static bool PrevConnect = true;
//==============================
// アプリメイン
//==============================
//--- アプリメイン
int main(void) {
  init();
  app_event_loop();
  deinit();
}

//--- アプリ初期化
static void init() {
  //--- メインウィンドウ作成
  sMainWin.Obj = window_create();
  //--- メインウィンドウハンドラ登録
  window_set_window_handlers(sMainWin.Obj, (WindowHandlers) {.load = main_window_load, .unload = main_window_unload} );
  window_set_background_color(sMainWin.Obj, GColorCyan);
  window_stack_push(sMainWin.Obj, true);
  //--- ハンドラ登録
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();
}

//--- アプリ後処理
static void deinit() {
  window_destroy(sMainWin.Obj);
}

//==============================
// メインウィンドウ
//==============================
//--- ロード
static void main_window_load(Window *window) {
  //--- 日付
  TextLayer_Init(&DateText, 32); DateText.Font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD); DateText.BGColor = GColorClear;
  TextLayer_Create(&DateText, 0, 0, 144, 154);
  TextLayer_SetText(&DateText, "----.--.-- ---");
  //--- 時刻
  TextLayer_Init(&TimeText, 32); TimeText.Font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49); TimeText.BGColor = GColorClear;
  TextLayer_Create(&TimeText, 0, 30, 144, 154);
  TextLayer_SetText(&TimeText, "00:00");
  //--- バッテリー
  TextLayer_Init(&BatteryText, 32); BatteryText.Font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD); BatteryText.BGColor = GColorClear;
  TextLayer_Create(&BatteryText, 0, 80, 144, 154);
  TextLayer_SetText(&BatteryText, "Checking...");
  //--- BlueToorh
  TextLayer_Init(&BlueToothText, 32); BlueToothText.Font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK); BlueToothText.BGColor = GColorClear;
  TextLayer_Create(&BlueToothText, 0, 120, 144, 154);
  TextLayer_SetText(&BlueToothText, "Checking...");
  //--- レイヤー追加
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(DateText.Layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(TimeText.Layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(BatteryText.Layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(BlueToothText.Layer));
  //--- 初期表示
  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());
  //--- イベントサービス起動
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);
}

//--- アンロード
static void main_window_unload(Window *window) {
  //--- イベントサービス削除
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  //--- レイヤー削除
  TextLayer_Destroy(&BlueToothText);
  TextLayer_Destroy(&BatteryText);
  TextLayer_Destroy(&TimeText);
  TextLayer_Destroy(&DateText);
}

//==============================
// 時刻更新
//==============================
//--- 時刻更新ハンドラー
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

//--- 時刻更新メイン
static void update_time() {
  //--- 時刻を取得してローカルタイムへ変換する
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  //--- 表示文字列の作成と表示
  char Text[32];
  strftime(Text, sizeof(Text), "%Y.%m.%d %a", tick_time);
  TextLayer_SetText(&DateText, Text);
  strftime(Text, sizeof(Text), ((clock_is_24h_style()) ? "%H:%M" : "%I:%M"), tick_time); //H=24h, I=12h
  TextLayer_SetText(&TimeText, Text);
}

//==============================
// バッテリー
//==============================
static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "BAT:%d%%", charge_state.charge_percent);
  }
  //text_layer_set_text(s_battery_layer, battery_text);
  TextLayer_SetText(&BatteryText, battery_text);
}

//==============================
// BlueTooth
//==============================
static void handle_bluetooth(bool connected) {
  int VibeCount;
  TextLayer_SetText(&BlueToothText, connected ? "" : "BT:XXX");
  layer_mark_dirty(window_get_root_layer(sMainWin.Obj));
  do {
    //--- 途切れた時
    if ((PrevConnect) && (!connected)) {
      window_set_background_color(sMainWin.Obj, GColorOrange);
      layer_mark_dirty(window_get_root_layer(sMainWin.Obj));
      for (VibeCount=1; VibeCount<=5; VibeCount++) {vibes_short_pulse(); psleep(800);}
      break;
    }
    //--- つながった時
    if ((!PrevConnect) && (connected)) {
      window_set_background_color(sMainWin.Obj, GColorCyan);
      vibes_short_pulse(); break;
    }
  } while (false);
  PrevConnect = connected;
}


#if false
/*
#include <pebble.h>
#include "FaClass.h"

static void init();
static void deinit();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void update_time();

//==============================
// グローバル変数
//==============================
//--- メイン
static struct {
  Window *Obj;                          //メインウィンドウ
} sMainWin;
//--- テキスト
TextLayer_class TimeText, BatteryText, BlueToothText;
//--- 背景画像
static struct {
  BitmapLayer  *Layer;                  //背景画像レイヤー
  GBitmap      *BitMap;                 //背景画像ビットマップ
} sBackPic;

//==============================
// アプリメイン
//==============================
//--- アプリメイン
int main(void) {
  init();
  app_event_loop();
  deinit();
}

//--- アプリ初期化
static void init() {
  //--- メインウィンドウ作成
  sMainWin.Obj = window_create();
  //--- メインウィンドウハンドラ登録
  window_set_window_handlers(sMainWin.Obj, (WindowHandlers) {.load = main_window_load, .unload = main_window_unload} );
  window_stack_push(sMainWin.Obj, true);
  //--- 時刻ハンドラ登録
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler); //分毎
  // Make sure the time is displayed from the start
  update_time();
}

//--- アプリ後処理
static void deinit() {
  window_destroy(sMainWin.Obj);
}

//==============================
// メインウィンドウ
//==============================
//--- ロード
static void main_window_load(Window *window) {
  //--- 背景画ロードとルートレイヤーへのセット
  sBackPic.Layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  sBackPic.BitMap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  bitmap_layer_set_bitmap(sBackPic.Layer, sBackPic.BitMap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(sBackPic.Layer));
  //--- 時刻表示テキストレイヤーロードとルートレイヤーへのセット
  TextLayer_Init(&sTime, 16); TextLayer_Create(&sTime, 5, 69, 134, 29); //(5, 52, 139, 50), RESOURCE_ID_FONT_PERFECT_DOS_24
  TextLayer_SetText(&sTime, "00:00:00");
  //sTime.Font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_24)); //text_layer_set_font/FONT_KEY_BITHAM_42_BOLD
  //text_layer_set_font(sTime.Layer, sTime.Font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(sTime.Layer));
}

//--- アンロード
static void main_window_unload(Window *window) {
  fonts_unload_custom_font(sTime.Font);  //Unload GFont
  //text_layer_destroy(sTime.Layer);       //時刻用レイヤーの破棄
  TextLayer_Destroy(&sTime);
  bitmap_layer_destroy(sBackPic.Layer);  //背景画用レイヤーの破棄
  gbitmap_destroy(sBackPic.BitMap);      //背景画の破棄
}

//==============================
// 時刻更新
//==============================
//--- 時刻更新ハンドラー
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

//--- 時刻更新メイン
static void update_time() {
  //--- 時刻を取得してローカルタイムへ変換する
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  //--- 表示文字列の作成と表示
  char Text[16];
  strftime(Text, sizeof(Text), ((clock_is_24h_style()) ? "%H:%M:%S" : "%I:%M:%S"), tick_time); //H=24h, I=12h
  TextLayer_SetText(&sTime, Text);  //text_layer_set_text(sTime.Layer, sTime.TextBuffer);
}
*/
#endif
