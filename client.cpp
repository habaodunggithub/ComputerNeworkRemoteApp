#include <iostream>
#include "netHeader.h"
#include "netClient.h"
#include <chrono>
#include <thread>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};



class CustomClient : public net::clientInterface<CustomMsgTypes>
{
public:
	void PingServer() {
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		// 1. Lấy thời điểm hiện tại
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		
        
        // 2. CHUYỂN ĐỔI: Lấy giá trị số nguyên (số tick)
        // Dùng 'count()' để lấy số lượng tick dưới dạng kiểu dữ liệu cơ bản
        uint64_t timeValue = timeNow.time_since_epoch().count();

		msg << timeValue; // Gửi giá trị uint64_t an toàn
		send(msg);
	}

	void MessageAll() {
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;		
		send(msg);
	}
};

int main()
{
	CustomClient c;
	c.connect("127.0.0.1", 60000);

    
    bool key[3] = { false, false, false };
    bool old_key[3] = { false, false, false };
	bool bQuit = false;
	while (!bQuit)
	{
        // Lấy trạng thái phím
        key[0] = GetAsyncKeyState('1') & 0x8000;
        key[1] = GetAsyncKeyState('2') & 0x8000;
        key[2] = GetAsyncKeyState('3') & 0x8000;

        // Phát hiện cạnh lên (chỉ thực thi một lần khi phím vừa nhấn)
        if (key[0] && !old_key[0]) c.PingServer();
        if (key[1] && !old_key[1]) c.MessageAll();
        if (key[2] && !old_key[2]) bQuit = true;

        // Cập nhật trạng thái cũ
        for (int i = 0; i < 3; i++) old_key[i] = key[i];
		
        if (c.isConnected())
		{
			if (!c.incoming().empty()) {
				auto msg = c.incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case CustomMsgTypes::ServerAccept:
				{
					// Server has responded to a ping request				
					std::cout << "Server Accepted Connection\n";
				}
				break;


				case CustomMsgTypes::ServerPing:
				{
                    // Lấy thời điểm hiện tại
                    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                    
                    // 1. TRÍCH XUẤT: Lấy giá trị số nguyên (số tick)
                    uint64_t timeValue;
                    msg >> timeValue; 
                    
                    // 2. CHUYỂN ĐỔI NGƯỢC: Tạo duration từ số tick
                    // Chú ý: Bạn phải biết "tỷ lệ duration" (duration ratio) mà system_clock sử dụng.
                    // Giả sử system_clock sử dụng std::chrono::nanoseconds (thường là mặc định trên GCC/MSVC):
                    
                    // Tạo duration từ giá trị số nguyên đã nhận
                    std::chrono::system_clock::duration duration(timeValue);
                    
                    // Tạo lại time_point
                    std::chrono::system_clock::time_point timeThen(duration);
                    
                    // Tính toán Ping
                    std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "s\n";
                }
				break;

				case CustomMsgTypes::ServerMessage:
				{
					// Server has responded to a ping request	
					uint32_t clientID;
					msg >> clientID;
					std::cout << "Hello from [" << clientID << "]\n";
				}
				break;
				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			bQuit = true;
		} 

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}