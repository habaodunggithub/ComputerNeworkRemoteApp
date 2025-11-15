#pragma once

#include "netCommon.h"
#include "netConnection.h"
#include "netMessage.h"
#include "netTSQueue.h"

namespace net {
    template <typename T>
    class clientInterface{
    public:
        clientInterface() : _socket(_context){
        }

        virtual ~clientInterface() {
            disconnect();
        }

        // Kết nối với server với ip và port
        bool connect(const std::string& host, const uint16_t port) {
            try {
                // Cấp phát một vùng nhớ và trả về con trỏ unique_ptr 
                _connection = std::make_unique<connection<T>>();

                // Resolver hoạt động như 1 DNS server, nhận vào hostname/địa chỉ IP và port dưới dạng
                // chuỗi, sau đó tìm địa chỉ IP phù hợp
                asio::ip::tcp::resolver resolver(_context);
                asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

                _connection->connectToServer(endpoints);
            
                // Bắt đầu một thread để context chạy trên đó
                thrContext = std::thread([this]() { _context.run(); });
            }
            catch (std::exception &e) {
                std::cerr << "Client error: " << e.what() << "\n";
                return false;
            }
            
            return false;
        }

        void disconnect() {
            if (isConnected()) {
                _connection->disconnect();
            }

            // Dừng context và luồng mà nó hoạt động
            _context.stop();
            if (thrContext.joinable())
                thrContext.join();

            // Giải phóng kết nối
            _connection.release();
        }

        bool isConnected() {
            if (_connection)
                return _connection->isConnected();
            return false;
        }

        TSQueue<ownedMessage<T>>& incoming() {
            return _qMsgsIn;
        }

    protected:
        // Asio context xử lý việc trao đổi dữ liệu,...
        asio::io_context _context;

        // Luồng riêng dành cho asio context
        std::thread thrContext;

        // Socket phần cứng kết nối với server
        asio::ip::tcp::socket _socket;

        // Client có một instance của connection để xử lý truyền dữ liệu
        std::unique_ptr<connection<T>> _connection;

    private:
        // Hàng đợi an toàn luồng chứa những msg đến từ sercver
        TSQueue<ownedMessage<T>> _qMsgsIn;

    };
}