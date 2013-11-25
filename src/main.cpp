/*  
* main.cpp
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

#include <cstdlib>
#include "connection.h"
#include "irc.h"

int main(int argc, char **argv) {
    string clhost, response, ret;
    int port;

    // Check args
    if ((argc < 3) || (argc > 4)) {
        cerr << "usage: bot [hostname] [port]" << endl;
        exit (1);
    }
    else {
        clhost = argv[1];
        sscanf (argv[2], "%d", &port);
    }

    // Init connection for specified host:port
    Connection c(clhost);
    c.port = port;

    // Main process name
    prctl(PR_GET_NAME, c.ps_name, 0, 0, 0);
                    
    // Open connection
    if (c.openConnection() == -1) exit (1);

    c.print();

    if (connectToIRC(c) == -1) exit (1);

    cout << ">> receiving data" << endl;
    cout << "-----------------" << endl;

    while (1) {
        response = c.recvData();
        if (response != "empty") {
            if (response.find("376") != string::npos || response.find("422") != string::npos) {
                joinChannel(c);
                nsIdentify(c);
                c.eventList();
                
                break;
            }
        }
        else sleep(1);
    }
    
    c.getBotConf();
    
    while (1) {
        response = c.recvData ();
        
        if (response != "empty") {
            cout << response;
            c.botMode(response);
        }
        else usleep(10000);
    }

    // Close connection
    if (c.closeConnection() == -1) exit (1);

    return 0;
}
