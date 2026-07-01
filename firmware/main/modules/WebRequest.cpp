//
//  WebRequest.cpp
//  SensorClock
//
//  created by yu2924 on 2025-11-23
//

#include "WebRequest.h"

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows

#define STRICT
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <comdef.h>
#include <process.h>
#include "utilities/platform.h"
#include "modules/Dispatcher.h"

struct ScopedCoinitialize
{
	ScopedCoinitialize() { if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) { DEBUGPRINTW("CoInitializeEx() failed"); } }
	~ScopedCoinitialize() { CoUninitialize(); }
};

_COM_SMARTPTR_TYPEDEF(IStream, __uuidof(IStream));
_COM_SMARTPTR_TYPEDEF(IDispatch, __uuidof(IDispatch));
static const LPCWSTR StrClsidWinhttpRequest = L"{2087c2f4-2cef-4953-a8ab-66779b670495}";

HRESULT InvokeDisp(IDispatch* disp, WORD flags, LPCOLESTR name, VARIANT* pargs, UINT cargs, VARIANT* result)
{
	_bstr_t bname(name);
	LPOLESTR pname = bname;
	DISPID dispid = 0;
	HRESULT r = disp->GetIDsOfNames(IID_NULL, &pname, 1, LOCALE_USER_DEFAULT, &dispid);
	if(FAILED(r)) { return r; }
	DISPPARAMS dispparams = { pargs, nullptr, cargs, 0 };
	return disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, flags, &dispparams, result, nullptr, nullptr);
}

class WebRequestImpl : public WebRequest
{
private:
	IDispatchPtr dispHttpRequest;
	HANDLE hThread = NULL;
	std::function<void(const Response&)> onFinish;
	bool aborted = false;
public:
	WebRequestImpl()
	{
	}
	~WebRequestImpl() override
	{
		cleanup();
	}
	bool isRunning() const override
	{
		return hThread != NULL;
	}
	bool start(const char* url, std::function<void(const Response&)> onfinish) override
	{
		if(hThread) return false;
		onFinish = onfinish;
		aborted = false;
		HRESULT r = dispHttpRequest.CreateInstance(StrClsidWinhttpRequest);
		if(FAILED(r)) { return false; }
		// NOTE: place these parameters in reverse order { method=GET, url, async=false }
		_variant_t params[] = { _variant_t(false), _variant_t(url), _variant_t(L"GET") };
		_variant_t res;
		r = InvokeDisp(dispHttpRequest, DISPATCH_METHOD, L"Open", params, std::size(params), &res);
		if(FAILED(r)) { return false; }
		hThread = (HANDLE)_beginthreadex(nullptr, 0, threadProc, this, 0, nullptr);
		if(!hThread) { cleanup(); return false; }
		return true;
	}
	bool isAborted() const override
	{
		return aborted;
	}
	void abort() override
	{
		aborted = true;
		if(dispHttpRequest)
		{
			_variant_t res;
			InvokeDisp(dispHttpRequest, DISPATCH_METHOD, L"Abort", nullptr, 0, &res);
		}
	}
private:
	void cleanup()
	{
		abort();
		if(hThread)
		{
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			hThread = NULL;
		}
		dispHttpRequest = nullptr;
		onFinish = nullptr;
		aborted = false;
	}
	static unsigned int __stdcall threadProc(void* p)
	{
		return ((WebRequestImpl*)p)->runThread();
	}
	unsigned int runThread()
	{
		ScopedCoinitialize coinit;
		_variant_t res;
		HRESULT r = InvokeDisp(dispHttpRequest, DISPATCH_METHOD, L"Send", nullptr, 0, &res);
		if(FAILED(r)) { DEBUGPRINTW("[WebRequest] IWinHttpRequest::Send() failed"); }
		Dispatcher::getMainDispatcher()->callAsync([this]()
		{
			if(aborted) { cleanup(); return; }
			_variant_t vstatus;
			InvokeDisp(dispHttpRequest, DISPATCH_PROPERTYGET, L"Status", nullptr, 0, &vstatus);
			vstatus.ChangeType(VT_I4);
			Response response{};
			response.status = vstatus.lVal;
			// NOTE: don't use ResponseText because it will return as ANSI encoding instead of UTF-8
			_variant_t vbody;
			InvokeDisp(dispHttpRequest, DISPATCH_PROPERTYGET, L"ResponseStream", nullptr, 0, &vbody);
			if(vbody.vt == VT_UNKNOWN)
			{
				if(IStreamPtr str = vbody.punkVal)
				{
					STATSTG stat{}; str->Stat(&stat, STATFLAG_DEFAULT);
					ULONG cb = (ULONG)stat.cbSize.QuadPart;
					response.body.resize(cb);
					ULONG cbr = 0; str->Read(response.body.data(), cb, &cbr);
				}
				else { DEBUGPRINTW("[WebRequest] get_ResponseStream: E_NOINTERFACE"); }
			}
			else { DEBUGPRINTW("[WebRequest] get_ResponseStream(): unexpected VARTYPE"); }
			if(onFinish) { onFinish(response); }
			cleanup();
		});
		return 0;
	}
};

