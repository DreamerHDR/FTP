#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <filesystem> 
#include <sstream>
#include <thread>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")


using namespace std;

string stroka;

static in_addr convert_IP(const char* SERVER_IP)
{
    in_addr ip_to_num;
    int ES{};
    ES = inet_pton(AF_INET, SERVER_IP, &ip_to_num);
    if (ES <= 0)
    {
        cout << "Error in IP translation to special numeric format" << endl;
    }
    else
    {
        return ip_to_num;
    }
}

static int secondary_port(string str)
{
    size_t start{ str.find("(") + 1 }, end{ str.find(")") };

    string tmp{ str.begin() + start, str.begin() + end };

    vector<string> ip;
    string num;

    for (auto ch : tmp)
    {
        if (ch != ',')
        {
            num.push_back(ch);
        }
        else
        {
            ip.push_back(num);
            num.clear();
        }
    }
    ip.push_back(num);

    int x{ stoi(ip[4]) };
    int y{ stoi(ip[5]) };

    return x * 256 + y;
}

void activ_mode(string str, string ip, int port)
{
    int ES{}, QB{};
    const short BUFF_SIZE = 1024;
    int Error_stat;
    WSADATA wsData;
    sockaddr_in serv_info;
    in_addr ip_to_num;
    SOCKET Sock_serv;

    Error_stat = inet_pton(AF_INET, ip.c_str(), &ip_to_num);
    if (Error_stat <= 0)
    {
        cout << "ip error" << endl;
        return;
    }


    Error_stat = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (Error_stat != 0)
    {
        cout << "Windsock error" << endl;
        cout << WSAGetLastError() << endl;
        return;
    }
    cout << "initialization Winsock is ok" << endl;

    Sock_serv = socket(AF_INET, SOCK_STREAM, 0);

    if (Sock_serv == INVALID_SOCKET)
    {
        cout << "Socket error" << endl;
        closesocket(Sock_serv);
        WSACleanup();
        return;

    }
    cout << "Socket is ok" << endl;



    ZeroMemory(&serv_info, sizeof(serv_info));

    serv_info.sin_addr = ip_to_num;
    serv_info.sin_port = htons(port);
    serv_info.sin_family = AF_INET;


    Error_stat = bind(Sock_serv, (sockaddr*)&serv_info, sizeof(serv_info));

    if (Error_stat != 0)
    {
        cout << "Error binding " << endl;
        closesocket(Sock_serv);
        WSACleanup();
        return;
    }

    cout << "binding is ok" << endl;

    Error_stat = listen(Sock_serv, SOMAXCONN);

    if (Error_stat != 0)
    {
        cout << "can't start to listen" << WSAGetLastError() << endl;
        closesocket(Sock_serv);
        WSACleanup();
        return;
    }

    cout << "Listen is ok";
    sockaddr_in clientInfo;
    ZeroMemory(&clientInfo, sizeof(&clientInfo));
    int clientInfo_size = sizeof(clientInfo);

    SOCKET Client_sock = accept(Sock_serv, (sockaddr*)&clientInfo, &clientInfo_size);
    Sleep(2000);
    if (clientInfo_size == INVALID_SOCKET)
    {
        cout << "client can't connet, but detected" << endl;
        closesocket(Client_sock);
        return;
    }
    vector<char> buf_for_read(BUFF_SIZE);

    QB = recv(Client_sock, buf_for_read.data(), buf_for_read.size(), 0);

    if (QB == SOCKET_ERROR)
    {
        cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
        closesocket(Client_sock);
        closesocket(Sock_serv);
        return;
    }
    else
    {
        stroka = string(buf_for_read.begin(), buf_for_read.begin() + QB);
        cout << stroka << endl;
    }
    closesocket(Client_sock);
    closesocket(Sock_serv);

}

void createTextFile(const string& name, const string& text) {
    // Открытие файла для записи
    ofstream file(name);

    // Проверка на успешное открытие файла
    if (file.is_open()) {
        // Запись текста в файл
        file << text;

        // Закрытие файла
        file.close();

        cout << "Файл " << name << " успешно создан." << endl;
    }
    else {
        cout << "Ошибка при открытии файла " << name << " для записи." << endl;
    }
}

