#include <iostream>
#include <sstream>
#include <string>
#include "tcp_connection.h"
#include "task_queue.h"
#include "database.h"
#include "message.h"
#include "ticket.h"

using std::exception;
using std::cerr;
using std::endl;
using std::istringstream;
using asio::io_context;

database db;
ticket service{db};

auto tasks = make_task_queue<message>([](const message &msg) {
	switch (msg.type) {
	case message::new_ticket:
		service.new_ticket(msg);
		return;
	case message::is_valid:
		service.is_valid(msg);
		return;
	case message::tickets_issued:
		service.tickets_issued(msg);
		return;
	case message::invalidate_ticket:
		service.invalidate_ticket(msg);
		return;
	}
}, 16);

class public_connection : public tcp_connection {
	void parse(const string &msg) {
		istringstream in{msg};

		string cmd, user{};
		in >> cmd >> user;
		if (user.empty()) {
			write("User not authenticated\n");
			return;
		}
		if (cmd == "new_ticket")
			tasks.dispatch(message{shared_from_this(),
						   message::new_ticket, user, {}});
		else if (cmd == "is_valid") {
			unsigned long id{};
			in >> id;
			tasks.dispatch(message{shared_from_this(),
						   message::is_valid, user, id});
		} else
			cerr << "Unknown message: " << msg << endl;
	}

public:
	public_connection(io_context &io) : tcp_connection{io} {}
};

class internal_connection : public tcp_connection {
	io_context &io;

	void parse(const string &msg) {
		istringstream in{msg};

		string cmd;
		in >> cmd;
		if (cmd == "tickets_issued") {
			tasks.dispatch(message{shared_from_this(),
						   message::tickets_issued, "", {}});
		} else if (cmd == "invalidate_ticket") {
			unsigned long id{};
			in >> id;
			tasks.dispatch(message{shared_from_this(),
						   message::invalidate_ticket, "", id});
		} else if (cmd == "stop") {
			io.stop();
		} else
			cerr << "Unknown message: " << msg << endl;
	}

public:
	internal_connection(io_context &io) : tcp_connection{io}, io{io} {}
};

unsigned short port(const char *s) {
	istringstream in{s};
	unsigned short n;
	in >> n;
	return n;
}

int main(int argc, char *argv[]) {
	unsigned short public_port{8080};
	unsigned short internal_port{8081};
	if (argc >= 3) internal_port = port(argv[2]);
	if (argc >= 2) public_port = port(argv[1]);

	try {
		io_context io;
		tcp_server<public_connection> public_server{io, public_port};
		tcp_server<internal_connection> internal_server{io, internal_port};
		io.run();
		return 0;
	} catch (exception &e) {
		cerr << e.what() << endl;
		return -1;
	} catch (...) {
		cerr << "Unknown exception" << endl;
		return -1;
	}
}
