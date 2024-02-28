#include <iostream>
#include <fstream>
#include <string>
#include <WinSock2.h>
#include <filesystem>
#include <vector>
constexpr int DEFAULT_PORT = 27015;

// Hàm kiểm tra và tạo thư mục nếu nó chưa tồn tại
bool CreateDirectoryIfNotExists(const std::string& directory) {
    if (!std::filesystem::exists(directory)) { // Kiểm tra xem thư mục đã tồn tại chưa
        if (!std::filesystem::create_directories(directory)) { // Tạo thư mục nếu chưa tồn tại
            std::cerr << "Error: Failed to create directory - " << directory << std::endl;
            return false;
        } else {
            std::cout << "Created directory: " << directory << std::endl;
        }
    }
    return true;
}

// Hàm nhận và lưu trữ văn bản
void ReceiveText(SOCKET socket) {
    std::string text;
    char buffer[1024];
    int bytes_received;
    while ((bytes_received = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        text.append(buffer, bytes_received);
    }

    if (bytes_received == SOCKET_ERROR) {
        std::cerr << "Error: Failed to receive text." << std::endl;
        return;
    }

    std::cout << "Received text from sender: " << text << std::endl;
}

// Hàm nhận và lưu trữ tệp tin
void ReceiveFile(SOCKET socket, const std::string& storage_directory) {
    // Nhận tên tệp tin từ máy gửi
    std::string file_name;
    int file_name_length;
    recv(socket, reinterpret_cast<char*>(&file_name_length), sizeof(file_name_length), 0);
    file_name.resize(file_name_length);
    recv(socket, &file_name[0], file_name_length, 0);

    // Tạo đường dẫn đầy đủ cho tệp tin mới
    std::string file_path = storage_directory + "\\" + file_name;

    // Mở tệp tin để ghi dữ liệu nhận được
    std::ofstream output_file(file_path, std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
        return;
    }

    std::vector<char> buffer(1024); // Sử dụng một vector có kích thước lớn hơn để nhận dữ liệu
    int bytes_received;
    while ((bytes_received = recv(socket, buffer.data(), buffer.size(), 0)) > 0) {
        output_file.write(buffer.data(), bytes_received);
    }

    if (bytes_received == SOCKET_ERROR) {
        std::cerr << "Error: Failed to receive file." << std::endl;
        return;
    }

    output_file.flush(); // Đảm bảo tất cả dữ liệu được ghi vào tệp tin
    output_file.close(); // Đóng tệp tin

    // Hiển thị thông điệp "File received and stored at:"
    std::cout << "File received and stored at: " << file_path << std::endl;
}

int main(int argc, char* argv[]) {
    // Kiểm tra số lượng đối số dòng lệnh
    if (argc != 2) {
        std::cerr << "Usage: ReceiveData <storage_directory>" << std::endl;
        return 1;
    }

    // Lấy thư mục lưu trữ từ đối số dòng lệnh
    std::string storage_directory = argv[1];

    // Kiểm tra và tạo thư mục lưu trữ nếu nó chưa tồn tại
    if (!CreateDirectoryIfNotExists(storage_directory)) {
        return 1;
    }

    // Khởi tạo Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed." << std::endl;
        return 1;
    }

    // Tạo socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Liên kết socket với cổng mặc định
    sockaddr_in listenAddr;
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(DEFAULT_PORT);
    listenAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, reinterpret_cast<sockaddr*>(&listenAddr), sizeof(listenAddr)) == SOCKET_ERROR) {
        std::cerr << "Error: Binding failed." << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    } else {
        std::cout << "Socket bound to port: " << DEFAULT_PORT << std::endl;
    }

    // Chờ kết nối từ máy gửi
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error: Listen failed." << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for connection from sender..." << std::endl;

    sockaddr_in client_address;
    int client_address_size = sizeof(client_address);
    SOCKET client_socket = accept(server_socket, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error: Accept failed." << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connection established with sender." << std::endl;

    // Nhận loại dữ liệu từ máy gửi và xử lý tương ứng
    int data_type;
    int bytes_received = recv(client_socket, reinterpret_cast<char*>(&data_type), sizeof(data_type), 0);
    if (bytes_received != sizeof(data_type)) {
        std::cerr << "Error: Failed to receive data type." << std::endl;
        closesocket(client_socket);
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (data_type == 1) {
        ReceiveText(client_socket);
    } else if (data_type == 2) {
        ReceiveFile(client_socket, storage_directory);
    } else {
        std::cerr << "Error: Invalid data type received." << std::endl;
    }

    // Đóng socket
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