int main(void)
{
    const short BUFF_SIZE = 2048;
    int ES;
    int serverPORT{ 21 };
    string serverIP{ "127.0.0.1" };
    string User = "USER test1\r\n";
    string Pass = "PASS 1234\r\n";
    string tmp;
    vector<char> servBuff(BUFF_SIZE);
    string resp;
    int QB;

    in_addr ip_to_num = convert_IP(serverIP.data());
    sockaddr_in servInfo; // стр-ра информации о сервере
    ZeroMemory(&servInfo, sizeof(servInfo));
    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(serverPORT);

    WSADATA wsData;
    ES = WSAStartup(MAKEWORD(2, 2), &wsData);
    if (ES != 0)
    {
        cout << "Error WinSock version initialization #" << WSAGetLastError();
        return 1;
    }

    SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSock == INVALID_SOCKET)
    {
        cout << "Error initializing socket " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    ES = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
    if (ES != 0)
    {
        cout << "Connection to Server is FAILED. Error " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }

    QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
    if (QB == SOCKET_ERROR)
    {
        cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }
    else
    {
        resp = string(servBuff.begin(), servBuff.begin() + QB);
        cout << "\nServer response:\n" << resp << endl;
    }

    QB = send(ClientSock, User.data(), User.size(), 0);
    if (QB == SOCKET_ERROR)
    {
        cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }

    QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
    if (QB == SOCKET_ERROR)
    {
        cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }
    else
    {
        resp = string(servBuff.begin(), servBuff.begin() + QB);
        cout << "\nServer response:\n" << resp << endl;
    }

    QB = send(ClientSock, Pass.data(), Pass.size(), 0);
    if (QB == SOCKET_ERROR)
    {
        cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }

    QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
    if (QB == SOCKET_ERROR)
    {
        cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }
    else
    {
        resp = string(servBuff.begin(), servBuff.begin() + QB);
        cout << "\nServer response:\n" << resp << endl;
    }


    int choice{};
    string list_files;

    while (choice != 4)
    {
        cout << "\n\nEnter move\n1-LIST\n2-SAVE\n3-QUIT\n\n";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            string ipp = "127.0.0.1";
            string ssttrr;
            int pport = 1800;
            thread my_thread(activ_mode, ssttrr, ipp, pport);

            Sleep(1000);
            tmp = "PORT 127,0,0,1,7,8\r\n";
            QB = send(ClientSock, tmp.data(), tmp.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            else
            {
                resp = string(servBuff.begin(), servBuff.begin() + QB);
                cout << "\nServer response:\n" << resp << endl;
            }
            tmp = "LIST\r\n";
            QB = send(ClientSock, tmp.data(), tmp.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            Sleep(100);
            QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            else
            {
                resp = string(servBuff.begin(), servBuff.begin() + QB);
                cout << "\nServer response:\n" << resp << endl;
            }Sleep(1000);
            my_thread.join();
            break;
        }
        case 2:
        {
            tmp = "CWD /qwerty\r\n";
            QB = send(ClientSock, tmp.data(), tmp.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            else
            {
                resp = string(servBuff.begin(), servBuff.begin() + QB);
                cout << "\nServer response:\n" << resp << endl;
            }
            string ipp = "127.0.0.1";
            string ssttrr;
            int pport = 1800;
            thread my_thread(activ_mode, ssttrr, ipp, pport);

            Sleep(1000);
            tmp = "PORT 127,0,0,1,7,8\r\n";
            QB = send(ClientSock, tmp.data(), tmp.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            else
            {
                resp = string(servBuff.begin(), servBuff.begin() + QB);
                cout << "\nServer response:\n" << resp << endl;
            }
            tmp = "NLST\r\n";
            QB = send(ClientSock, tmp.data(), tmp.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            Sleep(100);
            QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            else
            {
                resp = string(servBuff.begin(), servBuff.begin() + QB);
                cout << "\nServer response:\n" << resp << endl;
            }Sleep(1000);
            my_thread.join();
            stringstream ss(stroka);
            string name;
            while (ss >> name) {
                thread my_thread(activ_mode, ssttrr, ipp, pport);

                Sleep(1000);
                tmp = "PORT 127,0,0,1,7,8\r\n";
                QB = send(ClientSock, tmp.data(), tmp.size(), 0);
                if (QB == SOCKET_ERROR)
                {
                    cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
                    closesocket(ClientSock);
                    WSACleanup();
                    return 1;
                }
                QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
                if (QB == SOCKET_ERROR)
                {
                    cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
                    closesocket(ClientSock);
                    WSACleanup();
                    return 1;
                }
                else
                {
                    resp = string(servBuff.begin(), servBuff.begin() + QB);
                    cout << "\nServer response:\n" << resp << endl;
                }
                tmp = "RETR /qwerty/";
                tmp += name;
                tmp += "\r\n";
                QB = send(ClientSock, tmp.data(), tmp.size(), 0);
                if (QB == SOCKET_ERROR)
                {
                    cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
                    closesocket(ClientSock);
                    WSACleanup();
                    return 1;
                }
                Sleep(100);
                QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
                if (QB == SOCKET_ERROR)
                {
                    cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
                    closesocket(ClientSock);
                    WSACleanup();
                    return 1;
                }
                else
                {
                    resp = string(servBuff.begin(), servBuff.begin() + QB);
                    cout << "\nServer response:\n" << resp << endl;
                }
                Sleep(1000);
                my_thread.join();
                createTextFile(name, stroka);
            }
            break;
        }

        case 3:
        {
            tmp = "QUIT\r\n";
            QB = send(ClientSock, tmp.data(), tmp.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't send message to Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            QB = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
            if (QB == SOCKET_ERROR)
            {
                cout << "Can't receive message from Server. Error " << WSAGetLastError() << endl;
                closesocket(ClientSock);
                WSACleanup();
                return 1;
            }
            else
            {
                resp = string(servBuff.begin(), servBuff.begin() + QB);
                cout << "\nServer response:\n" << resp << endl;
            }
            Sleep(1000);
            break;
        }
        default:    
            break;
        }


    }

    closesocket(ClientSock);
    WSACleanup();

    return 0;
}
