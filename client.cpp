#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5555
#define BUFFER_SIZE 1024

void receive_messages(int client_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cout << "Disconnected from server.\n";
            break;
        }
        buffer[bytes_received] = '\0'; // Null-terminate the string
        std::cout << buffer;
    }
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // Change to server's IP if needed

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection to server failed\n";
        return -1;
    }

    std::thread(receive_messages, client_socket).detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "exit") {
            break;
        }
        send(client_socket, message.c_str(), message.size(), 0);
    }

    close(client_socket);
    return 0;
}