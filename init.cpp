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
        cout << " + " << current << endl;
        players.push_back(current);
    }
    return players;
}

int main(int argc, char* argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        (CMD_PERSO, po::value<string>()->default_value(PERSO_FILE), "initiatives")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    xdo_t *xt = xdo_new(NULL);
    vector<string> players = read_players((char*)vm[CMD_PERSO].as<string>().data());
    vector<Window> players_w = {};
    for(long unsigned int i = 0; i < players.size(); ++i)
    {
        Window w = 0;
        int ret = get_dofus_window(xt, players.at(i).data(), &w);
        if (ret != XDO_SUCCESS)
            continue;
        players_w.push_back(w);
    }
    for(long unsigned int i = 0; i < players_w.size(); ++i)
    {
        int next_i = (i+1) % players_w.size();
        int prev_i = (i-1+players_w.size()) % players_w.size();
        set_property(xt, players_w.at(i), XA_NEXT_P, players_w.at(next_i));
        set_property(xt, players_w.at(i), XA_PREV_P, players_w.at(prev_i));
        cout << "Window : " << players_w.at(prev_i) << " - " << players_w.at(i) << " - " << players_w.at(next_i) << endl;
    }
    cout << "Window : " << players_w.size() << " / " << players.size() << endl;
    xdo_free(xt);
    return 0;
}
