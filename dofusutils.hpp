extern "C" {
    #include <xdo.h>
}
#include <X11/Xlib.h>
#include <iostream>
#include <string>
#include <filesystem>
using namespace std;

const char* const XA_NEXT_P = "XA_NEXT_P";
const char* const XA_PREV_P = "XA_PREV_P";
const char* const XA_TIME = "XA_TIME";
const char* const XA_FISHER_W = "XA_FISHER_W";
const char* const XA_SADIDA_W = "XA_SADIDA_W";
const char* const CMD_FISHER_CYCLE = "fisher-cycle";
const char* const CMD_FISHER = "fisher";
const char* const CMD_FISHER_ENABLE = "fisher.enable";
const char* const CMD_FISHER_DELAY = "fisher.delay";
const char* const CMD_FISHER_NAME = "fisher.name";
const char* const CMD_SADIDA_CYCLE = "sadida-cycle";
const char* const CMD_SADIDA = "sadida";
const char* const CMD_SADIDA_ENABLE = "sadida.enable";
const char* const CMD_SADIDA_DELAY = "sadida.delay";
const char* const CMD_SADIDA_NAME = "sadida.name";

xdo_search_t get_parameter(char *player);
Window get_property_window(const xdo_t *xt, Window w, const char* property_name);
void set_property(const xdo_t* xt, Window w, const char* property_name, int property_val);
filesystem::path get_path(initializer_list<const char*> dirs);
int get_dofus_window(const xdo_t *xt, char* p, Window* res);
Time get_time(const xdo_t *xt, Window w);
Time get_current_time();