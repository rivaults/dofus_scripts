extern "C" {
    #include <xdo.h>
}
#include <X11/Xlib.h>
#include <iostream>
#include <string>
#include <filesystem>
using namespace std;

const char* const XA_PLAYER = "XA_PLAYER";
const char* const XA_NEXT_P = "XA_NEXT_P";
const char* const XA_PREV_P = "XA_PREV_P";
const char* const XA_TIME = "XA_TIME";

xdo_search_t get_parameter(char *player);
char *get_next_player(const xdo_t *xt, Window w);
char *get_previous_player(const xdo_t *xt, Window w);
char *get_curr_player(const xdo_t *xt, Window w);
filesystem::path get_path(const char* file);
int get_dofus_window(const xdo_t *xt, char* p, Window* res);
Time _get_time(const xdo_t *xt, Window w);
Time get_current_time();