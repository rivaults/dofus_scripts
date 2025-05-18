#include <vector>
#include <fstream>
#include "dofusutils.hpp"

void set_window_curr_player(const xdo_t *xt, Window w, char *player)
{
    xdo_set_window_property(xt, w, XA_PLAYER, player);
}

const char* const PERSO_FILE = "persos";

vector<string> read_players(){
    ifstream players_file (get_path(PERSO_FILE).string().data());
    string current;
    vector<string> players;
    while(getline(players_file, current))
    {
        cout << " + " << current << endl;
        players.push_back(current);
    }
    return players;
}

int main() {
    xdo_t *xt = xdo_new(NULL);
    vector<string> players = read_players();
    for(long unsigned int i = 0; i < players.size(); ++i)
    {
        Window w = 0;
        int ret = get_dofus_window(xt, players.at(i).data(), &w);
        if (ret != XDO_SUCCESS)
            continue;
        set_window_curr_player(xt, w, players.at(i).data());
        int next_i = (i+1) % players.size();
        int prev_i = (i-1+players.size()) % players.size();
        xdo_set_window_property(xt, w, XA_NEXT_P, players.at(next_i).data());
        xdo_set_window_property(xt, w, XA_PREV_P, players.at(prev_i).data());
        cout << "Window (" << w << ") " << players.at(i).data() << " : " << players.at(prev_i).data() << " - " << players.at(next_i).data() << endl;
    }
    xdo_free(xt);
    return 0;
}