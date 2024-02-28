#include <iostream>
#include <fstream>
#include <string>
#include <WinSock2.h>
#include <locale>
#include <codecvt>

constexpr int DEFAULT_PORT = 27015;

// Hàm gửi văn bản đến máy chủ
void SendText(SOCKET socket) {
    std::string text;
    std::cout << "Enter the text to send: ";
    std::getline(std::cin, text);

    // Gửi mã xác định loại dữ liệu là văn bản (1)
    int data_type = 1;
    send(socket, reinterpret_cast<const char*>(&data_type), sizeof(data_type), 0);

    // Gửi độ dài của văn bản
    int text_length = text.length();
    send(socket, reinterpret_cast<const char*>(&text_length), sizeof(text_length), 0);
    // Gửi văn bản
    send(socket, text.c_str(), text_length, 0);
}

// Hàm gửi tệp tin đến máy chủ
void SendFile(SOCKET socket) {
    std::string file_path;
    std::cout << "Enter the file path: ";
    std::getline(std::cin, file_path);

    // Gửi mã xác định loại dữ liệu là tệp tin (2)
    int data_type = 2;
    send(socket, reinterpret_cast<const char*>(&data_type), sizeof(data_type), 0);

    // Lấy tên tệp tin từ đường dẫn
    std::size_t found = file_path.find_last_of("/\\");
    std::string file_name = (found != std::string::npos) ? file_path.substr(found + 1) : file_path;

    // Gửi độ dài của tên tệp tin
    int file_name_length = file_name.length();
    send(socket, reinterpret_cast<const char*>(&file_name_length), sizeof(file_name_length), 0);
    // Gửi tên tệp tin
    send(socket, file_name.c_str(), file_name_length, 0);

    // Nhập buffer size từ người dùng
    int buffer_size;
    std::cout << "Enter buffer size (in bytes): ";
    std::cin >> buffer_size;

    // Mở tệp tin để gửi dữ liệu
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file." << std::endl;
        return;
    }

    char buffer[buffer_size];
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        int bytes_read = file.gcount();
        if (bytes_read > 0) {
            send(socket, buffer, bytes_read, 0);
        }
    }

    std::cout << "File sent successfully." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: SendData <destination_address>" << std::endl;
        return 1;
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed." << std::endl;
        return 1;
    }

    // Tạo socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Thiết lập địa chỉ máy chủ
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(DEFAULT_PORT);
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    // Kết nối đến máy chủ
    if (connect(client_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "Error: Connection failed." << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // Chọn loại dữ liệu để gửi
    std::cout << "Enter the type of data you want to send:" << std::endl;
    std::cout << "1. Text" << std::endl;
    std::cout << "2. File" << std::endl;
    std::cout << "Enter your choice: ";

    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "1") {
        SendText(client_socket);
    } else if (choice == "2") {
        SendFile(client_socket);
    } else {
        std::cerr << "Invalid choice." << std::endl;
    }

    // Đóng socket
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
