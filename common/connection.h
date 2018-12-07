#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace server_base
{
class Connection
{
public:
    explicit Connection(boost::asio::io_service* io_service);

    bool Start();
    void Stop();
    const boost::asio::ip::tcp::endpoint& EndPoint() {
        return m_endpoint;
    }

    boost::asio::ip::tcp::socket& Socket();

private:
    void HandleRead(const boost::system::error_code& e, std::size_t bytes_transf);
    void HandleWrite(const boost::system::error_code& e);

    bool RequestKeepAlive();

private:
    // 连接套字
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::io_service* m_io_service;
    boost::asio::ip::tcp::endpoint m_endpoint;
    boost::asio::strand m_conn_strand;
    boost::array<char, 8192> m_buffer; //默认socket最大一次发送为8k，也可查看系统设置

    std::vector<boost::asio::const_buffer> *sending_buffers;
    std::vector<boost::asio::const_buffer> *need_send_buffers;

    uint32_t  state;                // 连接状态
    // mutex ?
};

}
#endif
