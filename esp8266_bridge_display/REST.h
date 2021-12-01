#pragma once

#include "A_config.h"

void restServerRouting();
void rest_get_host();
void rest_get_machine_count();
void rest_get_machine();
void rest_post_machine();
void rest_get_machine_rssi();
void rest_post_fire();

void rest_get_all_functions();
void rest_post_function();
void rest_post_blink();
void rest_post_reboot();
void rest_post_change_id();
void rest_post_query_rssi();
void rest_upload_handler();
void rest_get_config();
void rest_post_save_config();

void rest_post_set_relaiscounter();


#ifdef ETH_ENABLE
void rest_bt_up();
void rest_bt_down();
void rest_bt_click();
void rest_bt_home();
#endif
