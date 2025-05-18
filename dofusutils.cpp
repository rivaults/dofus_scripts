#ifndef DOFUSUTILS_HPP
#define DOFUSUTILS_HPP
#include "dofusutils.hpp"

xdo_search_t get_parameter(char *player)
{
    return {
            .winclassname = "dofus retro",
            .winname = player,
            .max_depth = -1,
            .only_visible = 1,
            .require = xdo_search::SEARCH_ALL,
            .searchmask = SEARCH_CLASSNAME | SEARCH_NAME | SEARCH_ONLYVISIBLE,
    };
}

char* _get_player(const xdo_t *xt, Window w, const char* property)
{
    unsigned char *res;
    long nitem;
    Atom type;
    int size;
    xdo_get_window_property(xt, w, property, &res, &nitem, &type, &size);
    if (size != 8 || nitem == 0)
        return NULL;
    return (char*)res;
}

Time get_current_time(){
    struct timespec up;
    clock_gettime(CLOCK_BOOTTIME, &up);
    return up.tv_sec * 1000 + up.tv_nsec / 1000000;
}

Time _get_time(const xdo_t *xt, Window w)
{
    unsigned char *res;
    Atom type;
    unsigned long nitem;
    int format;
    unsigned long bytes_after;
    int status = XGetWindowProperty(xt->xdpy, w, XInternAtom(xt->xdpy, XA_TIME, False), 0, (~0L),
                              False, AnyPropertyType, &type,
                              &format, &nitem, &bytes_after,
                              &res);
    if (status != Success || nitem == 0)
        return 0;
    return *((Time*) res);
}

char *get_previous_player(const xdo_t *xt, Window w)
{
    return _get_player(xt, w, XA_PREV_P);
}

char *get_next_player(const xdo_t *xt, Window w)
{
    return _get_player(xt, w, XA_NEXT_P);
}

char *get_curr_player(const xdo_t *xt, Window w)
{
    return _get_player(xt, w, XA_PLAYER);
}

filesystem::path _getExecutablePath()
{
    return filesystem::canonical("/proc/self/exe").parent_path();
}

filesystem::path get_path(const char* file)
{
    filesystem::path dir = _getExecutablePath();
    auto res = dir / file;
    return res;
}

int get_dofus_window(const xdo_t *xt, char* p, Window* res){
    Window *results = NULL;
    unsigned int results_len;
    xdo_search_t params = get_parameter(p);
    xdo_search_windows(xt, &params, &results, &results_len);
    if (results_len != 1){
        return XDO_ERROR;
    }
    *res = results[0];
    return XDO_SUCCESS;
}
#endif