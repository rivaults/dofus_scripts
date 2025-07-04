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

charcodemap_t* _get_charmapcode(const xdo_t* xt, const char* key){
    KeySym symbol = XStringToKeysym(key);
    for (int i = 0; i < xt->charcodes_len; i++) {
        if (xt->charcodes[i].symbol == symbol) {
            return &(xt->charcodes[i]);
        }
    }
    return NULL;
}

void send_key_event(const xdo_t* xt, Window window, const char* key){
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

int switch_player(const xdo_t* xt, Window next_w_player){
    int a_ret = xdo_activate_window(xt, next_w_player);
    if (a_ret) {
        cerr << "xdo_activate_window on window:" << next_w_player << "reported an error" << endl;
        return XDO_ERROR;
    }
    xdo_wait_for_window_active(xt, next_w_player, 1);
    usleep(15000);
    return XDO_SUCCESS;
}

bool compare_timestamp(const xdo_t* xt, Window w, int delay){
    return get_time(xt, w)+delay < get_current_time();
}

void update_timestamp(const xdo_t* xt, Window w){
    Time t = get_current_time();
    XChangeProperty(xt->xdpy, w, 
        XInternAtom(xt->xdpy, XA_TIME, False), 
        XInternAtom(xt->xdpy, "INTEGER", False), 32, 
        PropModeReplace, (const unsigned char*)&t, 2);
}

void update_property(const xdo_t* xt, Window w_curr, Window w_next, int not_in_cycle){
    Window n_w = get_property_window(xt, w_curr, XA_NEXT_P);
    Window p_w = get_property_window(xt, w_curr, XA_PREV_P);
    set_property(xt, w_next, XA_NEXT_P, n_w);
    set_property(xt, w_next, XA_PREV_P, (not_in_cycle) ? p_w : n_w);
    update_timestamp(xt, w_next);
}

Window get_next_window(const xdo_t* xt, po::variables_map vm, Window w_curr){
    Window w_sadida = get_property_window(xt, w_curr, XA_SADIDA_W);
    Window w_fish = get_property_window(xt, w_curr, XA_FISHER_W);
    Window res;
    bool timeout_sadida = w_sadida != 0 && vm.count(CMD_SADIDA_CYCLE) && compare_timestamp(xt, w_sadida, vm[CMD_SADIDA_DELAY].as<int>());
    bool timeout_fish = w_fish != 0 && vm.count(CMD_FISHER_CYCLE) && compare_timestamp(xt, w_fish, vm[CMD_FISHER_DELAY].as<int>());
    if ((w_fish != 0 && w_curr == w_fish) || (w_sadida != 0 && w_curr == w_sadida)){
        update_timestamp(xt, w_curr);
    }
    if (w_sadida != 0 && (vm.count(CMD_SADIDA) || timeout_sadida) && w_sadida != w_curr && vm[CMD_SADIDA_ENABLE].as<bool>()){
        update_property(xt, w_curr, w_sadida, vm.count(CMD_SADIDA));
        res = w_sadida;
    }
    else if (w_fish != 0 && (vm.count(CMD_FISHER) || timeout_fish) && w_fish != w_curr && vm[CMD_FISHER_ENABLE].as<bool>()){
        update_property(xt, w_curr, w_fish, vm.count(CMD_FISHER));
        res = w_fish;
    }
    else if (vm.count(CMD_PREV)){
        res = get_property_window(xt, w_curr, XA_PREV_P);
    }else{
        res = get_property_window(xt, w_curr, XA_NEXT_P);
    }
    return res;
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
    po::store(po::parse_config_file(get_path({"settings.ini"}).string().data(),desc), vm);
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
        xdo_click_window(xt, w_curr, LeftButton);
    }
    switch_player(xt, get_next_window(xt, vm, w_curr));
    xdo_free(xt);
    return 0;
}
