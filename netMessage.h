#pragma once
#include "netCommon.h"

namespace net {

    template <typename T>
    struct messageHeader {
        T id{};
        uint32_t size = 0;
    };

    template <typename T>
    struct message {
        messageHeader<T> header{};
        std::vector<uint8_t> body{};

        size_t size() const {
            return sizeof(messageHeader<T>) + body.size(); 
        }

        friend std::ostream& operator<<(std::ostream& os, const message<T>& msg) {
            os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
            return os;
        }

        template <typename DataType>
        friend message<T>& operator<<(message<T>& msg, const DataType& data) {
            // Kiểm tra xem kiểu dữ liệu có tương thích để chứa trong vector hay không
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector.");
            
            // Lưu lại vị trí sẽ chèn data mới vào
            size_t i = msg.body.size();
            
            msg.body.resize(msg.body.size() + sizeof(DataType));

            // Copy data vào vector
            std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

            // Cập nhật kích thước của header
            msg.header.size = msg.size();

            return msg;
        }

        template <typename DataType>
        friend message<T>& operator>>(message<T>& msg, const DataType& data) {
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector.");
            
            // Lưu lại vị trí cuối của vector sau khi đẩy data ra
            size_t i = msg.body.size() - sizeof(DataType);

            std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

            msg.body.resize(i);

            msg.header.size = msg.size();

            return msg;
        }
    };

    template <typename T>
    class connection;

    template <typename T>
    struct ownedMessage {
        std::shared_ptr<connection<T>> remote = nullptr;
        message<T> msg;

        friend std::ostream& operator<<(std::ostream& os, const ownedMessage<T>& msg) {
            os << msg.msg;
            return os;
        }
    };
}