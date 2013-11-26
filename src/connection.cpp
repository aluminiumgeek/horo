/*  
* connection.cpp
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

#include "connection.h"

Connection::Connection(string str) {
    hostname = str;
    ip = "127.0.0.1";
    port = 6667;
    connected = false;
}

void Connection::print() {
    cout << endl << "Hostname  : " << hostname << endl;
    cout << "IP        : " << ip << endl;
    cout << "Port      : " << port << endl;
    cout << "Connected : " << connected << endl << endl;
}

int Connection::openConnection() {
    // Socket initialisation
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Initialisation error
    if (sockfd == -1) {
        cerr << "openConnection(): socket failure" << endl;
        return -1;
    }

    // Hostname -> IP
    if ((host = gethostbyname(hostname.c_str())) == NULL) {
        cerr << "openConnection(): hostname failure" << endl;
        return -1;
    }
    ip = inet_ntoa(*((struct in_addr *)host->h_addr));

    // Connection parameters
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    memset(&(dest_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1) {
        cerr << "openConnection(): connection failure" << endl;
        return -1;
    }

    connected = true;
    cout << ">> connection established" << endl;
    return 0;
}

int Connection::closeConnection() {
    connected = false;

    if (close(sockfd) == -1) {
        cerr << "closeConnection(): there was a problem disconnecting" << endl;
        return -1;
    }

    cout << ">> disconnected successfully" << endl;
    return 0;
}

int Connection::sendData(string str) const {
    string send_str;
    
    // UTF-8 detector
    const char *error;
    int error_off, rc, vect[100];
    pcre *utf8_re;
    pcre_extra *utf8_pe;
    utf8_re = pcre_compile(UTF8_DETECT_REGEXP, PCRE_CASELESS, &error, &error_off, NULL);
    utf8_pe = pcre_study(utf8_re, 0, &error);
    rc = pcre_exec(utf8_re, utf8_pe, str.c_str(), str.size(), 0, 0, vect, sizeof(vect)/sizeof(vect[0]));
    if (rc > 0 && CODEPAGE != "utf-8") str = codepage(str, CODEPAGE, "utf-8");
    
    //if (CODEPAGE != "utf-8") str = codepage(str, CODEPAGE, "utf-8");
    if (str.find("PRIVMSG #") != string::npos && dist != "") {
        int a;
        a = str.find(":", str.find("PRIVSMG #")+9);
        str.insert(a+1, dist + ": ");
    }
    cout << "<< " << str << endl;
    
    send_str = str + "\r\n\r\n";
    
    if (send(sockfd, send_str.c_str(), send_str.size(), 0) == -1) {
        cerr << "sendData(): there was a problem sending data" << endl;
        return -1;
    }

    return 0;
}

string Connection::recvData() {
    struct timeval timeout;
    string strn("");
    char str[MAX_STR];
    int bytes;
    int count;

    FD_ZERO(&set);
    FD_SET(sockfd, &set);

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    count = 0;

    while (select(sockfd + 1, &set, NULL, NULL, &timeout)) {
        if (count > 350) break;
        bytes = read(sockfd, str, MAX_STR -1);
        str [bytes] = '\0'; // is array over?

        strn += str;
        count += 1;
    }

    if (strn != "") {
        if (strn.find(" > ") != string::npos) {
            int a;
            a = strn.find(" > ");
            dist = strn.substr(a+3);
            dist = replaceAll(dist, "\n", "");
            dist = replaceAll(dist, "\r", "");
            strn = strn.substr(0, a);
        }
        else dist = "";
        return strn;
    }
    else return "empty";
    strn = "";
}

// BotMode
int Connection::getBotConf() {
    isCore = false;
    isFakePong = false;
    ifstream config(CONFIG_FILE);
    while (config >> str) {
        if (str[0] == '#')
            getline(config, str);
        else if (str == "NICK:") {
            config >> str;
            NICK = str;
        }
        else if (str == "OWNER:") {
            config >> str;
            OWNER = str;
        }
        else if (str == "WDETECT:") {
            config >> str;
            WDETECT = str;
        }
        else if (str == "WCHANNELS:") {
            config >> str;
            WCHANNELS = str;
        }
        else if (str == "CODEPAGE:") {
            config >> str;
            CODEPAGE = str;
        }
        else if (str == "FAKEPONG:") {
            config >> str;
            FAKEPONG = str;
        }
        else if (str == "FAKEPONGTIME:") {
            config >> str;
            FAKEPONGTIME = str;
        }
    }
    return 0;
}
int Connection::botMode(string s) {
    int pos, pos1, pos2;
    string CHAN, USERQ, smsg;
    //if (CODEPAGE != "utf-8" && sizeof(s)>4) s = codepage(s, "utf-8", CODEPAGE);
    
    // PING-answer
    string sp, server;
    sp = s.substr(0,4);
    if (sp == "PING") {
        server = s.substr(6);
        sendData("PONG " + server);
        if (FAKEPONG == "1" && !isFakePong) {
            fakePong(server);
            isFakePong = true;
        }
    }
    
    // Message channel
    if (s.find("PRIVMSG") != string::npos && s.find("#") != string::npos) {
        pos = s.find("#");
        pos1 = s.find(" ", pos);
        pos2 = pos1 - pos;
        CHAN = s.substr(pos, pos2);
    }
    
    // Message
    if (s.find("PRIVMSG") != string::npos) {
        pos = s.find("PRIVMSG");
        pos1 = s.find(":", pos);
        smsg = s.substr(pos1+1);
    }
    
    // Message sender
    if (s.find("!") != string::npos) {
        pos = s.find("!");
        USERQ = s.substr(1, pos-1);
    }
    
    if (s.find("PRIVMSG") != string::npos && s.find(NICK) != string::npos && s.find("#") == string::npos) {
        /** Send private messages to owner */
        sendData("PRIVMSG " + OWNER + " :" + s);
    }
    
    if (WDETECT == "1") {
        /** Windows users detector */
        if (s.find("JOIN") != string::npos && USERQ != NICK) sendData("PRIVMSG " + USERQ + " :\001 VERSION\001");
        if (s.find("\001VERSION") != string::npos) {
            string aver = s;
            const char* badclient[12] = {"mirc", "mIRC", "Script", "DenS", "dens", "Dens", "neon", "Windows", "windows", "narkoman", "eMule", "pIRC"};
            for (int i = 0; i < 12; i++) {
                if (aver.find(badclient[i]) != string::npos) {
                    sendData("PRIVMSG " + WCHANNELS + " :Windows-user detected: " + USERQ);
                    break;
                }
            }
        }
    }
    
    if (smsg.find(NICK + ": say") != string::npos) {
        /** Response test */
        sendData("PRIVMSG " + CHAN + " :I said");
    }
    
    if (smsg.find("%echo") != string::npos) {
        /** Echo */
        string sm;
        sm = smsg.substr((NICK + ": echo").length()+1);
        sendData("PRIVMSG " + CHAN + " :" + sm);
    }
    
    //if (smsg == "%ls") {
        /** List of commands */
    //    sendData("PRIVMSG " + CHAN + " :" + COMMANDS);
    //}
    
    if (smsg.find(NICK + ": ") != string::npos) {
        /** Call python-modules */
        
        char *pch, *params[20], *cmsg;
        
        // Split message and put it in array
        // First element of an array is module name, each other — args passed to a module
        int i = 0;
        cmsg = &smsg[0];
        pch = strtok(cmsg, " ");
        while (pch != NULL) {
            params[i] = pch;
            pch = strtok(NULL, " ");
            i++;
        }
        
        // Fix line break in the last element
        string lpar(params[i-1]);
        lpar = replaceAll(lpar, "\n", "");
        lpar = replaceAll(lpar, "\r", "");
        params[i-1] = &lpar[0];
        //

        string cmodule(params[1]);
        cmodule = replaceAll(cmodule, "\n", "");
        
        if (modules().find(cmodule) != string::npos) {
            cout << "Call module " << cmodule << endl;
            
            vector<string> parameters;
            
            // Transorm array with arguments to a vector
            for (int j = 2; j < i; j++) {
                string par(params[j]);
                parameters.push_back(par);
            }

            string prefix = ":m/";
            string module_filename = cmodule+".py";
            
            createFork(prefix, module_filename, cmodule, CHAN, USERQ, parameters);
        }
    }
    
    if (smsg.substr(0,6) == "%lsmod") {
        /** Modules list */
        sendData("PRIVMSG " + CHAN + " :Modules:" + modules());
    }
    
    if (smsg.find("http://") != string::npos && USERQ != NICK) {
        /** Return title from URL */
        string url;
        pos = smsg.find("http://");
        if (smsg.find(" ", pos)) {
            pos1 = smsg.find(" ", pos);
        }
        
        else {
            pos1 = s.find("\r", pos);
        }
        pos2 = pos1 - pos;
        
        url = smsg.substr(pos, pos2);
        
        cout << url << "\n" << endl;
        
        int p = fork();
        if (p == 0) {
            // Set process name
            string new_ps_name(ps_name);
            new_ps_name.append(": url title");
            prctl(PR_SET_NAME,&new_ps_name[0],0,0,0);
            
            string title;
            title = urlhandler(url, true, false);
            
            if (title != "None") {
                sendData("PRIVMSG " + CHAN + " :Title: " + title);
            }
            
            // There're additional info for videos on youtube
            if (url.find("youtube.com/watch?v=") != string::npos) {
                sendData("PRIVMSG " + CHAN + " :Info: " + urlhandler(url, false, true));
            }
            
            exit(0);
        }
        
        signal(SIGCHLD, SIG_IGN);
        
        if (p < 0) {
            perror("FORK FAILED");
            exit(0);
        }
    }
    
    if (!isCore) {
        coreModules();
        isCore = true;
    }
    
    if (smsg.substr(0,7) == "%update") {
        /** Refresh event list */
        eventList();
        
        string events_size;
        stringstream out;
        out << events.size();
        events_size = out.str();
        
        sendData("PRIVMSG " + CHAN + " :Done. Found " + events_size + " events");
    }
    
    /** Call modules on events/words/phrases */
    int events_size = events.size();
    for (int i = 0; i < events_size; i++) {
        string current(events.at(i));
        string module;
        string event;
        
        module = current.substr(0, current.find(".py"));
        event = current.substr(current.find(" ")+1);
        
        if (smsg.find(event) != string::npos) {
            cout << "Found event: [" << event << "] Calling " << module << endl;
            smsg = replaceAll(smsg, "\n", "");
            smsg = replaceAll(smsg, "\r", "");
            
            vector<string> parameters;
            parameters.push_back(smsg);
            
            string prefix = ":e/";
            string module_filename = module+".py";
            
            createFork(prefix, module_filename, module, CHAN, USERQ, parameters);
        }
    }

    return 0;
}

