#include <vector>
#include <fstream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include "dofusutils.hpp"

const char* const CMD_PERSO = "file";
const char* INITIATIVES_DIR = "initiatives";
const char* const PERSO_FILE = "perso";

vector<string> read_players(char* f_perso){
    ifstream players_file (get_path({INITIATIVES_DIR, f_perso}).string().data());
    string current;
    vector<string> players;
    while(getline(players_file, current))
    {
        players.push_back(current);
    }
    return players;
}

vector<Window> get_players_windows(const xdo_t* xt, vector<string> players){
    vector<Window> players_w = {};
    for(long unsigned int i = 0; i < players.size(); ++i)
    {
        Window w = 0;
        int ret = get_dofus_window(xt, players.at(i).data(), &w);
        if (ret != XDO_SUCCESS)
            continue;
        players_w.push_back(w);
        cout << " + " << players.at(i) << " : " << w << endl;
    }
    return players_w;
}

int main(int argc, char* argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        (CMD_PERSO, po::value<string>()->default_value(PERSO_FILE), "initiatives")
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
    vector<string> players = read_players((char*)vm[CMD_PERSO].as<string>().data());
    vector<Window> players_w = get_players_windows(xt, players);
    Window w_fish = 0;
    get_dofus_window(xt, (char*)vm[CMD_FISHER_NAME].as<string>().data(), &w_fish);
    Window w_sadida = 0;
    get_dofus_window(xt, (char*)vm[CMD_SADIDA_NAME].as<string>().data(), &w_sadida);
    if (w_fish != 0){
        set_property(xt, w_fish, XA_FISHER_W, w_fish);
        set_property(xt, w_fish, XA_SADIDA_W, w_sadida);
        cout << " + fisher : " << w_fish << endl;
    }
    if (w_sadida != 0){
        set_property(xt, w_sadida, XA_FISHER_W, w_fish);
        set_property(xt, w_sadida, XA_SADIDA_W, w_sadida);
        cout << " + sadida : " << w_fish << endl;
    }
    for(long unsigned int i = 0; i < players_w.size(); ++i)
    {
        int next_i = (i+1) % players_w.size();
        int prev_i = (i-1+players_w.size()) % players_w.size();
        set_property(xt, players_w.at(i), XA_NEXT_P, players_w.at(next_i));
        set_property(xt, players_w.at(i), XA_PREV_P, players_w.at(prev_i));
        if (vm[CMD_FISHER_ENABLE].as<bool>() && w_fish != 0)
            set_property(xt, players_w.at(i), XA_FISHER_W, w_fish);
        if (vm[CMD_SADIDA_ENABLE].as<bool>() && w_sadida != 0)
            set_property(xt, players_w.at(i), XA_SADIDA_W, w_sadida);
        cout << "Window : " << players_w.at(prev_i) << " - " << players_w.at(i) << " - " << players_w.at(next_i) << endl;
    }
    cout << "Window : " << players_w.size() << " / " << players.size() << endl;
    xdo_free(xt);
    return 0;
}
