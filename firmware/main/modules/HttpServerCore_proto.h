//
//  HttpServerCore_proto.h
//  SensorClock
//
//  created by yu2924 on 2026-04-25
//

#if defined(_WIN32)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define STRICT
#if !defined NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include <algorithm>
#include <array>
#include <format>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <process.h>

// ================================================================================
// portable helpers

#if defined(_WIN32)
#define strcasecmp	_stricmp
#define strncasecmp _strnicmp
typedef int socklen_t;
#else
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#define closesocket		close
typedef int SOCKET;
#endif

void DebugPrint(char level, const char* fmt, ...)
{
	struct TimeStamp
	{
		static uint32_t getMilliseconds()
		{
			return (uint32_t)((int64_t)clock() * 1000 / CLOCKS_PER_SEC);
		}
	};
	char s[256]{};
	int lpfx = snprintf(s, std::size(s), "%c (%u) ", level, TimeStamp::getMilliseconds());
	va_list va;
	va_start(va, fmt);
	vsnprintf(s + lpfx, std::size(s) - lpfx, fmt, va);
	va_end(va);
	strncat(s, "\n", std::size(s));
	strcpy(s + 254, "\n"); // failsafe
#if defined(_WIN32)
	OutputDebugStringA(s);
	printf(s);
#else
	printf(s);
#endif
}

using stringstringmap = std::map<std::string, std::string>;
using stringstringpair = std::pair<std::string, std::string>;
using stringlist = std::list<std::string>;

static std::string StringTrim(const std::string& s, const std::string& delim)
{
	if(s.length() == 0) return s;
	size_t i0 = s.find_first_not_of(delim);
	if(i0 == std::string::npos) return std::string();
	size_t i1 = s.find_last_not_of(delim);
	return std::string(s, i0, i1 - i0 + 1);
}

static std::string StringTrimTail(const std::string& s, const std::string& delim)
{
	if(s.length() == 0) return s;
	size_t i = s.find_last_not_of(delim);
	return std::string(s, 0, i + 1);
}

static std::string StringTokenize(const std::string& v, const std::string& delim, size_t* ppos)
{
	if(*ppos == std::string::npos) return std::string();
	size_t i0 = v.find_first_not_of(delim, *ppos);
	if(i0 == std::string::npos)
	{
		*ppos = std::string::npos;
		return std::string();
	}
	size_t i1 = v.find_first_of(delim, i0); if(i1 == std::string::npos) i1 = v.size();
	*ppos = v.find_first_not_of(delim, i1); if(*ppos == std::string::npos) *ppos = v.size();
	return std::string(v, i0, i1 - i0);
}

static stringlist StringListBreak(const std::string& v, const std::string& delim)
{
	stringlist cs;
	size_t pos = 0;
	while(true)
	{
		std::string s = StringTokenize(v, delim, &pos); if(pos == std::string::npos) break;
		cs.push_back(s);
	}
	return cs;
}

static stringstringpair StringPairBreak(const std::string& v, const std::string& delim)
{
	size_t pos = 0;
	std::string a = StringTokenize(v, delim, &pos);
	std::string b = v.substr(pos);
	return stringstringpair(a, b);
}

static std::string StringReplace(const std::string& v, const std::string& from, const std::string& to)
{
	std::string vr = v;
	auto pos = vr.find(from);
	if(pos != std::string::npos) { vr.replace(pos, from.size(), to); }
	return vr;
}

// ================================================================================
// HttpServer

// cf. esp-idf/components/http_parser/http_parser.h
enum
{
	HTTP_DELETE = 0,
	HTTP_GET,
	HTTP_HEAD,
	HTTP_POST,
	HTTP_PUT,
};

static const struct { int code; const char* str; } HttpMethodTable[] =
{
	{ HTTP_DELETE	, "DELETE"	},
	{ HTTP_GET		, "GET"		},
	{ HTTP_HEAD		, "HEAD"	},
	{ HTTP_POST		, "POST"	},
	{ HTTP_PUT		, "PUT"		},
};

static const char* HttpMethodGetString(int code)
{
	for(const auto& ent : HttpMethodTable) { if(code == ent.code) { return ent.str; } }
	return "unknown";
}

static int HttpMethodGetCode(const char* text)
{
	for(const auto& ent : HttpMethodTable) { if(strcasecmp(ent.str, text) == 0) { return ent.code; } }
	return -1;
}

static std::string GetPrimaryHostAddress(uint16_t port)
{
	std::string saddr;
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(s != INVALID_SOCKET)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("192.0.2.4"); // RFC3330 TEST-NET
		addr.sin_port = htons(port);
		if(connect(s, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR)
		{
			socklen_t cbaddr = sizeof(addr);
			if((getsockname(s, (sockaddr*)&addr, &cbaddr) != SOCKET_ERROR) && (addr.sin_addr.s_addr != inet_addr("127.0.0.1"))) { saddr = inet_ntoa(addr.sin_addr); }
		}
		closesocket(s);
	}
	return saddr;
}

