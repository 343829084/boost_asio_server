#include "connection.h"
#include <boost/bind.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>
#include <boost/make_shared.hpp>
#include <iostream>

namespace server_base
{

Connection::Connection(boost::asio::io_service* io_service, HandlerPtr handler)
    :m_socket(io_service),
    m_io_service(io_service),
    m_conn_strand(io_service)
{
}

bool Connection::Start()
{
    try {
        m_endpoint = m_socket.remote_endpoint();
    } catch(...) {
        std::cout << "call remote_endpoint failed!" << std::endl;
        return false;
    }
    m_socket.async_read_some(boost::asio::buffer(m_buffer),
        m_conn_strand.wrap(
            boost::bind(&Connection::HandleRead, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
    return true;
}


void Connection::Stop()
{
    std::cout << "stop connection" << std::endl;
    m_socket.close();
}


void Connection::HandleWrite(const boost::system::error_code& e)
{
    if (!e) {
        sending_buffers->clear();
        AsyncSend();
    } else {
        //log socket write failed!
    }
}

void Connection::AsyncSend(const char* buf, size_t len, size_t param)
{
    if (buf != NULL) {
        need_send_buffers->push_back(boost::asio::buffer(buf, len));
    }
    if (sending_buffers->empty()) {
        std::swap(sending_buffers, need_send_buffers);
        if (!(sending_buffers->empty())) {
            if (!(m_socket.is_open())) {
                //log
                return;
            }
            m_socket.async_write(*sending_buffers,
                        m_conn_strand.wrap(
                            boost::bind(&Connection::HandleWrite, shared_from_this(),
                            boost::asio::placeholders::error, param));
        }

    }
}


void Connection::HandleRead(const boost::system::error_code& e, std::size_t bytes_transf)
{
    // 在handleconnection时取消握手定时
    // timer->cancel(); fix me
    size_t packet_len = 0;
    if (!e) {
        buffer_all_.append(m_buffer.data(), bytes_transf);
        while(true) { // 解包
            packet_len = HasWholePacket(); //
            if (packet_len > 0) { // 完整的包
                // fix me, 二次拷贝，需要修复
                DoHandleMessage(buffer_all_, packet_len);
                buffer_all_ = buffer_all_.substr(ret);
            } else if (ret == 0) {
                m_socket.async_read_some(boost::asio::buffer(m_buffer),
                m_conn_strand.wrap(
                    boost::bind(&Connection::HandleRead, shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred)));
                break;
            } else { //解包出问题
                Stop(); // 断开连接
                break;
            }
        }
    }else {
        Stop();
    }
}


} // namespace server_base

