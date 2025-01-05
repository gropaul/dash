#pragma once

#include <string>

namespace duckdb {
struct Uri {
	// FROM https://stackoverflow.com/a/11044337
	std::string QueryString {};
	std::string Path {};
	std::string Protocol {};
	std::string Host {};
	std::string Port {};

	static Uri Parse(const std::string &uri) {
		Uri result {};

		typedef std::string::const_iterator iterator_t;

		if (uri.length() == 0) {
			return result;
		}

		iterator_t uriEnd = uri.end();

		// get query start
		iterator_t queryStart = std::find(uri.begin(), uriEnd, L'?');

		// protocol
		iterator_t protocolStart = uri.begin();
		iterator_t protocolEnd = std::find(protocolStart, uriEnd, L':'); //"://");

		if (protocolEnd != uriEnd) {
			std::string prot = &*(protocolEnd);
			if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
				result.Protocol = std::string(protocolStart, protocolEnd);
				protocolEnd += 3; //      ://
			} else
				protocolEnd = uri.begin(); // no protocol
		} else {
			// no protocol
			protocolEnd = uri.begin();
		}

		// host
		iterator_t hostStart = protocolEnd;
		iterator_t pathStart = std::find(hostStart, uriEnd, L'/'); // get pathStart

		iterator_t hostEnd = std::find(protocolEnd, (pathStart != uriEnd) ? pathStart : queryStart,
		                               L':'); // check for port

		result.Host = std::string(hostStart, hostEnd);

		// port
		if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == L':')) {
			hostEnd++;
			iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
			result.Port = std::string(hostEnd, portEnd);
		}

		// path
		if (pathStart != uriEnd) {
			result.Path = std::string(pathStart, queryStart);
		}

		// query
		if (queryStart != uriEnd) {
			result.QueryString = std::string(queryStart, uri.end());
		}

		if (result.Port.empty()) {
			if (result.Protocol == "http") {
				result.Port = "80";
			} else if (result.Protocol == "https") {
				result.Port = "443";
			}
		}

		return result;
	}

	void AssertValid() const {
		if (Protocol.empty()) {
			throw BinderException("Protocol cannot be empty");
		}

		if (Host.empty()) {
			throw BinderException("Host cannot be empty");
		}

		if (Port.empty()) {
			throw BinderException("Port cannot be empty");
		}

		if (ParsedPort() <= 0) {
			throw BinderException("Port must be a positive integer");
		}
	}

	bool Valid() const {
		try {
			AssertValid();
			return true;
		} catch (...) {
			return false;
		}
	}



	bool operator==(const Uri &uri) const {
		return QueryString == uri.QueryString && Path == uri.Path && Protocol == uri.Protocol && Host == uri.Host &&
		       Port == uri.Port;
	}

	int ParsedPort() const {
		try {
			return std::stoi(Port);
		} catch (...) {
			return -1;
		}
	}

	string ToString() const {
		string result = Protocol + "://" + Host;
		if (Port != "80" && Port != "443") {
			result += ":" + Port;
		}
		result += Path;
		if (!QueryString.empty()) {
			result += "?" + QueryString;
		}
		return result;
	}

	string HostWithProtocol() const {
		string result = Protocol + "://" + Host;
		if (Port != "80" && Port != "443") {
			result += ":" + Port;
		}

		return result;
	}
};
} // namespace duckdb
