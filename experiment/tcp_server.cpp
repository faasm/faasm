#include <iostream>
#include <csignal>
#include <ctime>
#include <util/TCPServer.h>

using namespace util;

TCPServer tcp;
pthread_t msg1[MAX_CLIENT];
int num_message = 0;
int time_send   = 2700;

void close_app(int s) {
    tcp.closed();
    exit(0);
}

void * send_client(void * m) {
    struct descript_socket *desc = (struct descript_socket*) m;

    while(1) {
        if(!tcp.is_online() && tcp.get_last_closed_sockets() == desc->id) {
            cerr << "Connessione chiusa: stop send_clients( id:" << desc->id << " ip:" << desc->ip << " )"<< endl;
            break;
        }
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
        int hour = now->tm_hour;
        int min  = now->tm_min;
        int sec  = now->tm_sec;

        std::string date =
                to_string(now->tm_year + 1900) + "-" +
                to_string(now->tm_mon + 1)     + "-" +
                to_string(now->tm_mday)        + " " +
                to_string(hour)                + ":" +
                to_string(min)                 + ":" +
                to_string(sec)                 + "\r\n";
        cerr << date << endl;
        tcp.Send(date, desc->id);
        sleep(time_send);
    }
    pthread_exit(NULL);
    return 0;
}

void * received(void * m)
{
    pthread_detach(pthread_self());
    vector<descript_socket*> desc;
    while(1)
    {
        desc = tcp.getMessage();
        for(unsigned int i = 0; i < desc.size(); i++) {
            if( desc[i]->message != "" )
            {
                if(!desc[i]->enable_message_runtime)
                {
                    desc[i]->enable_message_runtime = true;
                    if( pthread_create(&msg1[num_message], NULL, send_client, (void *) desc[i]) == 0) {
                        cerr << "ATTIVA THREAD INVIO MESSAGGI" << endl;
                    }
                    num_message++;
                    // start message background thread
                }
                cout << "id:      " << desc[i]->id      << endl
                     << "ip:      " << desc[i]->ip      << endl
                     << "message: " << desc[i]->message << endl
                     << "socket:  " << desc[i]->socket  << endl
                     << "enable:  " << desc[i]->enable_message_runtime << endl;
                tcp.clean(i);
            }
        }
        usleep(1000);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        cerr << "Usage: ./server port (opt)time-send" << endl;
        return 0;
    }
    if(argc == 3)
        time_send = atoi(argv[2]);
    std::signal(SIGINT, close_app);

    pthread_t msg;
    vector<int> opts = { SO_REUSEPORT, SO_REUSEADDR };

    if( tcp.setup(atoi(argv[1]),opts) == 0) {
        if( pthread_create(&msg, NULL, received, (void *)0) == 0)
        {
            while(1) {
                tcp.accepted();
                cerr << "Accepted" << endl;
            }
        }
    }
    else
        cerr << "Errore apertura socket" << endl;
    return 0;
}
