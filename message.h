#ifndef message_h
#define message_h

#include <string>
#include "tcp_connection.h"

using std::string;
using std::shared_ptr;

struct message {
	shared_ptr<tcp_connection> from;
	enum {
		new_ticket,
		is_valid,
		tickets_issued,
		invalidate_ticket
	} type;
	string user;
	unsigned long id;
};

#endif
