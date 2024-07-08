// BridgeGameAgent.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

int main()
{

    
    // 初始化 Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // 创建套接字
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // 设置服务器地址和端口
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    InetPton(AF_INET, L"127.0.0.1", &(serverAddress.sin_addr));// 本地地址
    serverAddress.sin_port = htons(37000); // 服务器端口

    // 连接到服务器
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to connect to the server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }


    bool initialized = false; //未初始化时，程序首先通过套接字发送自身路径，并在收到第一条回复后initialized变为真

    

    while (true)
    {
        
        int bytesSent = 0;
        if (!initialized)
        {
            wchar_t pathStr[MAX_PATH];
            GetModuleFileName(NULL, pathStr, MAX_PATH);
            int bufferSize = WideCharToMultiByte(CP_UTF8, 0, pathStr, -1, NULL, 0, NULL, NULL);
            std::string input(bufferSize, '\0');
            WideCharToMultiByte(CP_UTF8, 0, pathStr, -1, &input[0], bufferSize, NULL, NULL);
            bytesSent = send(clientSocket, input.c_str(), static_cast<int>(input.length()), 0);
        }
        else
        {
            std::string input;
            std::getline(std::cin, input);
            bytesSent = send(clientSocket, input.c_str(), static_cast<int>(input.length()), 0);
        }
        // 从控制台读取输入
        // 发送输入的数据到服务器
        
        if (bytesSent == SOCKET_ERROR)
        {
            std::cerr << "Failed to send data to the server." << std::endl;
            break;
        }


        // 接收服务器的回复
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0)
        {
            if (initialized)
            {
                std::cout << buffer << std::endl;
            }
            else
            {
                initialized = true;
            }


        }
        else if (bytesReceived == 0)
        {
            std::cerr << "Server disconnected." << std::endl;
            break;
        }
        else
        {
            std::cerr << "Failed to receive data from the server." << std::endl;
            break;
        }
        // 检查是否输入了退出命令
        std::string reply;
        if (buffer == "SHUTDOWN")
        {
            reply = "OK SHUTDOWN";
            send(clientSocket, "OK SHUTDOWN", reply.size(), 0);
            break;
        }
    }

    // 关闭套接字和清理 Winsock
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