// Modules list
string Connection::modules() const {
    string list;
    DIR *dp;
    struct dirent *ep;
    
    dp = opendir("./modules/");
    if (dp != NULL) {
        string s(" ");
        while ((ep = readdir(dp))) list += ep->d_name + s;
        closedir(dp);
    }
    else perror("Couldn't open the directory");
    
    list = replaceAll(list, ".py", "");
    list = replaceAll(list, ".", "");
    list.replace(list.find(" "), 1, "");
    return list;
}

string Connection::replaceAll(string& context, const string& from, const string& to) const {
    size_t lookHere = 0;
    size_t foundHere;
    
    while ((foundHere = context.find(from, lookHere)) != string::npos) {
        context.replace(foundHere, from.size(), to);
        lookHere = foundHere + to.size();
    }
    
    return context;
}

// Get and process webpage data
string Connection::urlhandler(string& url, bool onlytitle, bool ytinfo) const {
    url = replaceAll(url, "\n", "");
    url = replaceAll(url, "\r", "");
    string content, title, com;
    
    if (onlytitle) {
        com = "wget robots=off --quiet -O - \"" + url + "\"";
        size_t c;
        
        FILE* fp = popen(&com[0], "r"); // I think this is easier than libcurl
        
        while ((c = getc(fp)) != unsigned EOF) {
            content += c;
        }
        
        pclose(fp);
        
        if (content.find("<title>") != string::npos) {
            int pos, pos1, pos2, pos3;
            pos  = content.find("<title>");
            pos1 = content.find("</title>", pos);
            pos2 = pos1 - pos - 7;
            
            title = content.substr(pos+7, pos2);
            title = replaceAll(title, "&quot;", "\"");
            title = replaceAll(title, "&mdash;", "-");
            
            if (content.find("charset=") != string::npos) {
                string cp;
                
                pos = content.find("<meta ");
                pos1 = content.find("charset=", pos);
                pos2 = content.find("\"", pos1);
                pos3 = pos2 - pos1 - 8;
                cp = content.substr(pos1+8, pos3);
                
                cout << "%%%" << cp <<endl;
                
                if (cp != "utf-8" && cp != "UTF-8") {
                    if (cp == "windows-1251") {
                        cp = "cp1251";
                    }
                    title = codepage(title, "utf-8", cp);
                }
            }
            
            if (title.find("\t") != string::npos) {
                title = replaceAll(title, "\t", "");
            }
            if (title.find("\n") == string::npos) {
                return title;
            }
            //else return "Can't parse this shitty page";
            else {
                return replaceAll(title, "\n", " ");
            }
        }
        
        else {
            return "None";
        }
    }
    
    if (ytinfo) {
        // Call the YouTube API module and return video information
        string ret;
        
        int p = fork();
        if (p == 0) {
            // Set process name
            string new_ps_name(ps_name);
            new_ps_name.append(": yt info");
            prctl(PR_SET_NAME,&new_ps_name[0],0,0,0);
            
            url = replaceAll(url, "http://", "");
            com = "python core/youtube_info.py \"" + url + "\"";
            FILE* fp = popen(&com[0],"r");
            size_t c;
            
            if (fp) {
                string ret;
                while ((c = getc(fp)) != unsigned EOF) ret += c;
                pclose(fp);
                return ret;
            }
            else {
                cout << "FAILED" << endl;
            }
            
            exit(0);
        }
        
        signal(SIGCHLD, SIG_IGN);
        
        if (p < 0) {
            perror("FORK FAILED");
            exit(0);
        }
        //return ret;
    }
    
    return 0;
}