xheap::uniquePtr<WebRequest> WebRequest::createInstance()
{
	return xheap::makeUnique<WebRequest, WebRequestImpl>();
}

#elif defined(ESP_PLATFORM)

// ================================================================================
// Production code for ESP32

#include <esp_crt_bundle.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_tls.h>
#include "modules/Dispatcher.h"

static const char* TagWebRequest = "WebRequest";

class WebRequestImpl : public WebRequest
{
private:
	static constexpr UBaseType_t TaskPriority = ESP_TASK_MAIN_PRIO + 1;
	static constexpr uint32_t TaskStackDepth = 8192;
	static constexpr int64_t MaxResponseSize = 16 * 1024;
	TaskHandle_t hTask = NULL;
	esp_http_client_handle_t hHttpClient = NULL;
	Response response{};
	std::function<void(const Response&)> onFinish;
	bool aborted = false;
public:
	WebRequestImpl()
	{
		// ESP_LOGI(TagWebRequest, "construct");
		response.body.reserve(MaxResponseSize);
	}
	~WebRequestImpl() override
	{
		if(hTask) { vTaskDelete(hTask); }
		if(hHttpClient) { esp_http_client_cleanup(hHttpClient); }
		// ESP_LOGI(TagWebRequest, "destruct");
	}
private:
	static esp_err_t httpEventHandler(esp_http_client_event_t* ev)
	{
		if(WebRequestImpl* p = (WebRequestImpl*)ev->user_data) { return p->onHttpEvent(ev); }
		else { return ESP_FAIL; }
	}
	esp_err_t onHttpEvent(esp_http_client_event_t* ev)
	{
		switch(ev->event_id)
		{
			case HTTP_EVENT_ERROR: ESP_LOGI(TagWebRequest, "HTTP_EVENT_ERROR"); break;
			case HTTP_EVENT_ON_CONNECTED: break; // ESP_LOGI(TagWebRequest, "HTTP_EVENT_ON_CONNECTED"); break;
			case HTTP_EVENT_HEADER_SENT: break; // ESP_LOGI(TagWebRequest, "HTTP_EVENT_HEADER_SENT"); break;
			case HTTP_EVENT_ON_HEADER: break; // ESP_LOGI(TagWebRequest, "HTTP_EVENT_ON_HEADER key=%s value=%s", ev->header_key, ev->header_value); break;
			case HTTP_EVENT_ON_DATA:
			{
				// ESP_LOGI(TagWebRequest, "HTTP_EVENT_ON_DATA datalen=%d", ev->data_len);
				if(response.body.empty())
				{
					int64_t content_length = esp_http_client_get_content_length(ev->client);
					//ESP_LOGI(TagWebRequest, "  content_length=%lld", content_length);
					if((0 < content_length) && (content_length <= MaxResponseSize))
					{
						if(!response.body.reserve((size_t)content_length)) { return ESP_ERR_NO_MEM; }
					}
				}
				if(response.body.size() + ev->data_len <= (size_t)MaxResponseSize)
				{
					if(!response.body.append((const uint8_t*)ev->data, ev->data_len)) { return ESP_ERR_NO_MEM; }
				}
				break;
			}
			case HTTP_EVENT_ON_FINISH: break; // ESP_LOGI(TagWebRequest, "HTTP_EVENT_ON_FINISH"); break;
			case HTTP_EVENT_DISCONNECTED:
			{
				// ESP_LOGI(TagWebRequest, "HTTP_EVENT_DISCONNECTED");
				int errtls = 0;
				esp_err_t erresp = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)ev->data, &errtls, nullptr);
				if(erresp != ESP_OK)
				{
					ESP_LOGI(TagWebRequest, "  error esp=%s (0x%x) tls=0x%x", esp_err_to_name(erresp), erresp, errtls);
				}
				break;
			}
			case HTTP_EVENT_REDIRECT:
				ESP_LOGI(TagWebRequest, "HTTP_EVENT_REDIRECT");
				esp_http_client_set_redirection(ev->client);
				break;
			default: ESP_LOGI(TagWebRequest, "HTTP EVENT UNKNOWN 0x%08x", ev->event_id); break;
		}
		return ESP_OK;
	}
	static void taskProc(void* arg)
	{
		if(WebRequestImpl* p = (WebRequestImpl*)arg) { p->runTask(); }
	}
	void runTask()
	{
		esp_err_t err = esp_http_client_perform(hHttpClient);
		if(err == ESP_OK)
		{
			response.status = esp_http_client_get_status_code(hHttpClient);
			ESP_LOGI(TagWebRequest, "esp_http_client_perform() succeeded, Status=%d bodysize=%u", response.status, (uint32_t)response.body.size());
		}
		else
		{
			ESP_LOGW(TagWebRequest, "esp_http_client_perform() failed: %s (0x%x)", esp_err_to_name(err), err);
		}
		Dispatcher::getMainDispatcher()->callAsync([this]()
		{
			if(hTask) { vTaskDelete(hTask); }
			hTask = NULL;
			if(hHttpClient) { esp_http_client_cleanup(hHttpClient); }
			hHttpClient = NULL;
			if(onFinish) { onFinish(response); }
			onFinish = nullptr;
			response.status = 0;
			response.body.clear();
		});
		vTaskSuspend(NULL);
	}