class HTTPReadBuffer
{
public:
	static const char* memsearch(const char* p, int l, const char* pk, int lk)
	{
		if(l < lk) return NULL;
		const char* pr = std::search(p, p + l, pk, pk + lk);
		if(p + l <= pr) return NULL;
		return pr;
	}
	SOCKET mSock = INVALID_SOCKET;
	std::string mBuf;
	size_t mBufCapacity = 0, mBufSize = 0;
	size_t mSockLength = 0, mSockPos = 0;
	void Initialize(SOCKET s, size_t lbuf)
	{
		mSock = s;
		mBuf.resize(lbuf);
		mBufCapacity = lbuf;
		mBufSize = 0;
		mSockLength = (size_t)-1;
		mSockPos = 0;
	}
	void contentLengthDidDetermined(size_t l)
	{
		mSockLength = l;
		mSockPos = mBufSize; // already read
	}
	bool isEndOfStream() const
	{
		return !mBufSize && (mSockLength <= mSockPos);
	}
	bool readUntil(const std::string& key, std::ostream* ps)
	{
		if(mBufCapacity <= key.size()) return false;
		while(true)
		{
			if(const char* p = memsearch(mBuf.c_str(), (int)mBufSize, key.c_str(), (int)key.size()))
			{
				size_t l = p - mBuf.c_str();
				ps->write(mBuf.c_str(), l);
				mBuf.erase(mBuf.begin(), mBuf.begin() + l + key.size());
				mBuf.resize(mBufCapacity);
				mBufSize -= l + key.size();
				return true;
			}
			if(key.size() < mBufSize)
			{
				size_t l = mBufSize - key.size();
				ps->write(mBuf.c_str(), l);
				mBuf.erase(mBuf.begin(), mBuf.begin() + l);
				mBuf.resize(mBufCapacity);
				mBufSize -= l;
			}
			if(mSockLength <= mSockPos) return false;
			int c = (int)(mBufCapacity - mBufSize);
			int cr = (int)recv(mSock, &mBuf[mBufSize], c, 0);
			if(cr == 0) mSockPos = mSockLength; // eos
			if(cr < 0) return false;
			mBufSize += cr;
			mSockPos += cr;
		}
		// never reached
		return false;
	}
	bool readLine(std::string* ps)
	{
		std::stringstream sl; if(!readUntil("\r\n", &sl)) return false;
		ps->assign(sl.str());
		return true;
	}
};

class HTTPRequestLine
{
public:
	static char* nexttoken(char* p, int delim)
	{
		char* pnext = strchr(p, delim);
		if(pnext) { *pnext = 0; ++pnext; }
		else { pnext = p + strlen(p); };
		return pnext;
	}
	int method = -1;
	std::string methodString; // e.g. "GET"
	std::string resourcePath; // e.g. "/"
	static constexpr size_t MaxQueryParams = 8;
	std::string queryBuffer;
	struct KEYVAL { const char* k; const char* v; };
	std::array<KEYVAL, MaxQueryParams> arrQueryParams;
	size_t numQueryParams = 0;
	std::string protocolVersionString; // e.g. "HTTP/1.1"
	bool read(HTTPReadBuffer& buffer)
	{
		std::string line; if(!buffer.readLine(&line)) return false;
		size_t pos = 0;
		methodString = StringTokenize(line, " ", &pos);
		method = HttpMethodGetCode(methodString.c_str());
		std::string resstr = StringTokenize(line, " ", &pos);
		stringstringpair respair = StringPairBreak(resstr, "?");
		resourcePath = respair.first;
		queryBuffer = respair.second;
		numQueryParams = 0;
		char* prm = &queryBuffer[0];
		char* resend = prm + strlen(prm);
		while(prm < resend)
		{
			if(arrQueryParams.size() <= numQueryParams) break;
			char* prmnext = nexttoken(prm, '&');
			char* pv = nexttoken(prm, '=');
			arrQueryParams[numQueryParams] = { prm, pv };
			++numQueryParams;
			prm = prmnext;
		}
		protocolVersionString = StringTokenize(line, " ", &pos);
		return true;
	}
	size_t getNumQueryParameters() const { return numQueryParams; }
	const KEYVAL& getQueryParameterAt(size_t i) const { return arrQueryParams[i]; }
	const char* lookupQueryParameter(const char* k) const
	{
		for(size_t c = numQueryParams, i = 0; i < c; ++i) { if(strcasecmp(arrQueryParams[i].k, k) == 0) return arrQueryParams[i].v; }
		return nullptr;
	}
};

