#ifndef ticket_h
#define ticket_h

#include <sstream>
#include "message.h"
#include "database.h"

using std::size_t;
using std::ostringstream;

class ticket {
	database &db;

public:
	ticket(database &db) : db{db} {}

	void new_ticket(const message &msg) {
		unsigned long id{db.next_id()};
		if (db.insert(id, msg.user)) {
			ostringstream o;
			o << "Ticket: " << id << '\n';
			msg.from->write(o.str());
		}
	}

	void is_valid(const message &msg) {
		auto [id, user] = db.select(msg.id);
		ostringstream o;
		o << "Ticket: " << id
		  << (id == msg.id && user == msg.user ? " is" : " is NOT")
		  << " valid\n";
		msg.from->write(o.str());
	}

	void tickets_issued(const message &msg) {
		size_t count{db.count()};
		ostringstream o;
		o << count << " tickets issued\n";
		msg.from->write(o.str());
	}

	void invalidate_ticket(const message &msg) {
		if (db.erase(msg.id)) {
			ostringstream o;
			o << "Ticket: " << msg.id << " invalidated\n";
			msg.from->write(o.str());
		}
	}
};

#endif