public:
	bool isRunning() const override
	{
		return hTask != NULL;
	}
	bool start(const char* url, std::function<void(const Response&)> onfinish) override
	{
		if(hTask || hHttpClient) { return false; }
		esp_http_client_config_t cfg{};
		cfg.url = url;
		cfg.event_handler = httpEventHandler;
		cfg.transport_type = HTTP_TRANSPORT_OVER_SSL;
		cfg.user_data = this;
		cfg.crt_bundle_attach = esp_crt_bundle_attach;
		hHttpClient = esp_http_client_init(&cfg);
		if(!hHttpClient)
		{
			ESP_LOGW(TagWebRequest, "esp_http_client_init() failed");
			return false;
		}
		response.status = 0;;
		response.body.clear();
		onFinish = onfinish;
		if(xTaskCreate(&taskProc, "httpclient", TaskStackDepth, this, TaskPriority, &hTask) != pdPASS)
		{
			ESP_LOGW(TagWebRequest, "xTaskCreate() failed");
			esp_http_client_cleanup(hHttpClient);
			hHttpClient = NULL;
			onFinish = nullptr;
			return false;
		}
		return true;
	}
	bool isAborted() const override
	{
		return aborted;
	}
	void abort() override
	{
		aborted = true;
		if(hHttpClient) { esp_http_client_cancel_request(hHttpClient); }
	}
};

xheap::uniquePtr<WebRequest> WebRequest::createInstance()
{
	return xheap::makeUnique<WebRequest, WebRequestImpl>();
}

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(CONFIG_IDF_TARGET)
