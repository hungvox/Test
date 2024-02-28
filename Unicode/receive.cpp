#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <fstream>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")

std::string directory;  // Global variable to store the directory

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

// Hàm nhận văn bản từ máy khách
void ReceiveText(SOCKET socket) {
    wchar_t buffer[1024];
    int iResult = recv(socket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
    if (iResult > 0) {
        std::wcout << L"Received text: " << buffer << std::endl;
    } else {
        std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
        closesocket(socket);
        WSACleanup();
        return;
    }
    // Inform user that data has been received successfully
    std::cout << "Receive Text OK!" << std::endl;
}

// Hàm nhận tệp tin từ máy khách
void ReceiveFile(SOCKET socket) {
    // Nhận độ dài của tên tệp tin
    int file_name_length;
    recv(socket, reinterpret_cast<char*>(&file_name_length), sizeof(file_name_length), 0);
    // Nhận tên tệp tin
    char file_name[256];
    recv(socket, file_name, file_name_length, 0);
    file_name[file_name_length] = '\0';  // Null-terminate the string

    // Mở tệp tin để nhận dữ liệu
    std::string file_path = directory + "\\" + file_name;
    if (!CreateDirectoryIfNotExists(directory)) {
        return;
    }
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file." << std::endl;
        return;
    }

    char buffer[1024];
    int bytes_received;
    while ((bytes_received = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        file.write(buffer, bytes_received);
    }

    std::cout << "File received successfully." << std::endl;
}

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 3) {
        std::cerr << "Usage: ReceiveData -out <location_store_file>" << std::endl;
        return 1;
    }

    // Get the directory from command line arguments
    if (std::string(argv[1]) == "-out") {
        directory = argv[2];
    } else {
        std::cerr << "Invalid command line arguments." << std::endl;
        return 1;
    }

    // Set console to Unicode mode
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    // Set console font
    HANDLE hdlConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX consoleFont;
    consoleFont.cbSize = sizeof(consoleFont);
    if (!GetCurrentConsoleFontEx(hdlConsole, FALSE, &consoleFont)) {
        std::cerr << "Failed to get current console font!" << std::endl;
        return 1;
    }
    wcscpy_s(consoleFont.FaceName, L"Consolas");
    if (!SetCurrentConsoleFontEx(hdlConsole, FALSE, &consoleFont)) {
        std::cerr << "Failed to set console font!" << std::endl;
        return 1;
    }

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    // Create socket
    SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind socket
    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to server IP address
    serverService.sin_port = htons(12345); // Change to server port
    if (bind(ListenSocket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
        std::cerr << "Error binding: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << "Error accepting: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Nhận mã xác định loại dữ liệu
    int data_type;
    recv(ClientSocket, reinterpret_cast<char*>(&data_type), sizeof(data_type), 0);

    // Gọi hàm tương ứng dựa trên mã loại dữ liệu
    if (data_type == 1) {
        ReceiveText(ClientSocket);
    } else if (data_type == 2) {
        ReceiveFile(ClientSocket);
    } else {
        std::cerr << "Invalid data type." << std::endl;
    }

    // Cleanup
    closesocket(ClientSocket);
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}
