#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

// 处理HTTP请求
std::string handleRequest(const std::string &method, const std::string &uri, const std::string &body)
{
    std::string response;

    if (method == "GET")
    {
        // 处理GET请求
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html; charset=utf-8\r\n"
                   "\r\n"
                   "<html><head><title>Get Request</title></head><body><h1>Get Request Received!</h1></body></html>";
    }
    else if (method == "POST")
    {
        // 处理POST请求
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html; charset=utf-8\r\n"
                   "\r\n"
                   "<html><head><title>Post Request</title></head><body><h1>Post Request Received!</h1><p>" +
                   body + "</p></body></html>";
    }
    else
    {
        // 处理其他请求
        response = "HTTP/1.1 404 Not Found\r\n"
                   "Content-Type: text/html; charset=utf-8\r\n"
                   "\r\n"
                   "<html><head><title>Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
    }

    return response;
}

// 处理客户端连接
void handleClient(int client_socket)
{
    char buffer[BUFFER_SIZE];

    while (true)
    {
        // 接收HTTP请求
        std::string request;
        int n = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (n <= 0)
        {
            break;
        }
        request.append(buffer, n);

        // 解析HTTP请求
        size_t pos = request.find(' ');
        if (pos == std::string::npos)
        {
            continue;
        }
        std::string method = request.substr(0, pos);

        pos = request.find(' ', pos + 1);
        if (pos == std::string::npos)
        {
            continue;
        }
        std::string uri = request.substr(request.find(' ') + 1, pos - request.find(' ') - 1);

        // 查找HTTP请求头结束位置
        pos = request.find("\r\n\r\n");
        if (pos == std::string::npos)
        {
            continue;
        }

        // 解析HTTP请求Body
        std::string body = request.substr(pos + 4);

        // 处理HTTP请求
        std::string response = handleRequest(method, uri, body);

        // 发送HTTP响应
        send(client_socket, response.c_str(), response.length(), 0);

        // 关闭连接
        close(client_socket);
        break;
    }
}

int main()
{
    // 创建TCP套接字
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Create socket failed!" << std::endl;
        return -1;
    }

    // 绑定地址
    struct sockaddr_in serv_addr
    {
    };
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // 使用Ipv4地址
    serv_addr.sin_port = htons(PORT); // htons的目的是转换成能在web使用的端口格式
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        std::cerr << "Bind failed!" << std::endl;
        return -1;
    }

    // 监听端口
    if (listen(server_socket, SOMAXCONN) == -1)
    {
        std::cerr << "Listen failed!" << std::endl;
        return -1;
    }

    std::cout << "Server started. Listening on port " << PORT << "..." << std::endl;

    while (true)
    {
        // 接收连接请求
        struct sockaddr_in client_address
        {
        };
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1)
        {
            std::cerr << "Accept failed!" << std::endl;
            continue;
        }

        std::cout << "Client connected. IP address: " << inet_ntoa(client_address.sin_addr) << std::endl;

        // 处理客户端连接
        std::thread client_thread(handleClient, client_socket);
        client_thread.detach();
    }

    // 关闭服务器套接字
    close(server_socket);

    return 0;
}