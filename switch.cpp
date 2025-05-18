extern "C" {
    #include <xdo.h>
}
#include "dofusutils.hpp"
#include <iterator>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#define LeftButton 1

const char* const CMD_KEY = "key";
const char* const CMD_MOUSE = "mouse";
const char* const CMD_PREV = "prev";
const char* const CMD_NEXT = "next";
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

charcodemap_t* _get_charmapcode(xdo_t* xt, const char* key){
    KeySym symbol = XStringToKeysym(key);
    for (int i = 0; i < xt->charcodes_len; i++) {
        if (xt->charcodes[i].symbol == symbol) {
            return &(xt->charcodes[i]);
        }
    }
    return NULL;
}

void send_key_event(xdo_t* xt, Window window, const char* key){
    XKeyEvent xk;
    xk.display = xt->xdpy;
    xk.subwindow = None;
    xk.time = get_current_time();
    xk.same_screen = True;
    xk.x = xk.x_root = 57;
    xk.y = xk.y_root = 111;
    xk.window = window;
    charcodemap_t* charcode = _get_charmapcode(xt, key);
    xk.keycode = charcode->code;
    xk.state = charcode->modmask | (charcode->group << 13);
    xk.type = KeyPress;
    XSendEvent(xt->xdpy, xk.window, True, KeyPressMask, (XEvent *)&xk);
    XFlush(xt->xdpy);
    usleep(15000);
    xk.type = KeyRelease;
    XSendEvent(xt->xdpy, xk.window, True, KeyReleaseMask, (XEvent *)&xk);
    XFlush(xt->xdpy);
    usleep(15000);
}

int switch_player(xdo_t* xt, Window window, char* player){
    Window w = 0;
    int d_ret = get_dofus_window(xt, player, &w);
    if (d_ret != XDO_SUCCESS)
        return d_ret;
    int a_ret = xdo_activate_window(xt, w);
    if (a_ret) {
        cerr << "xdo_activate_window on window:" << w << "reported an error\n";
        return d_ret;
    }
    xdo_wait_for_window_active(xt, w, 1);
    usleep(15000);
    return XDO_SUCCESS;
}

bool _compare_timestamp(xdo_t* xt, Window w, int delay){
    return _get_time(xt, w)+delay < get_current_time();
}

void update_timestamp(xdo_t* xt, Window w){
    Time t = get_current_time();
    XChangeProperty(xt->xdpy, w, 
        XInternAtom(xt->xdpy, XA_TIME, False), 
        XInternAtom(xt->xdpy, "INTEGER", False), 32, 
        PropModeReplace, (const unsigned char*)&t, 2);
}

void update_property(xdo_t* xt, Window w_curr, Window w_next, int not_in_cycle){
    char* p = get_next_player(xt, w_curr);
    xdo_set_window_property(xt, w_next, XA_NEXT_P, p);
    xdo_set_window_property(xt, w_next, XA_PREV_P, p);
    update_timestamp(xt, w_next);
}

int main(int argc, char* argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        (CMD_KEY, po::value<string>(), "send key sequence")
        (CMD_MOUSE, "send click event")
        (CMD_PREV, "switch to previous player")
        (CMD_NEXT, "switch to next player")
        (CMD_FISHER, "switch to the fisher")
        (CMD_FISHER_NAME, po::value<string>(), "fisherman name")
        (CMD_FISHER_CYCLE, "add the fisher in the cycle")
        (CMD_FISHER_ENABLE, po::value<bool>(), "enable the switch via conf")
        (CMD_FISHER_DELAY, po::value<int>(), "delay to switch")
        (CMD_SADIDA, "switch to the sadida")
        (CMD_SADIDA_NAME, po::value<string>(), "sadida name")
        (CMD_SADIDA_CYCLE, "add the sadida in the cycle")
        (CMD_SADIDA_ENABLE, po::value<bool>(), "enable the switch via conf")
        (CMD_SADIDA_DELAY, po::value<int>(), "delay to switch")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    po::store(po::parse_config_file(get_path("settings.ini").string().data(),desc), vm);
    po::notify(vm);
    
    xdo_t *xt = xdo_new(NULL);
    Window w_curr;
    xdo_get_focused_window(xt, &w_curr);
    if (vm.count(CMD_KEY)){
        send_key_event(xt, w_curr, vm[CMD_KEY].as<string>().data());
    }
    if (vm.count(CMD_MOUSE)){
        xdo_move_mouse_relative(xt, 0, 1);
        xdo_click_window(xt, w_curr, LeftButton);
        xdo_move_mouse_relative(xt, 0, -1);
    }
    char *player = (char*) vm[CMD_SADIDA_NAME].as<string>().data();
    Window w_sadida = 0;
    get_dofus_window(xt, player, &w_sadida);
    Window w_fish = 0;
    get_dofus_window(xt, ((char*)vm[CMD_FISHER_NAME].as<string>().data()), &w_fish);
    bool timeout_sadida = w_sadida != 0 && vm.count(CMD_SADIDA_CYCLE) && _compare_timestamp(xt, w_sadida, vm[CMD_SADIDA_DELAY].as<int>());
    bool timeout_fish = w_fish != 0 && vm.count(CMD_FISHER_CYCLE) && _compare_timestamp(xt, w_fish, vm[CMD_FISHER_DELAY].as<int>());
    if ((w_fish != 0 && w_curr == w_fish) || (w_sadida != 0 && w_curr == w_sadida)){
        update_timestamp(xt, w_curr);
    }
    if (w_sadida != 0 && (vm.count(CMD_SADIDA) || timeout_sadida) && w_sadida != w_curr && vm[CMD_SADIDA_ENABLE].as<bool>()){
        update_property(xt, w_curr, w_sadida, vm.count(CMD_SADIDA));
    }
    else if (w_fish != 0 && (vm.count(CMD_FISHER) || timeout_fish) && w_fish != w_curr && vm[CMD_FISHER_ENABLE].as<bool>()){
        update_property(xt, w_curr, w_fish, vm.count(CMD_FISHER));
        player = (char*)vm[CMD_FISHER_NAME].as<string>().data();
    }
    else if (vm.count(CMD_PREV)){
        player = get_previous_player(xt, w_curr);
    }else{
        player = get_next_player(xt, w_curr);
    }
    switch_player(xt, w_curr, player);
    xdo_free(xt);
    return 0;
}