class HTTPRequestHeader
{
public:
	stringstringmap fieldMap;
	bool read(HTTPReadBuffer& buffer)
	{
		fieldMap.clear();
		while(true)
		{
			std::string line; if(!buffer.readLine(&line)) return false;
			if(line.empty()) break;
			stringstringpair ss = StringPairBreak(line, ":");
			ss.first = StringTrim(ss.first, "\r\n\t ");
			ss.second = StringTrim(ss.second, "\r\n\t ");
			fieldMap.insert(ss);
		}
		return true;
	}
	std::string getFieldValueForKey(const std::string& k) const
	{
		stringstringmap::const_iterator i = fieldMap.find(k);
		if(i != fieldMap.end()) return i->second;
		return {};
	}
};

struct HTTPResponseHeader
{
	std::string host;
	std::string hdr;
	bool sent = false;
	void initialize(const char* h)
	{
		host = h;
		sent = false;
	}
	void set(const char* statustext, const char* mimetype)
	{
		hdr += std::format("HTTP/1.1 {}\r\n", statustext);
		hdr += std::format("Host: {}\r\n", host);
		hdr += "{CONTENTLENGTH}\r\n";
		hdr += std::format("Content-Type: {}\r\n", mimetype);
		hdr += "Connection: close\r\n";
		hdr += "Cache-Control: no-cache\r\n";
		hdr += "\r\n";
	}
	void fixupContentLength(size_t len, bool chunked)
	{
		if(chunked) hdr = StringReplace(hdr, "{CONTENTLENGTH}", "Transfer-Encoding: chunked");
		else hdr = StringReplace(hdr, "{CONTENTLENGTH}", std::format("Content-Length: {}", len));
	}
};

struct HTTPContentContext
{
	SOCKET mSockConnection = INVALID_SOCKET;
	HTTPReadBuffer readBuffer;
	HTTPRequestLine requestLine;
	HTTPRequestHeader requestHeader;
	HTTPResponseHeader responseHeader;
	HTTPContentContext() = delete;
	HTTPContentContext(SOCKET sock, const char* host)
	{
		mSockConnection = sock;
		responseHeader.initialize(host);
	}
	int getRequestMethod() const { return requestLine.method; }
	const char* getRequestPath() const { return requestLine.resourcePath.c_str(); }
	size_t getRequestNumQueryParameters() const { return requestLine.getNumQueryParameters(); }
	const HTTPRequestLine::KEYVAL& getRequestQueryParameterAt(size_t i) const { return requestLine.getQueryParameterAt(i); }
	const char* lookupRequestQueryParameter(const char* k) const { return requestLine.lookupQueryParameter(k); }
	void setResponseHeader(const char* statustext, const char* mimetype) { responseHeader.set(statustext, mimetype); }
	bool sendSimpleResponse(const char* body, size_t bodylen = (size_t)-1)
	{
		if(bodylen == (size_t)-1) bodylen = strlen(body);
		if(!responseHeader.sent)
		{
			responseHeader.fixupContentLength(bodylen, false);
			if((size_t)send(mSockConnection, responseHeader.hdr.c_str(), (int)responseHeader.hdr.size(), 0) < responseHeader.hdr.size()) { return false; }
			responseHeader.sent = true;
		}
		if(!sendRespBody(body, bodylen)) return false;
		return true;
	}
	bool sendChunkedResponse(const char* body, size_t bodylen = (size_t)-1)
	{
		if(!responseHeader.sent)
		{
			responseHeader.fixupContentLength(0, true);
			if((size_t)send(mSockConnection, responseHeader.hdr.c_str(), (int)responseHeader.hdr.size(), 0) < responseHeader.hdr.size()) { return false; }
			responseHeader.sent = true;
		}
		if(bodylen == (size_t)-1) bodylen = strlen(body);
		if(!sendRespChunk(body, bodylen)) return false;
		return true;
	}
	bool sendRespBody(const char* pbody, size_t lbody)
	{
		if(!pbody || !lbody) { return true; }
		if((size_t)send(mSockConnection, pbody, (int)lbody, 0) < lbody) { return false; }
		return true;
	}
	bool sendRespChunk(const char* pck, size_t lck)
	{
		std::string sz = std::format("{:x}\r\n", lck);
		if((size_t)send(mSockConnection, sz.c_str(), (int)sz.size(), 0) < sz.size()) { return false; }
		if(0 < lck) { if((size_t)send(mSockConnection, pck, (int)lck, 0) < lck) { return false; } }
		if((size_t)send(mSockConnection, "\r\n", 2, 0) < 2) { return false; }
		return true;
	}
};

struct IHTTPContentHandler
{
	virtual ~IHTTPContentHandler() {}
	virtual bool handleContent(HTTPContentContext* ctx) = 0;
};

