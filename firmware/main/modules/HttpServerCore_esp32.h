//
//  HttpServerCore_esp32.h
//  SensorClock
//
//  created by yu2924 on 2026-04-26
//

#include <esp_http_server.h>
#include <esp_https_server.h>
#include <esp_log.h>
#include <esp_tls_crypto.h>
#include <mdns.h>
#include <mutex>

#if __has_include("modules/HttpServerCore.config.h")
#include "modules/HttpServerCore.config.h"
#endif

static const char* TagSvr = "Svr";

class MdnsService
{
	public:
	MdnsService()
	{
	}
	~MdnsService()
	{
		stop();
	}
	bool start(const char* hostname, const char* devicename = nullptr)
	{
		// mDNS Properties
		ESP_ERROR_CHECK_WITHOUT_ABORT(mdns_init());
		ESP_ERROR_CHECK_WITHOUT_ABORT(mdns_hostname_set(hostname)); // hostname: will resolve to "SensorClock.local"
		if(devicename) { ESP_ERROR_CHECK_WITHOUT_ABORT(mdns_instance_name_set(devicename)); } // default_instance: friendly name for the device
		// mDNS Services
		static const mdns_txt_item_t TxtData[3] = {
			{"board", "esp32"},
			{"u", "user"},
			{"p", "password"}
		};
		ESP_ERROR_CHECK_WITHOUT_ABORT(mdns_service_add(nullptr, "_http", "_tcp", 80, const_cast<mdns_txt_item_t*>(TxtData), std::size(TxtData)));
		ESP_ERROR_CHECK_WITHOUT_ABORT(mdns_service_subtype_add_for_host(nullptr, "_http", "_tcp", nullptr, "_server"));
#if HTTPSERVER_SUPPORT_HTTPS
		ESP_ERROR_CHECK_WITHOUT_ABORT(mdns_service_add(nullptr, "_https", "_tcp", 443, const_cast<mdns_txt_item_t*>(TxtData), std::size(TxtData)));
		ESP_ERROR_CHECK_WITHOUT_ABORT(mdns_service_subtype_add_for_host(nullptr, "_https", "_tcp", nullptr, "_server"));
#endif
		return true;
	}
	void stop()
	{
		mdns_free();
	}
};

struct HTTPContentContext
{
	static char* nexttoken(char* p, int delim)
	{
		char* pnext = strchr(p, delim);
		if(pnext) { *pnext = 0; ++pnext; }
		else { pnext = p + strlen(p); };
		return pnext;
	}
	httpd_req_t* request = nullptr;
	static constexpr size_t MaxParams = 8;
	strutil::Str<CONFIG_HTTPD_MAX_URI_LEN> resBuffer;
	const char* resourcePath = nullptr;
	struct KEYVAL { const char* k; const char* v; };
	std::array<KEYVAL, MaxParams> arrQueryParams;
	size_t numQueryParams = 0;
	HTTPContentContext() = delete;
	HTTPContentContext(httpd_req_t* req)
	{
		request = req;
		ESP_LOGI(TagSvr, "uri=\"%s\"", request->uri);
		resBuffer = request->uri; // e.g. "/path?query1=value1&query2=value2&..."
		// break the path and query parameters manually
		char* path = resBuffer.getBuffer();
		char* resend = path + strlen(path);
		char* prm = nexttoken(path, '?');
		resourcePath = path;
		ESP_LOGI(TagSvr, "  resourcePath=\"%s\"", resourcePath);
		while(prm < resend)
		{
			if(arrQueryParams.size() <= numQueryParams) break;
			char* prmnext = nexttoken(prm, '&');
			char* pv = nexttoken(prm, '=');
			arrQueryParams[numQueryParams] = { prm, pv };
			++numQueryParams;
			ESP_LOGI(TagSvr, "  param \"%s\"=\"%s\"", prm, pv);
			prm = prmnext;
		}
	}
	int getRequestMethod() const { return request->method; }
	const char* getRequestPath() const { return resourcePath; }
	size_t getRequestNumQueryParameters() const { return numQueryParams; }
	KEYVAL getRequestQueryParameterAt(size_t i) const { return (i < numQueryParams) ? arrQueryParams[i] : KEYVAL{}; }
	const char* lookupRequestQueryParameter(const char* k) const
	{
		for(size_t c = numQueryParams, i = 0; i < c; ++i) { if(strcasecmp(arrQueryParams[i].k, k) == 0) { return arrQueryParams[i].v; } }
		return nullptr;
	}
	void setResponseHeader(const char* statustext, const char* mimetype)
	{
		httpd_resp_set_status(request, statustext);
		httpd_resp_set_type(request, mimetype);
		httpd_resp_set_hdr(request, "Connection", "close");
		httpd_resp_set_hdr(request, "Cache-Control", "no-cache");
	}
	bool sendSimpleResponse(const char* body, size_t bodylen = (size_t)-1)
	{
		if(bodylen == (size_t)-1) { bodylen = HTTPD_RESP_USE_STRLEN; }
		return httpd_resp_send(request, body, bodylen) == ESP_OK;
	}
	bool sendChunkedResponse(const char* body, size_t bodylen = (size_t)-1)
	{
		if(bodylen == (size_t)-1) { bodylen = HTTPD_RESP_USE_STRLEN; }
		return httpd_resp_send_chunk(request, body, bodylen) == ESP_OK;
	}
};

