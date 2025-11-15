#pragma once

#include "netCommon.h"
#include "netMessage.h"
#include "netTSQueue.h"

namespace net {
        // Kế thừa lớp này để tạo 1 shared_ptr (this) thay vì raw pointer
        template <typename T>
        class connection : public std::enable_shared_from_this<connection<T>> {
        public:

            enum class owner {
                server,
                client
            };

            connection(owner parent, asio::ip::tcp::socket socket, asio::io_context& asioContext, TSQueue<ownedMessage<T>>& qIn)
                : _socket(std::move(socket)), _asioContext(asioContext), _qMsgsIn(qIn)
            {
                _nOwnerType = parent;
            }
            virtual ~connection(){}

            void connectToClient(uint32_t uid = 0) {
                if (_nOwnerType == owner::server) {
                    if (_socket.is_open()) {
                        id = uid; 
                        readHeader();
                    }
                }
            }

            void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {
                if (_nOwnerType != owner::client)
                    return;

                asio::async_connect(_socket, endpoints,
                    [this](std::error_code  ec, asio::ip::tcp::endpoint endpoint) {
                        if (ec) {
                            std::cout << "[" << id << "] Endpoint connect fail.\n";
                            return;
                        }

                        readHeader();
                    }
                );
            }

            void disconnect() {
                if (isConnected())
                    asio::post(_asioContext, [this]() { _socket.close(); });
            }

            bool isConnected() const { return _socket.is_open(); };

            void send(const message<T>& msg) {
                asio::post(_asioContext,
                    [this, msg] () {
                        bool isWritingMsg = !_qMsgsOut.empty();
                        _qMsgsOut.push_back(msg);

                        if (!isWritingMsg)
                            writeHeader();
                    }
                );
            }

            void startListening() {

            }

            uint32_t getID() const { return id; }
        
        private:
            void readHeader() {
                asio::async_read(_socket, asio::buffer(&_msgTempIn.header, sizeof(messageHeader<T>)), 
                    [this](std::error_code ec, std::size_t length) {
                        if (ec) {
                            std::cout << "[" << id << "] Read header fail.\n";
                            _socket.close();
                            return;
                        }

                        if (_msgTempIn.header.size > 0) {
                            _msgTempIn.body.resize(_msgTempIn.header.size);
                            readBody();
                        }
                        else
                            addToIncomingQueue();
                    }               
                );
            } 

            void readBody() {
                asio::async_read(_socket, asio::buffer(_msgTempIn.body.data(), _msgTempIn.body.size()), 
                    [this](std::error_code ec, std::size_t length) {
                        if (ec) {
                            std::cout << "[" << id << "] Read body fail.\n";
                            _socket.close();
                            return;
                        }

                        addToIncomingQueue();
                    }
                );
            }

            void writeHeader() {
                asio::async_write(_socket, asio::buffer(&_qMsgsOut.front().header, sizeof(messageHeader<T>)), 
                    [this](std::error_code ec, std::size_t length) {
                        if (ec) {
                            std::cout << "[" << id << "] Write header fail.\n";
                            _socket.close();
                            return;
                        }

                        if (_qMsgsOut.front().body.size() > 0) {
                            writeBody();
                        }
                        else {
                            _qMsgsOut.pop_front();

                            if (!_qMsgsOut.empty())
                                writeHeader();
                        }
                    }
                );
            }

            void writeBody() {
                asio::async_write(_socket, asio::buffer(_qMsgsOut.front().body.data(), _qMsgsOut.front().body.size()),
                    [this](std::error_code ec, size_t length) {
                        if (ec) {
                            std::cout << "[" << id << "] Write body fail.\n";
                            _socket.close();
                            return; 
                        }

                        _qMsgsOut.pop_front();

                        if (!_qMsgsOut.empty())
                            writeHeader();
                    }
                );
            }
            
            void addToIncomingQueue() {
                if (_nOwnerType == owner::server)
                    _qMsgsIn.push_back({ this->shared_from_this(), _msgTempIn });
                else
                    _qMsgsIn.push_back({ nullptr, _msgTempIn });

                readHeader();
            }

            protected:
                asio::ip::tcp::socket _socket;
            
                // Chỉ tham chiếu đến địa chỉ để dùng chung
                asio::io_context& _asioContext;
            
                // Hàng đợi này chứa tất cả các messages gửi đến bên kia của kết nối
                TSQueue<message<T>> _qMsgsOut;

                // Hàng đợi này chứa tất cả các messages đến kết nối. Nó chỉ là tham chiếu
                // vì nó sẽ được quản lí bởi lớp khác
                TSQueue<ownedMessage<T>>& _qMsgsIn;

                message<T> _msgTempIn;
                
                owner _nOwnerType = owner::server;
                
                uint32_t id = 0;
        };
}