// Encoding converter
string Connection::codepage(string& text, const string& from, const string& to) const {
    iconv_t cd;
    size_t f, t;
    const char *code = &text[0];
    char* in = const_cast<char*>(code);;
    char buf[300];
    char* out = buf;

    cd = iconv_open(&from[0], &to[0]);
    if (cd == (iconv_t)(-1)) printf("iconv error");
    f = strlen(code);
    t = sizeof buf;
    
    memset(&buf, 0, sizeof buf);
    iconv(cd, &in, &f, &out, &t);

    iconv_close(cd);
    
    return buf;
}

// Launch modules with a timer
int Connection::coreModules() {
    string line, module;
    int timer;
    ifstream file ("config/mod.conf");
    
    if (file.is_open()) {
        while (!file.eof()) {
            getline(file, line);
            
            module = line.substr(0, line.find(" "));
            
            if (module != "") {
                stringstream ss(line.substr(line.find(" ")+1));
                ss >> timer;
                
                int p = fork();
                if (p == 0) {
                    // Set process name
                    string mod_filename;
                    mod_filename = module.substr(module.find("/")+1);
                    string new_ps_name(ps_name);
                    new_ps_name.append(":c/");
                    new_ps_name.append(mod_filename);
                    prctl(PR_SET_NAME,&new_ps_name[0],0,0,0);
                    
                    module = mod_filename.substr(0, mod_filename.find(".py"));
                    
                    Py_Initialize();
                    PyRun_SimpleString("import sys");
                    PyRun_SimpleString("sys.path.append('.')");
                    
                    // Import module
                    string module_string = "core."+module;
                    
                    PyObject* pluginModule = PyImport_Import(PyString_FromString(&module_string[0]));
                    if (!pluginModule) {
                    PyErr_Print();
                        cout << "Error importing module " << module_string << endl;
                    }
                    
                    // Getting main method
                    PyObject* mainFunc = PyObject_GetAttrString(pluginModule, "horo");
                    if (!mainFunc) {
                        PyErr_Print();
                        cout << "Error retrieving 'horo' from " << module_string << endl;
                    }
                    
                    int i = 0; // Module call count
                    while (1) {
                        // Calling main method
                        PyObject* result = PyObject_CallObject(mainFunc, NULL);
                        if (!result) {
                            PyErr_Print();
                            cout << "Error invoking 'horo' in " << module_string << endl;
                        }
        
                        const char* cResult = PyString_AsString(result);
                        if (!cResult) {
                            PyErr_Print();
                            cout << "Error converting result to C string" << endl;
                        }
                      
                        sendData(cResult);
        
                        i++;
                        cout << "## " << module << ": " << i << endl;
                        sleep(timer);
                    }
                    
                    Py_Finalize();
                }
                
                signal(SIGCHLD, SIG_IGN);
                
                if (p < 0) {
                    perror("FORK FAILED");
                    exit(0);
                }
            }
        }
        
        file.close();
    }
    else {
        cout << "## Unable to open mod.conf" << endl; 
    }

    return 0;
}

