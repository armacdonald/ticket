#ifndef database_h
#define database_h

#include <string>
#include <mutex>
#include <map>

using std::size_t;
using std::string;
using std::map;

class database {
	mutable std::mutex mutex;
	unsigned long next{};
	map<unsigned long, string> tickets{};

public:
	struct entry { unsigned long id; string user; };

	unsigned long next_id() {
		std::unique_lock<std::mutex> lock{mutex};
		return ++next;
	}

	std::size_t count() const {
		std::unique_lock<std::mutex> lock{mutex};
		return tickets.size();
	}

	entry select(unsigned long id) const {
		std::unique_lock<std::mutex> lock{mutex};
		auto it{tickets.find(id)};
		return it == tickets.end() ? entry{} : entry{it->first, it->second};
	}

	bool insert(unsigned long id, const string &user) {
		std::unique_lock<std::mutex> lock{mutex};
		return tickets.emplace(id, user).second;
	}

	bool erase(unsigned long id) {
		std::unique_lock<std::mutex> lock{mutex};
		return tickets.erase(id);
	}
};

#endif
