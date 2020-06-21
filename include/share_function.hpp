#pragma once
#include "citro2d.h"
#define STACKSIZE (64 * 1024)

extern bool s_mcu_success;

extern bool s_debug_slow;
extern bool s_disabled_enter_afk_mode;
extern bool s_system_setting_menu_show;

extern u8 s_wifi_signal;
extern u8 s_battery_level;
extern u8 s_battery_charge;
extern u8* wifi_state;
extern u8* wifi_state_internet_sample;

extern int s_afk_time;
extern int s_afk_lcd_brightness;
extern int s_num_of_app_start;
extern int s_fps_show;
extern int s_free_ram;
extern int s_free_linear_ram;
extern int s_fps;
extern int s_lcd_brightness;
extern int s_time_to_enter_afk;
extern int s_current_app_ver;
extern int s_current_gas_ver;
extern double s_frame_time;
extern double s_scroll_speed;

extern char s_status[100];
extern char s_swkb_input_text[8192];
extern std::string s_clipboards[15];
extern std::string s_bot_button_string[2];
extern std::string s_circle_string;
extern std::string s_battery_level_string;
extern std::string s_setting[32];
extern std::string s_success;
extern std::string s_error;
extern std::string s_spt_ver;
extern std::string s_gtr_ver;
extern std::string s_imv_ver;
extern std::string s_line_ver;
extern std::string s_cam_ver;
extern std::string s_mic_ver;
extern std::string s_mup_ver;
extern std::string s_app_ver;
extern std::string s_httpc_user_agent;

extern C2D_Image Wifi_icon_image[9];
extern C2D_Image Battery_level_icon_image[21];
extern C2D_Image Battery_charge_icon_image[1];
extern C2D_Image Square_image[1];

extern C2D_ImageTint texture_tint, dammy_tint, black_or_white_tint, white_or_black_tint, white_tint, weak_white_tint, red_tint, weak_red_tint, aqua_tint, weak_aqua_tint, yellow_tint, weak_yellow_tint, blue_tint, weak_blue_tint, black_tint, weak_black_tint;

extern TickCounter s_tcount_frame_time;
extern SwkbdState s_swkb;
extern SwkbdLearningData s_swkb_learn_data;
extern SwkbdDictWord s_swkb_words[8];
extern SwkbdButton s_swkb_press_button;