// Creating list of events/words in a message for module calling, saving to a vector
int Connection::eventList() {
    events.clear();
    string list;
    DIR *dp;
    struct dirent *ep;
    
    string dir("./modules/"); // modules directory
    string ret;
    
    dp = opendir(&dir[0]);
    if (dp != NULL) {
        string s(" ");
        
        while ((ep = readdir(dp))) {
            /* Find EVENTS lists in all new modules. This list contains some chars
             * for bot's reaction. Then process the list and add each element 
             * to a vector in format: "module_name.py event_string" */
            
            string command("grep -r EVENTS "+dir+ep->d_name);
            FILE *fp = popen(&command[0], "r" );
            size_t c;
            
            while ((c = getc(fp)) != unsigned EOF) {
                if (c != '\n') ret += c;
                else break;
            }
            
            pclose(fp);
            
            ret = replaceAll(ret, "[", "");
            ret = replaceAll(ret, "]", "");
            ret = replaceAll(ret, "'", "");
            ret = replaceAll(ret, "\"", "");
            ret = replaceAll(ret, ",", "");
            
            if (ret.find("=") != string::npos) {
                ret = ret.substr(ret.find("=")+1);
            }
            
            istringstream iss(ret, istringstream::in);
            string word;
            while(iss >> word) {
                string w(word);
                
                if (w != "") {
                    string file(ep->d_name);
                    string element(file+" "+w);
                    events.push_back(element);
                }
            }
            
            ret.clear();
        }
        
        closedir(dp);
    }
    else {
        perror("Couldn't open the directory");
    }
    
    return 0;
}

