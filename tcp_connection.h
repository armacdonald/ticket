#ifndef tcp_connection_h
#define tcp_connection_h

#include <iostream>
#include <string>
#include <queue>
#include <asio.hpp>

using std::size_t;
using std::cerr;
using std::endl;
using std::string;
using std::queue;
using std::enable_shared_from_this;
using std::make_shared;
using asio::io_context;
using asio::buffer;
using asio::streambuf;
using asio::error_code;
using asio::async_read_until;
using asio::async_write;
using asio::buffers_begin;
using asio::ip::tcp;

class tcp_connection : public enable_shared_from_this<tcp_connection> {
	tcp::socket sock;
	streambuf buf;
	queue<string> output;
	std::mutex mutex;

protected:
	virtual void parse(const string &msg) = 0;

public:
	tcp_connection(io_context &io) : sock{io} {}

	tcp::socket &socket() noexcept { return sock; }

	void read() {
		async_read_until(sock, buf, '\n',
			[self = shared_from_this()](const error_code &error, size_t count) {
				if (count <= 1) {
					if (error) cerr << error << endl;
					return;
				}
				string msg{buffers_begin(self->buf.data()),
				           buffers_begin(self->buf.data()) + count - 1};
				self->parse(msg);
				self->buf.consume(count);
				self->read();
			});
	}

	void write(const string &msg) {
		std::unique_lock<std::mutex> lock{mutex};
		output.push(msg);
		async_write(sock, buffer(output.back()),
			[self = shared_from_this()](const error_code &, size_t) {
				std::unique_lock<std::mutex> lock{self->mutex};
				self->output.pop();
			});
	}
};

template<typename T> class tcp_server {
	io_context &io;
	tcp::acceptor acceptor;

	void accept() {
		auto connection{make_shared<T>(io)};
		acceptor.async_accept(connection->socket(),
			[=](const error_code &error) {
				if (!error) connection->read();
				accept();
			});
	}

public:
	tcp_server(io_context &io, unsigned short port) :
		io{io}, acceptor{io, tcp::endpoint{tcp::v4(), port}} { accept(); }
};

#endif
