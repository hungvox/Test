#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

// Hàm gửi văn bản đến máy chủ
void SendText(SOCKET socket) {
    std::wstring text;
    std::wstring line;
    std::wcout << L"Send text (type 'END' to send):" << std::endl;
    while (std::getline(std::wcin, line)) {
        if (line == L"END") {
            break;
        }
        text += line + L"\n";
    }
    int iResult = send(socket, reinterpret_cast<const char*>(text.c_str()), text.size() * sizeof(wchar_t), 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
        closesocket(socket);
        WSACleanup();
        return;
    }
    // Inform user that data has been sent successfully
    std::cout << "Send Text OK!" << std::endl;
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
    // Check command line arguments
    if (argc != 2) {
        std::cerr << "Usage: SendData <destination_address>" << std::endl;
        return 1;
    }

    // Get the destination address from command line arguments
    std::string destination_address = argv[1];

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
    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Connect to server
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(destination_address.c_str()); // Change to server IP address
    clientService.sin_port = htons(12345); // Change to server port
    if (connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
        std::cerr << "Error connecting: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Ask user what type of data they want to send
    std::cout << "What type of data do you want to send?\n";
    std::cout << "1. Text\n";
    std::cout << "2. File\n";
    std::cout << "Enter your choice: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore();  // Ignore remaining newline character

    // Call the corresponding function based on user's choice
    if (choice == 1) {
        SendText(ConnectSocket);
    } else if (choice == 2) {
        SendFile(ConnectSocket);
    } else {
        std::cerr << "Invalid choice." << std::endl;
    }

    // Cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