// Fake pongs
int Connection::fakePong(string server) {
    int p = fork();
    if (p == 0) {
        // Set process name
        string new_ps_name(ps_name);
        new_ps_name.append(": fake pong");
        prctl(PR_SET_NAME,&new_ps_name[0],0,0,0);
        
        while (1) {
          sendData("PONG " + server);
          
          sleep(atoi(FAKEPONGTIME.c_str()));
        }
    }
    
    signal(SIGCHLD, SIG_IGN);
    
    if (p < 0) {
        perror("FORK FAILED");
        exit(0);
    }
    
    return 0;
}

// Create fork
int Connection::createFork(string& child_prefix, string& child_name, string& module_name,\
string& channel, string& user, vector<string> params) {
    int p;
    p = fork();
    
    if (p == 0) {
        // Set proccess name
        string new_ps_name(ps_name);
        new_ps_name.append(child_prefix);
        new_ps_name.append(child_name);
        prctl(PR_SET_NAME, &new_ps_name[0], 0, 0, 0);
        
        Py_Initialize();
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('.')");
        
        bool isHelp = false;
        
        PyObject *parameters = PyTuple_New(params.size());
        
        if (params.size() && (params.at(0) == "--help" || params.at(0) == "-h")) {
            // Docstring retrieved
            isHelp = true;
        }
        else {
            // Preparing parameters
            if (!parameters) {
                parameters = NULL;
            }
        
            for (unsigned int i = 0; i < params.size(); i++) {
                PyObject* param = PyString_FromString(&params.at(i)[0]);
                PyTuple_SET_ITEM(parameters, i, param);
            
                Py_DECREF(param);
            }
        }
        
        // Import module
        string module_string = "modules."+module_name;
        PyObject* pluginModule = PyImport_Import(PyString_FromString(&module_string[0]));
        if (!pluginModule) {
            PyErr_Print();
            cout << "Error importing module " << module_string << endl;
        }
        
        // Getting main method
        PyObject* mainFunc = PyObject_GetAttrString(pluginModule, "horo");
        if (!mainFunc) {
            PyErr_Print();
            cout << "Error retrieving 'horo' from " << module_string << endl;
        }
        
        if (isHelp) {
            // Getting docstring
            PyObject* pyDocString = PyObject_GetAttrString(mainFunc, "__doc__");
            string docString = PyString_AsString(pyDocString);

            if (docString != "") {
                string result = "PRIVMSG " + user + " :" + docString;
            
                sendData(result);
            }
            
            Py_DECREF(pyDocString);
        }
        else {
            // Building arguments object
            PyObject* args = Py_BuildValue("s, s, O", &channel[0], &user[0], parameters);
            if (!args) {
                PyErr_Print();
                cout << "Error building args tuple for " << module_string << endl;
            }
        
            // Calling main method
            PyObject* result = PyObject_CallObject(mainFunc, args);
            if (!result) {
                PyErr_Print();
                cout << "Error invoking 'horo' in " << module_string << endl;
            }
            
            Py_DECREF(args);
        
            string cResult = PyString_AsString(result);
            if (cResult == "") {
                PyErr_Print();
                cout << "Error converting result to C string" << endl;
            }
            
            Py_DECREF(result);
        
            if (cResult.find("\n") != string::npos) {
                stringstream ss(cResult);
                string resultLine;
            
                while (getline(ss, resultLine, '\n')) {
                    sendData(resultLine);
                }
            }
            else {
                sendData(cResult);
            }
        }

        // Cleaning up
        Py_DECREF(pluginModule);
        Py_DECREF(parameters);
        Py_DECREF(mainFunc);
        
        Py_Finalize();
    }
    
    signal(SIGCHLD, SIG_IGN);
    
    if (p < 0) {
        perror("FORK FAILED");
        exit(0);
    }
    
    return 0;
}
