#include <pebble.h>
//===================================================================================================
// 関数プロトタイプ
//===================================================================================================
static void init();
static void deinit();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void update_time();
static void handle_battery(BatteryChargeState charge_state);
static void handle_bluetooth(bool connected);
//===================================================================================================
// グローバル変数
//===================================================================================================
//--- ウィンドウ/レイヤー
Window *gWinMain;
TextLayer *gTLayDate; char gTLayDateText[32];
TextLayer *gTLayTime; char gTLayTimeText[32];
TextLayer *gTLayBatt; char gTLayBattText[32];
TextLayer *gTLayBlue; char gTLayBlueText[32];
//---
static bool PrevConnect = true;
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
  Layer*     RootLayer = window_get_root_layer(window);
  //--- 日付
  gTLayDate = text_layer_create(GRect(0, 0, 144, 154));
  text_layer_set_background_color(gTLayDate, GColorClear);
  text_layer_set_text_color(gTLayDate, GColorBlack);
  text_layer_set_text_alignment(gTLayDate, GTextAlignmentCenter);
  text_layer_set_font(gTLayDate, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  strcpy(gTLayDateText, "----.--.-- ---"); text_layer_set_text(gTLayDate, gTLayDateText);
  //--- 時刻
  gTLayTime = text_layer_create(GRect(0, 30, 144, 154));
  text_layer_set_background_color(gTLayTime, GColorClear);
  text_layer_set_text_color(gTLayTime, GColorBlack);
  text_layer_set_text_alignment(gTLayTime, GTextAlignmentCenter);
  text_layer_set_font(gTLayTime, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text(gTLayTime, "--:--");
  strcpy(gTLayTimeText, "--:--"); text_layer_set_text(gTLayTime, gTLayTimeText);
  //--- バッテリー
  gTLayBatt = text_layer_create(GRect(0, 80, 144, 154));
  text_layer_set_background_color(gTLayBatt, GColorClear);
  text_layer_set_text_color(gTLayBatt, GColorBlack);
  text_layer_set_text_alignment(gTLayBatt, GTextAlignmentCenter);
  text_layer_set_font(gTLayBatt, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  strcpy(gTLayBattText, "Checking..."); text_layer_set_text(gTLayBatt, gTLayBattText);
  //--- BlueToorh
  gTLayBlue = text_layer_create(GRect(0, 120, 144, 154));
  text_layer_set_background_color(gTLayBlue, GColorClear);
  text_layer_set_text_color(gTLayBlue, GColorBlack);
  text_layer_set_text_alignment(gTLayBlue, GTextAlignmentCenter);
  text_layer_set_font(gTLayBlue, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  strcpy(gTLayBlueText, "Checking..."); text_layer_set_text(gTLayBlue, gTLayBlueText);
  //--- レイヤー追加
  layer_add_child(RootLayer, text_layer_get_layer(gTLayDate));
  layer_add_child(RootLayer, text_layer_get_layer(gTLayTime));
  layer_add_child(RootLayer, text_layer_get_layer(gTLayBatt));
  layer_add_child(RootLayer, text_layer_get_layer(gTLayBlue));
  //--- 初期表示
  update_time();
  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());
  //--- イベントサービス起動
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);
}

//--- アンロード
static void main_window_unload(Window *window) {
  //--- イベントサービス削除
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  tick_timer_service_unsubscribe();
  //--- レイヤー削除
  text_layer_destroy(gTLayBlue);
  text_layer_destroy(gTLayBatt);
  text_layer_destroy(gTLayTime);
  text_layer_destroy(gTLayDate);
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
  strftime(gTLayDateText, sizeof(gTLayDateText), "%Y.%m.%d %a", tick_time); text_layer_set_text(gTLayDate, gTLayDateText);
  strftime(gTLayTimeText, sizeof(gTLayTimeText), ((clock_is_24h_style()) ? "%H:%M" : "%I:%M"), tick_time); text_layer_set_text(gTLayTime, gTLayTimeText);
}

//==============================
// バッテリーイベントハンドラ
//==============================
static void handle_battery(BatteryChargeState charge_state) {
  snprintf(gTLayBattText, sizeof(gTLayBattText), charge_state.is_charging ? "BAT:Charging" : "BAT: %d%%", charge_state.charge_percent);
  text_layer_set_text(gTLayBatt, gTLayBattText);
}

//==============================
// BlueToothイベントハンドラ
//==============================
static void handle_bluetooth(bool connected) {
  int VibeCount;
  strcpy(gTLayBlueText, connected ? "" : "BT:XXX"); text_layer_set_text(gTLayBlue, gTLayBlueText);
  do {
    //--- 途切れた時
    if ((PrevConnect) && (!connected)) {
      window_set_background_color(gWinMain, GColorOrange);
      for (VibeCount=1; VibeCount<=5; VibeCount++) {vibes_short_pulse(); psleep(800);}
      break;
    }
    //--- つながった時
    if ((!PrevConnect) && (connected)) {
      window_set_background_color(gWinMain, GColorCyan);
      vibes_short_pulse(); break;
    }
  } while (false);
  PrevConnect = connected;
}
