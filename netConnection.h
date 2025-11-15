#pragma once

#include "netCommon.h"
#include "netMessage.h"
#include "netTSQueue.h"
#include <memory>

namespace net {
        // Kế thừa lớp này để tạo 1 shared_ptr (this) thay vì raw pointer
        template <typename T>
        class connection : public std::enable_shared_from_this<connection<T>> {
        public:
            connection(){}
            virtual ~connection(){}

            bool connectToServer();
            bool disconnect();
            bool isConnected() const;

            bool send(const message<T>& msg);

        protected:
            asio::ip::tcp::socket _socket;

            // Chỉ tham chiếu đến địa chỉ để dùng chung
            asio::io_context& _asioContext;
        
            // Hàng đợi này chứa tất cả các messages gửi đến bên kia của kết nối
            TSQueue<message<T>> _qMsgsOut;

            // Hàng đợi này chứa tất cả các messages đến kết nối. Nó chỉ là tham chiếu
            // vì nó sẽ được quản lí bởi lớp khác
            TSQueue<ownedMessage<T>>& _qMsgsIn;
            
        };
}