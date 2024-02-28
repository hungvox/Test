---HƯỚNG DẪN CHẠY CHƯƠNG TRÌNH---
- Câu lệnh để biên dịch file SendData: g++ -o SendData.exe SendData.cpp -lws2_32
- Câu lệnh để biên dịch file receiveDat: g++ -o ReceiveData.exe ReceiveData.cpp -lws2_32
- Sau khi mở 2 source code đã được compile là SendData và ReceiveData chúng ta sẽ chạy lần lượt 2 chương trình trong đó ta chạy file ReceiveData trước.
- Để chạy file ta dùng dòng lệnh ./ReceiveData.exe <đường_dẫn_lưu_trữ>
- Chạy file SendData ta dùng dòng lệnh ./SendData.exe 127.0.0.1
	+ Tại terminal sẽ hiện lên lựa chọn SendFile hoặc SendText ta nhấn số để lựa chọn.
	+ Nếu chọn SendText: nhập đoạn text muốn gửi
	+ Nếu chọn SendFile: nhập đường dẫn mà file đang ở, sau đó nhập buffer_size 
- Bên Receive sẽ in ra màn hình đoạn Text được gửi đến và nếu là File sẽ in ra vị trí File đó được lưu trữ. Kiểm tra trong thư mục được chỉ định lưu trữ sẽ thấy File được gửi đến đó.
	
