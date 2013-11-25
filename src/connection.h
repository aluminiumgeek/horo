/*  
* connection.h
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

#ifndef __CONNECTION_H
#define __CONNECTION_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <fstream>
#include <sstream>

#include <dirent.h>
#include <string>
#include <iostream>
#include <sys/wait.h>
#include <stdlib.h>
#include <iconv.h>
#include <pcre.h>
#include <sys/prctl.h>
#include <vector>

#include <Python.h>

#define MAX_STR 1000

#ifndef CONFIG_FILE
#define CONFIG_FILE "config/config"
#endif

#define COMMANDS "[say %echo %ls]"
#define UTF8_DETECT_REGEXP "^([\x09\x0A\x0D\x20-\x7E]|[\xC2-\xDF][\x80-\xBF]|\xE0[\xA0-\xBF][\x80-\xBF]|[\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}|\xED[\x80-\x9F][\x80-\xBF]|\xF0[\x90-\xBF][\x80-\xBF]{2}|[\xF1-\xF3][\x80-\xBF]{3}|\xF4[\x80-\x8F][\x80-\xBF]{2})*$"

using namespace std;

class Connection {

public:
    Connection(string str);
    string hostname;
    string ip;
    unsigned short port;

    int openConnection();
    int closeConnection();
    bool isConnected() {
        return connected;
    };
    int sendData(string) const;
    string recvData();
    
    int botMode(string);
    
    string replaceAll(string&, const string&, const string&) const;
    
    string modules() const;
    string urlhandler(string&, bool, bool) const;
    
    int getBotConf();
    
    string str, NICK, OWNER, WDETECT, WCHANNELS, CODEPAGE, FAKEPONG, FAKEPONGTIME;
    
    string codepage(string&, const string&, const string&) const;
    string dist;
    
    int coreModules();
    bool isCore;
    int eventList();
    vector<string> events;
    
    bool isFakePong;
    int fakePong(string);
    
    int createFork(string&, string&, string&, string&, string&, vector<string>);
    char ps_name[16];

    void print();

private:
    mutable fd_set set;
    bool connected;
    mutable int sockfd;
    struct sockaddr_in dest_addr;
    struct hostent *host;
};

#endif