struct IHTTPContentHandler
{
	virtual ~IHTTPContentHandler() {}
	virtual bool handleContent(HTTPContentContext* ctx) = 0;
};

class HttpServerCore
{
private:
	static int createAuthTypeAndCredentials(const char* username, const char* password, char* pbuf, size_t lbuf)
	{
		static const char* TypeStr = "Basic ";
		constexpr size_t TypeLen = 6;
		size_t lcred = strlen(username) + strlen(password) + 1;
		char* pcred = (char*)alloca(lcred + 1);
		snprintf(pcred, lcred + 1, "%s:%s", username, password);
		size_t len = 0;
		if(!pbuf || !lbuf)
		{
			// NOTE: base64 size increase is approximately 30% to 60%
			esp_crypto_base64_encode(nullptr, 0, &len, (const unsigned char*)pcred, lcred);
			return (int)(TypeLen + len);
		}
		if(lbuf <= TypeLen) { return -1; }
		strcpy(pbuf, TypeStr);
		esp_crypto_base64_encode((unsigned char*)pbuf + TypeLen, lbuf - TypeLen, &len, (const unsigned char*)pcred, lcred);
		return (int)(TypeLen + len);
	}
	strutil::Str<64> authTypeAndcredentials;
	IHTTPContentHandler* contentHandler = nullptr;
	httpd_handle_t hServer = NULL;
	MdnsService mdnsService;
	static constexpr size_t MaxContentPathEntries = 4;
	struct ContentPathEntry { const char* path; int method; };
	std::array<ContentPathEntry, MaxContentPathEntries> arrContentPathEntries;
	size_t numContentPathEntries = 0;
	std::recursive_mutex mtx;
public:
	HttpServerCore()
	{
	}
	~HttpServerCore()
	{
		stop();
	}
	void setCredentials(const char* user, const char* pass)
	{
		std::lock_guard sl(mtx);
		const strutil::Str<16> username = user ? user : "";
		const strutil::Str<16> password = pass ? pass : "";
		authTypeAndcredentials = "";
		if(!username.isEmpty() && !password.isEmpty()) { createAuthTypeAndCredentials(username, password, authTypeAndcredentials.getBuffer(), authTypeAndcredentials.capacity()); }
	}
	bool addContentPathAndMethod(const char* path, int method)
	{
		if(MaxContentPathEntries <= numContentPathEntries) { return false; }
		arrContentPathEntries[numContentPathEntries] = { path, method };
		++numContentPathEntries;
		if(hServer)
		{
			httpd_uri_t uri =
			{
				.uri = path,
				.method = (httpd_method_t)method,
				.handler = contentHandlerProc,
				.user_ctx = this
			};
			httpd_register_uri_handler(hServer, &uri);
		}
		return true;
	}
	bool start(const char* hostname, uint16_t port, IHTTPContentHandler* handler)
	{
		stop();
		contentHandler = handler;
		esp_err_t r = ESP_OK;
#if HTTPSERVER_SUPPORT_HTTPS
		if(port == 443)
		{
			// NOTE: the main/CMakeLists.txt includes the entry { EMBED_TXTFILES "certs/servercert.pem" "certs/prvtkey.pem" }
			extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
			extern const unsigned char servercert_end[]   asm("_binary_servercert_pem_end");
			extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
			extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
			httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
			conf.servercert = servercert_start;
			conf.servercert_len = servercert_end - servercert_start;
			conf.prvtkey_pem = prvtkey_pem_start;
			conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;
			r = httpd_ssl_start(&hServer, &conf);
		}
		else
#endif
		{
			httpd_config_t conf = HTTPD_DEFAULT_CONFIG();
			conf.server_port = port;
			conf.lru_purge_enable = true;
			r = httpd_start(&hServer, &conf);
		}
		if(r != ESP_OK)
		{
			ESP_LOGW(TagSvr, "failed to starting server, %s", esp_err_to_name(r));
			stop();
			return false;
		}
		for(size_t c = numContentPathEntries, i = 0; i < c; ++i)
		{
			const ContentPathEntry& ent = arrContentPathEntries[i];
			httpd_uri_t uri =
			{
				.uri = ent.path,
				.method = (httpd_method_t)ent.method,
				.handler = contentHandlerProc,
				.user_ctx = this
			};
			httpd_register_uri_handler(hServer, &uri);
		}
		esp_event_handler_register(ESP_HTTP_SERVER_EVENT, ESP_EVENT_ANY_ID, serverEventHandler, this);
		esp_event_handler_register(ESP_HTTPS_SERVER_EVENT, ESP_EVENT_ANY_ID, serverEventHandler, this);
		mdnsService.start(hostname, nullptr);
		return true;
	}
	void stop()
	{
		mdnsService.stop();
		if(hServer)
		{
			esp_event_handler_unregister(ESP_HTTPS_SERVER_EVENT, ESP_EVENT_ANY_ID, serverEventHandler);
			esp_event_handler_unregister(ESP_HTTP_SERVER_EVENT, ESP_EVENT_ANY_ID, serverEventHandler);
			for(size_t c = numContentPathEntries, i = 0; i < c; ++i)
			{
				const ContentPathEntry& ent = arrContentPathEntries[i];
				httpd_unregister_uri(hServer, ent.path);
			}
			httpd_stop(hServer);
		}
		hServer = NULL;
		contentHandler = nullptr;
	}
	bool isRunning() const
	{
		return hServer != NULL;
	}
private:
	static void serverEventHandler(void* arg, esp_event_base_t base, int32_t evid, void* evdata)
	{
		if(HttpServerCore* p = (HttpServerCore*)arg) { p->onServerEvent(base, evid, evdata); }
	}
	void onServerEvent(esp_event_base_t base, int32_t evid, void* evdata)
	{
		if(base == ESP_HTTP_SERVER_EVENT)
		{
			switch(evid)
			{
				// case HTTP_SERVER_EVENT_ERROR: ESP_LOGI(TagSvr, "Event: ERROR"); break;
				case HTTP_SERVER_EVENT_START: ESP_LOGI(TagSvr, "Event: START"); break;
				case HTTP_SERVER_EVENT_STOP: ESP_LOGI(TagSvr, "Event: STOP"); break;
			}
		}
		else if(base == ESP_HTTPS_SERVER_EVENT)
		{
			if (evid == HTTPS_SERVER_EVENT_ERROR)
			{
				if(esp_https_server_last_error_t* lasterror = (esp_tls_last_error_t*)evdata)
				{
					ESP_LOGE(TagSvr, "HTTPS Error: error=%s tls_err=%d tls_flag=%d", esp_err_to_name(lasterror->last_error), lasterror->esp_tls_error_code, lasterror->esp_tls_flags);
				}
			}
		}
	}
	bool handleAuthorization(httpd_req_t* req)
	{
		bool authorized = false;
		{
			std::lock_guard sl(mtx);
			if(authTypeAndcredentials.isEmpty()) { return true; }
			size_t lauth = httpd_req_get_hdr_value_len(req, "Authorization");
			if(0 < lauth)
			{
				char* pauth = (char*)alloca(lauth + 1);
				httpd_req_get_hdr_value_str(req, "Authorization", pauth, lauth + 1);
				authorized = strncmp(authTypeAndcredentials, pauth, lauth) == 0;
			}
		}
		if(!authorized)
		{
			httpd_resp_set_status(req, "401 Unauthorized");
			httpd_resp_set_type(req, "application/json");
			httpd_resp_set_hdr(req, "Connection", "keep-alive");
			httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"history\"");
			httpd_resp_send(req, nullptr, 0);
		}
		return authorized;
	}
	static esp_err_t contentHandlerProc(httpd_req_t* req)
	{
		if(HttpServerCore* p = (HttpServerCore*)req->user_ctx) { return p->handleContent(req); }
		return ESP_FAIL;
	}
	esp_err_t handleContent(httpd_req_t* req)
	{
		if(!contentHandler) { return ESP_FAIL; }
		if(!handleAuthorization(req)) return ESP_OK;
		HTTPContentContext ctx(req);
		return contentHandler->handleContent(&ctx) ? ESP_OK : ESP_FAIL;
	}
};
