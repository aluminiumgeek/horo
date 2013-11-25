/*  
* irc.cpp
* Copyright (C) 2011 Mikhail Mezyakov <mihail265@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>
#include <algorithm>
#include "irc.h"

#ifndef CONFIG_FILE
#define CONFIG_FILE "config/config"
#endif

string channels, nspwd;

/*void checkForPing(const Connection & c) {
    string stream = c.recvData(), response = "PING ";
    //string::iterator i1;
    size_t i1;

    cout << stream << endl;

    //i1 = search(stream.begin(), stream.end(), response.begin(), response.end());
    i1 = stream.find('PING ');
    if (i1 != string::npos) {
        string server = stream.substr(i1+7);
        response = "PONG " + server;
        c.sendData(response);
        cout << response << endl;
    }
}*/

int connectToIRC(const Connection & c) {
    // Read config
    ifstream config(CONFIG_FILE);
    string str, nick, email, ident, realname;

    while (config >> str) {
        if (str[0] == '#') {
            getline(config, str);
        }
        else if (str == "NICK:") {
            config >> str;
            nick = str;
        } else if (str == "IDENT:") {
            config >> str;
            ident = str;
        } else if (str == "REALNAME:") {
            config >> str;
            realname = str;
        } else if (str == "CHANNELS:") {
            config >> str;
            channels = str;
        } else if (str == "NSPWD:") {
            config >> str;
            nspwd = str;
        } /*else { // Config error
            cerr << "connectToIRC(): error in the config file" << endl;
            return -1;
        }*/
    }

    c.sendData("USER " + ident + " horo " + c.hostname + " :" + realname);

    // checkForPing(c);

    c.sendData("NICK " + nick);

    return 0;
}

int joinChannel(const Connection & c) {
    cout << "Joining channels";
    c.sendData("JOIN " + channels);
    return 0;
}

int nsIdentify(const Connection & c) {
    c.sendData("NICKSERV identify " + nspwd);
    return 0;
}