class HttpServerCore
{
public:
private:
	static constexpr size_t ReceiveBufferSize = 8192;
	std::string mHost;
	uint16_t mPort = 0;
	SOCKET mSockListen = INVALID_SOCKET;
	SOCKET mSockConnection = INVALID_SOCKET;
	HANDLE mhThread = NULL;
	IHTTPContentHandler* contentHandler = nullptr;
public:
	HttpServerCore()
	{
	}
	~HttpServerCore()
	{
		stop();
	}
	void setCredentials(const char* user, const char* pass) {}
	bool addContentPathAndMethod(const char* path, int method) { return true; }
	bool start(const char* hostname, uint16_t port, IHTTPContentHandler* handler)
	{
		stop();
		contentHandler = handler;
		bool r = false;
		if((mSockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET)
		{
			int reuseaddr = 1; setsockopt(mSockListen, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr, sizeof(reuseaddr));
			mHost = GetPrimaryHostAddress(port);
			mPort = port;
			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = htons(mPort);
			if(bind(mSockListen, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR)
			{
				if(listen(mSockListen, 1) != SOCKET_ERROR)
				{
					mhThread = (HANDLE)_beginthreadex(nullptr, 0, threadProc, this, 0, nullptr);
					if(!mhThread) { DebugPrint('W', "HttpServerCore: _beginthreadex() failed"); }
					r = mhThread != NULL;
				}
				else { DebugPrint('W', "HttpServerCore: listensocket listen() failed"); }
			}
			else { DebugPrint('W', "HttpServerCore: listensocket bind() failed"); }
		}
		else { DebugPrint('W', "HttpServerCore: listensocket socket() failed"); }
		if(!r) { stop(); }
		return r;
	}
	void stop()
	{
		if(mSockListen != INVALID_SOCKET) { closesocket(mSockListen); }
		if(mSockConnection != INVALID_SOCKET) { closesocket(mSockConnection); }
		if(mhThread)
		{
			WaitForSingleObject(mhThread, INFINITE);
			CloseHandle(mhThread);
		}
		mhThread = NULL;
		mSockConnection = INVALID_SOCKET;
		mSockListen = INVALID_SOCKET;
		mHost = {};
		mPort = 0;
		contentHandler = nullptr;
	}
	bool isRunning() const
	{
		return mSockListen != INVALID_SOCKET;
	}
private:
	static unsigned int threadProc(void* arg)
	{
		if(HttpServerCore* p = (HttpServerCore*)arg) { p->runThread(); }
		return 0;
	}
	void runThread()
	{
		DebugPrint('I', "HttpServerCore: thread begin, host=%s port=%u", mHost.c_str(), mPort);
		while(true)
		{
			sockaddr_in addr;
			socklen_t cbaddr = sizeof(addr);
			if((mSockConnection = accept(mSockListen, (sockaddr*)&addr, &cbaddr)) != INVALID_SOCKET)
			{
				// DebugPrint('I', "HttpServerCore: connection begin, client=%s", inet_ntoa(addr.sin_addr));
				LINGER opt{ 1, 5 };
				setsockopt(mSockConnection, SOL_SOCKET, SO_LINGER, (const char*)&opt, (int)sizeof(opt));
				handleConnection();
				closesocket(mSockConnection);
				mSockConnection = INVALID_SOCKET;
				// DebugPrint('I', "HttpServerCore: connection end");
			}
			else break;
		}
		DebugPrint('I', "HttpServerCore: thread end");
	}
	bool handleConnection()
	{
		bool r = false;
		try
		{
			HTTPContentContext ctx(mSockConnection, mHost.c_str());
			ctx.readBuffer.Initialize(mSockConnection, ReceiveBufferSize);
			if(ctx.requestLine.read(ctx.readBuffer))
			{
				if(ctx.requestHeader.read(ctx.readBuffer))
				{
					size_t content_length = strtol(ctx.requestHeader.getFieldValueForKey("Content-Length").c_str(), nullptr, 0);
					if(!content_length) content_length = (size_t)-1;
					ctx.readBuffer.contentLengthDidDetermined(content_length);
					DebugPrint('I', "HttpServerCore: method=\"%s\" version=\"%s\" resource=\"%s\"", ctx.requestLine.methodString.c_str(), ctx.requestLine.protocolVersionString.c_str(), ctx.requestLine.resourcePath.c_str());
					for(size_t c = ctx.getRequestNumQueryParameters(), i = 0; i < c; ++i) { const auto& kv = ctx.getRequestQueryParameterAt(i); DebugPrint('I', "  \"%s\"=\"%s\"", kv.k, kv.v); }
					HTTPResponseHeader resphdr(mHost);
					r = contentHandler->handleContent(&ctx);
					DebugPrint('I', "HttpServerCore: end response");
				}
			}
		}
		catch(std::exception& e)
		{
			DebugPrint('I', "HttpServerCore: exception reason=%s", e.what());
			r = false;
		}
		return r;
	}
};
