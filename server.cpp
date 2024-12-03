#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

std::vector<int> clients;
std::mutex clients_mutex;

void broadcast(const std::string& message, int sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int client : clients) {
        if (client != sender_socket) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

void handle_client(int client_socket) {
    char buffer[1024];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received string
        std::string message = std::string(buffer);
        std::cout << "Received: " << message << std::endl;
        broadcast(message, client_socket);
    }

    // Remove client from the list and close the socket
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    }
    close(client_socket);
    std::cout << "Client disconnected." << std::endl;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP
    server_addr.sin_port = htons(5555); // Port number

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket." << std::endl;
        return 1;
    }

    listen(server_socket, 5);
    std::cout << "Server is listening on port 5555..." << std::endl;

    while (true) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection." << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_socket);
        }

        std::thread(handle_client, client_socket).detach();
        std::cout << "New client connected." << std::endl;
    }

    close(server_socket);
    return 0;
}