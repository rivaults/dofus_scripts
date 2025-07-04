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

Time get_current_time(){
    struct timespec up;
    clock_gettime(CLOCK_BOOTTIME, &up);
    return up.tv_sec * 1000 + up.tv_nsec / 1000000;
}

void set_property(const xdo_t* xt, Window w, const char* property_name, int property_val){
    XChangeProperty(xt->xdpy, w, 
        XInternAtom(xt->xdpy, property_name, False), 
        XInternAtom(xt->xdpy, "INTEGER", False), 32, 
        PropModeReplace, (const unsigned char*)&property_val, 2);
}

int _get_property(const xdo_t *xt, Window w, const char* property, unsigned char** res){
    Atom type;
    unsigned long nitem;
    int format;
    unsigned long bytes_after;
    int status = XGetWindowProperty(xt->xdpy, w, XInternAtom(xt->xdpy, (char*)property, False), 0, (~0L),
                              False, AnyPropertyType, &type,
                              &format, &nitem, &bytes_after,
                              res);
    if (status != Success || nitem == 0)
    {
        cerr << "_get_property on window: " << w << " reported an error : " << status << " items: " << nitem << endl;
        return XDO_ERROR;
    }
    return XDO_SUCCESS;
}

Time get_time(const xdo_t *xt, Window w)
{
    Time* res;
    int ret = _get_property(xt, w, XA_TIME, (unsigned char**)&res);
    if (ret != XDO_SUCCESS){
        cerr << "get_time on window: " << w << " reported an error " << endl;
        return 0;
    }
    return *res;
}

Window get_property_window(const xdo_t *xt, Window w, const char* property_name)
{
    Window* res;
    int ret = _get_property(xt, w, property_name, (unsigned char**)&res);
    if (ret != XDO_SUCCESS){
        cerr << "get_property_window on window: " << w << " reported an error" << endl;
        return 0;
    }
    return *res;
}

filesystem::path _getExecutablePath()
{
    return filesystem::canonical("/proc/self/exe").parent_path();
}

filesystem::path get_path(initializer_list<const char*> dirs)
{
    filesystem::path res = _getExecutablePath();
    for(auto d : dirs)
        res = res / d;
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
