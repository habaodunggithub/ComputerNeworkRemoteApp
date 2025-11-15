#pragma once

#include "netCommon.h"
#include "netConnection.h"
#include "netMessage.h"
#include "netTSQueue.h"

namespace net {
    template <typename T>
    class serverInterface {
        public:
            serverInterface(uint16_t port)
                : _asioAcceptor(_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
            {

            }

            virtual ~serverInterface() {
                stop();
            }

            bool start() {
                try {
                    waitForClientConnection();

                    _thrContext = std::thread([this]() { _asioContext.run(); });
                }
                catch (std::exception &e) {
                    std::cerr << "[SERVER] Exception: " << e.what() << "\n";
                    return false;
                }

                std::cout << "[SERVER] Started.\n";
                return true;
            }

            void stop() {
                _asioContext.stop();
                if (_thrContext.joinable()) 
                    _thrContext.join();

                std::cout << "[SERVER] Stopped.\n";
            }

            // ASYNC - Chờ kết nối từ client
            void waitForClientConnection() {
                _asioAcceptor.async_accept(
                    [this](std::error_code ec, asio::ip::tcp::socket socket) {
                        if (ec) {
                            std::cout << "[SERVER] New connection error: " << ec.message() << "\n";
                        }

                        std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << "\n";

                        // std::shared_ptr<connection<T>> newConnection = std::make_shared<connection<T>> (
                        //     connection<T>::onwner::server,
                        //     _asioContext,
                        //     std::move(socket),
                        //     _qMsgsIn  
                        // );

                        // // Để server có thể từ chối kết nối hoặc không
                        // if (onClientConnect(newConnection)) {
                        //     _deqConnections.push_back(std::move(newConnection));

                        //     _deqConnections.back()->connectToClient(nIDCounter++);

                        //     std::cout << "[" << _deqConnections.back()->getID() << "] Connection approved.\n";
                        // }
                        // else {
                        //     std::cout << "[-----] Connection denied.\n";
                        // }

                        // Duy trì trạng thái chờ
                        waitForClientConnection();

                    }
                );
            }   

            // Gửi msg đến client
            void msgClient(std::shared_ptr<connection<T>> client, const message<T>& msg) {
                if (!client || !client->isConnected()) {
                    onClientDisconnect(client);
                    client.reset();

                    _deqConnections.erase(
                        std::remove(_deqConnections.begin(), _deqConnections.end(), client),
                        _deqConnections.end()
                    );

                    return;
                }

                client->send(msg);
            }

            void msgAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr) {
                bool bInvalidClientExists = false;

                for (auto &client : _deqConnections) {
                    if (!client || !client->isConnected()) {
                        onClientDisconnect(client);
                        client.reset();           
                        
                        bInvalidClientExists = true;
                        continue;
                    }

                    if (client != pIgnoreClient)
                        client->send(msg);
                }

                if (bInvalidClientExists) 
                    _deqConnections.erase(
                        std::remove(_deqConnections.begin(), _deqConnections.end(), nullptr),
                        _deqConnections.end()
                    );
            }

            // size_t là unsigned nên gắn bằng -1 nó sẽ là giá trị max của size_t
            void update(size_t nMaxMsgs = -1) {
                size_t nMsgsCount = 0;
                while (nMsgsCount < nMaxMsgs && !_qMsgsIn.empty()) {
                    auto msg = _qMsgsIn.pop_front();

                    // Giao cho bộ phận xử lý message 
                    onMsg(msg.remote, msg.msg);

                    ++nMsgsCount;
                }
            }

        protected:
        // Gọi hàm khi client kết nối
        virtual bool onClientConnect(std::shared_ptr<connection<T>> client) {
            return false;
        }

        // Gọi khi client ngắt kết nối
        virtual void onClientDisconnect(std::shared_ptr<connection<T>> client) {

        }

        virtual void onMsg(std::shared_ptr<connection<T>> client, message<T>& msg) {

        }

        // Hàng đợi an toàn luông cho những gói tin đến
        TSQueue<ownedMessage<T>> _qMsgsIn;

        // Container chứa các kết nối hợp lệ
        std::deque<std::shared_ptr<connection<T>>> _deqConnections;

        asio::io_context _asioContext;
        std::thread _thrContext;

        // Dùng để kết nối với socket của client. Acceptor cũng cần context
        asio::ip::tcp::acceptor _asioAcceptor;

        // ID cho clients
        uint32_t nIDCounter = 10000;
     };
}