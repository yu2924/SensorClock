//
//  AlarmSounder.cpp
//  SensorClock
//
//  created by yu2924 on 2026-04-08
//

#include "AlarmSounder.h"

#if defined(_WINDOWS)

// ================================================================================
// Prototype code for Windows

#include <mfapi.h>
#include <mfmediaengine.h>
#include <mmdeviceapi.h>
#include <AudioSessionTypes.h>
#include <comdef.h>
#include "utilities/listenerlist.h"
#include "utilities/platform.h"
#include "modules/Dispatcher.h"

_COM_SMARTPTR_TYPEDEF(IMFMediaEngineClassFactory, __uuidof(IMFMediaEngineClassFactory));
_COM_SMARTPTR_TYPEDEF(IMFAttributes, __uuidof(IMFAttributes));
_COM_SMARTPTR_TYPEDEF(IMFMediaEngine, __uuidof(IMFMediaEngine));
_COM_SMARTPTR_TYPEDEF(IMFMediaEngineEx, __uuidof(IMFMediaEngineEx));
_COM_SMARTPTR_TYPEDEF(IMFMediaError, __uuidof(IMFMediaError));

static const char* MediaEngineEventCodeString(DWORD evcode)
{
	static const struct { DWORD code; const char* text; } Table[] =
	{
		{ MF_MEDIA_ENGINE_EVENT_LOADSTART, "LOADSTART" },
		{ MF_MEDIA_ENGINE_EVENT_PROGRESS, "PROGRESS" },
		{ MF_MEDIA_ENGINE_EVENT_SUSPEND, "SUSPEND" },
		{ MF_MEDIA_ENGINE_EVENT_ABORT, "ABORT" },
		{ MF_MEDIA_ENGINE_EVENT_ERROR, "ERROR" },
		{ MF_MEDIA_ENGINE_EVENT_EMPTIED, "EMPTIED" },
		{ MF_MEDIA_ENGINE_EVENT_STALLED, "STALLED" },
		{ MF_MEDIA_ENGINE_EVENT_PLAY, "PLAY" },
		{ MF_MEDIA_ENGINE_EVENT_PAUSE, "PAUSE" },
		{ MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA, "LOADEDMETADATA" },
		{ MF_MEDIA_ENGINE_EVENT_LOADEDDATA, "LOADEDDATA" },
		{ MF_MEDIA_ENGINE_EVENT_WAITING, "WAITING" },
		{ MF_MEDIA_ENGINE_EVENT_PLAYING, "PLAYING" },
		{ MF_MEDIA_ENGINE_EVENT_CANPLAY, "CANPLAY" },
		{ MF_MEDIA_ENGINE_EVENT_CANPLAYTHROUGH, "CANPLAYTHROUGH" },
		{ MF_MEDIA_ENGINE_EVENT_SEEKING, "SEEKING" },
		{ MF_MEDIA_ENGINE_EVENT_SEEKED, "SEEKED" },
		{ MF_MEDIA_ENGINE_EVENT_TIMEUPDATE, "TIMEUPDATE" },
		{ MF_MEDIA_ENGINE_EVENT_ENDED, "ENDED" },
		{ MF_MEDIA_ENGINE_EVENT_RATECHANGE, "RATECHANGE" },
		{ MF_MEDIA_ENGINE_EVENT_DURATIONCHANGE, "DURATIONCHANGE" },
		{ MF_MEDIA_ENGINE_EVENT_VOLUMECHANGE, "VOLUMECHANGE" },
		{ MF_MEDIA_ENGINE_EVENT_FORMATCHANGE, "FORMATCHANGE" },
		{ MF_MEDIA_ENGINE_EVENT_PURGEQUEUEDEVENTS, "PURGEQUEUEDEVENTS" },
		{ MF_MEDIA_ENGINE_EVENT_TIMELINE_MARKER, "TIMELINE_MARKER" },
		{ MF_MEDIA_ENGINE_EVENT_BALANCECHANGE, "BALANCECHANGE" },
		{ MF_MEDIA_ENGINE_EVENT_DOWNLOADCOMPLETE, "DOWNLOADCOMPLETE" },
		{ MF_MEDIA_ENGINE_EVENT_BUFFERINGSTARTED, "BUFFERINGSTARTED" },
		{ MF_MEDIA_ENGINE_EVENT_BUFFERINGENDED, "BUFFERINGENDED" },
		{ MF_MEDIA_ENGINE_EVENT_FRAMESTEPCOMPLETED, "FRAMESTEPCOMPLETED" },
		{ MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE, "NOTIFYSTABLESTATE" },
		{ MF_MEDIA_ENGINE_EVENT_FIRSTFRAMEREADY, "FIRSTFRAMEREADY" },
		{ MF_MEDIA_ENGINE_EVENT_TRACKSCHANGE, "TRACKSCHANGE" },
		{ MF_MEDIA_ENGINE_EVENT_OPMINFO, "OPMINFO" },
		{ MF_MEDIA_ENGINE_EVENT_RESOURCELOST, "RESOURCELOST" },
		{ MF_MEDIA_ENGINE_EVENT_DELAYLOADEVENT_CHANGED, "DELAYLOADEVENT_CHANGED" },
		{ MF_MEDIA_ENGINE_EVENT_STREAMRENDERINGERROR, "STREAMRENDERINGERROR" },
		{ MF_MEDIA_ENGINE_EVENT_SUPPORTEDRATES_CHANGED, "SUPPORTEDRATES_CHANGED" },
		{ MF_MEDIA_ENGINE_EVENT_AUDIOENDPOINTCHANGE, "AUDIOENDPOINTCHANGE" },
	};
	for(const auto& ent : Table)
	{
		if(ent.code == evcode) { return ent.text; }
	}
	return "unknown";
}

struct ScopedMfInit
{
	ScopedMfInit() { HRESULT r = MFStartup(MF_VERSION); }
	~ScopedMfInit() { MFShutdown(); }
};

struct MfPlayer : IMFMediaEngineNotify
{
	IMFMediaEnginePtr mediaEngine;
	std::function<void(MF_MEDIA_ENGINE_EVENT)> onEvent;
	MfPlayer()
	{
		DEBUGPRINTI("MfPlayer: construct");
		IMFMediaEngineClassFactoryPtr factory;
		HRESULT r = factory.CreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER);
		if(FAILED(r)) { DEBUGPRINTW("MfPlayer: CreateInstance(MFMediaEngineClassFactory) failed"); return; }
		IMFAttributesPtr attr;
		r = MFCreateAttributes(&attr, 0);
		if(FAILED(r)) { DEBUGPRINTW("MfPlayer: MFCreateAttributes() failed"); return; }
		attr->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, this);
		attr->SetUINT32(MF_MEDIA_ENGINE_AUDIO_CATEGORY, AudioCategory_Media);
		r = factory->CreateInstance(MF_MEDIA_ENGINE_AUDIOONLY | MF_MEDIA_ENGINE_REAL_TIME_MODE, attr, &mediaEngine);
		if(FAILED(r)) { DEBUGPRINTW("MfPlayer: MFMediaEngineClassFactory::CreateInstance() failed"); return; }
		mediaEngine->SetPreload(MF_MEDIA_ENGINE_PRELOAD_AUTOMATIC);
		// DEBUGPRINTI("[MfPlayer] MFMediaEngineClassFactory::CreateInstance() succeeded");
	}
	~MfPlayer()
	{
		if(mediaEngine) mediaEngine->Shutdown();
		DEBUGPRINTI("MfPlayer: destruct");
	}
	bool setMediaUrl(BSTR url)
	{
		if(!mediaEngine) return false;
		if(url)
		{
			HRESULT r = mediaEngine->SetSource(url);
			if(FAILED(r)) { DEBUGPRINTW("MfPlayer: SetSource() failed, path=%s", url); return false; }
			// DEBUGPRINTI("[MfPlayer] SetSource() succeeded url=\"%s\"", (const char*)_bstr_t((const wchar_t*)url));
			r = mediaEngine->Load();
			if(FAILED(r)) { DEBUGPRINTW("MfPlayer: Load() failed, hr=0x%08X", r); return false; }
			// DEBUGPRINTI("[MfPlayer] Load() succeeded");
		}
		else
		{
			mediaEngine->SetSource(nullptr);
		}
		return true;
	}
	bool hasAudio() const { return mediaEngine ? mediaEngine->HasAudio() : false; }
	double getDuration() const { return mediaEngine ? mediaEngine->GetDuration() : 0; }
	double getCurrentPosition() const { return mediaEngine ? mediaEngine->GetCurrentTime() : 0; }
	void setCurrentPosition(double v) { if(mediaEngine) mediaEngine->SetCurrentTime(v); };
	double getVolume() const { return mediaEngine ? mediaEngine->GetVolume() : 0; }
	void setVolume(double v) { if(mediaEngine) mediaEngine->SetVolume(v); }
	bool getLoop() const { return mediaEngine ? mediaEngine->GetLoop() : false; }
	void setLoop(bool v) { if(mediaEngine) mediaEngine->SetLoop(v); }
	void play() { if(mediaEngine) mediaEngine->Play(); }
	void pause() { if(mediaEngine) mediaEngine->Pause(); }
	bool isPaused() const { return mediaEngine ? mediaEngine->IsPaused() : false; }
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
	{
		if(iid == __uuidof(IUnknown)) { *ppv = (IUnknown*)this; return S_OK; }
		if(iid == __uuidof(IMFMediaEngineNotify)) { *ppv = (IMFMediaEngineNotify*)this; return S_OK; }
		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG) AddRef() { return 1; }
	STDMETHODIMP_(ULONG) Release() { return 1; }
	// IMFMediaEngineNotify::EventNotify
	HRESULT EventNotify(DWORD ev, DWORD_PTR param1, DWORD param2)
	{
		// DEBUGPRINTI("[MfPlayer] EventNotify %d %s", ev, MediaEngineEventCodeString(ev));
		if(ev == MF_MEDIA_ENGINE_EVENT_ERROR)
		{
			IMFMediaErrorPtr error; mediaEngine->GetError(&error);
			if(error)
			{
				USHORT errorCode = error->GetErrorCode();
				HRESULT extCode = error->GetExtendedErrorCode();
				DEBUGPRINTW("MfPlayer: ERROR: code=%u, extended=0x%08X", errorCode, extCode);
			}
		}
		if(onEvent) { Dispatcher::getMainDispatcher()->callAsync([this, ev]() { onEvent((MF_MEDIA_ENGINE_EVENT)ev); }); }
		return S_OK;
	}
};

class AlarmSounderImpl : public AlarmSounder
{
public:
	ListenerListT<Listener> listenerList;
	ScopedMfInit mfinit;
	MfPlayer mfPlayer;
	_bstr_t mediaFile;
	int volume = 100;
	bool looped = true;
	bool enabled = true;
	bool playOn = false;
	bool auditionOn = false;
	AlarmSounderImpl()
	{
		mfPlayer.setLoop(looped);
		mfPlayer.onEvent = [this](MF_MEDIA_ENGINE_EVENT ev) { onEvent(ev); };
	}
	~AlarmSounderImpl() override
	{
	}
	void onEvent(MF_MEDIA_ENGINE_EVENT ev)
	{
		switch(ev)
		{
			case MF_MEDIA_ENGINE_EVENT_ENDED:
				if(!mfPlayer.getLoop())
				{
					Dispatcher::getMainDispatcher()->callAsync([this]()
					{
						setPlayOn(false);
						listenerList.call(&Listener::alertSounderReachedToEnd, this);
					});
				}
				break;
		}
	}
	void internalSetPlayingState()
	{
		bool shouldplay = mfPlayer.hasAudio() && enabled && (playOn || auditionOn);
		if     ( shouldplay &&  mfPlayer.isPaused()) { mfPlayer.setCurrentPosition(0); mfPlayer.play(); }
		else if(!shouldplay && !mfPlayer.isPaused()) { mfPlayer.pause(); mfPlayer.setCurrentPosition(0); }
	}
	void addListener(Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(Listener* p) override
	{
		listenerList.remove(p);
	}
	int getVolume() const override
	{
		return volume;
	}
	void setVolume(int v) override
	{
		if(volume == v) { return; }
		volume = std::max(0, std::min(100, v));
		mfPlayer.setVolume((double)volume * 0.01);
		listenerList.call(&Listener::alertSounderSettingsDidChange, this, volume, looped, enabled);
	}
	bool isLooped() const override
	{
		return looped;
	}
	void setLooped(bool v) override
	{
		if(looped == v) { return; }
		looped = v;
		mfPlayer.setLoop(looped);
		listenerList.call(&Listener::alertSounderSettingsDidChange, this, volume, looped, enabled);
	}
	bool isEnabled() const override
	{
		return enabled;
	}
	void setEnabled(bool v) override
	{
		if(enabled == v) { return; }
		enabled = v;
		internalSetPlayingState();
		listenerList.call(&Listener::alertSounderSettingsDidChange, this, volume, looped, enabled);
	}
	const char* getMediaFile() const override
	{
		return mediaFile.length() ? (const char*)mediaFile : nullptr;
	}
	void setMediaFile(const char* v) override
	{
		mediaFile = v;
		mfPlayer.setMediaUrl(mediaFile.length() ? (BSTR)mediaFile : nullptr);
	}
	bool isPlayOn() const override
	{
		return playOn;
	}
	void setPlayOn(bool v) override
	{
		playOn = v;
		internalSetPlayingState();
	}
	bool isAuditionOn() const override
	{
		return auditionOn;
	}
	void setAuditionOn(bool v) override
	{
		auditionOn = v;
		internalSetPlayingState();
	}
};

void AlarmSounder::platformInit()
{
}

xheap::uniquePtr<AlarmSounder> AlarmSounder::createInstance()
{
	return xheap::makeUnique<AlarmSounder, AlarmSounderImpl>();
}

#elif defined(ESP_PLATFORM)

// ================================================================================
// Production code for ESP32

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_task.h>
#include <esp_codec_dev.h>
#include <esp_audio_dec.h>
#include <esp_audio_simple_dec.h>
#include <esp_audio_simple_dec_default.h>
#include <bsp/esp-bsp.h>
#include "utilities/listenerlist.h"
#include "utilities/strutil.h"
#include "utilities/xheap.h"
#include "modules/Dispatcher.h"

static const char* Tag = "AlarmSounder";

namespace alarmsndimpl
{
	class AudioOutDevice final
	{
	private:
		struct Singleton
		{
			static constexpr uint32_t DefaultSampleRate = 44100;
			static constexpr uint8_t DefaultBitDepth = 16;
			static constexpr uint8_t DefaultNumChannels = 2;
			esp_codec_dev_handle_t play_dev_handle = NULL;
			Singleton()
			{
				// cf. bsp_extra_codec_init()
				play_dev_handle = bsp_audio_codec_speaker_init();
				if(!play_dev_handle) { ESP_LOGW(Tag, "bsp_audio_codec_speaker_init() failed, null"); }
				else { ESP_LOGI(Tag, "bsp_audio_codec_speaker_init() succeeded"); }
				setPcmFormatToDefault();
			}
			~Singleton()
			{
				if(play_dev_handle) { esp_codec_dev_delete(play_dev_handle); }
			}
			bool isReady() const
			{
				return play_dev_handle != NULL;
			}
			esp_err_t setPcmFormatToDefault()
			{
				return setPcmFormat(DefaultSampleRate, DefaultBitDepth, DefaultNumChannels);
			}
			esp_err_t setPcmFormat(uint32_t rate, uint8_t bits_cfg, uint8_t ch)
			{
				// cf. bsp_extra_codec_set_fs()
				if(!play_dev_handle) { return ESP_FAIL; }
				esp_codec_dev_sample_info_t inf{};
				inf.bits_per_sample = bits_cfg;
				inf.channel = ch;
				inf.sample_rate = rate;
				esp_codec_dev_close(play_dev_handle);
				return esp_codec_dev_open(play_dev_handle, &inf);
			}
			esp_err_t setVolume(int v)
			{
				// cf. bsp_extra_codec_volume_set()
				if(!play_dev_handle) { return ESP_FAIL; }
				return esp_codec_dev_set_out_vol(play_dev_handle, v);
			}
			esp_err_t setMute(bool m)
			{
				// cf. bsp_extra_codec_mute_set()
				if(!play_dev_handle) { return ESP_FAIL; }
				return esp_codec_dev_set_out_mute(play_dev_handle, m);
			}
			esp_err_t writeSound(void* audio_buffer, size_t len)
			{
				// cf. bsp_extra_i2s_write()
				if(!play_dev_handle) { return ESP_FAIL; }
				return esp_codec_dev_write(play_dev_handle, audio_buffer, len);
			}
			static Singleton& getInstance()
			{
				static xheap::uniquePtr<Singleton> gInst;
				if(!gInst) { gInst = xheap::makeUnique<Singleton>(); }
				return *gInst;
			}
		};
		AudioOutDevice() = delete;
	public:
		static constexpr int MaxVolume = 100;
		static constexpr int MinVolume = 0;
		static constexpr int DefVolume = 60;
		static void initExplicitly() { Singleton::getInstance(); }
		static bool isReady() { return Singleton::getInstance().isReady();}
		static esp_err_t setPcmFormat(uint32_t fs, uint8_t bps, uint8_t ch) { return Singleton::getInstance().setPcmFormat(fs, bps, ch); }
		static esp_err_t setVolume(int v) {return Singleton::getInstance().setVolume(v);}
		static esp_err_t setMute(bool m) {return Singleton::getInstance().setMute(m);}
		static esp_err_t writeSound(void* pb, size_t cb) {return Singleton::getInstance().writeSound(pb, cb);}
	};
	class AudioFileReader
	{
	private:
		static void oneTimeInit()
		{
			static bool init = false;
			if(init) { return; }
			esp_audio_dec_register_default();
			esp_audio_simple_dec_register_default();
			init = true;
		}
		union simp_dec_all_t
		{
			esp_m4a_dec_cfg_t m4a_cfg;
			esp_aac_dec_cfg_t aac_cfg;
		};
		// cf. simple_decder_test.c: get_simple_decoder_type(), get_simple_decoder_config()
		static bool createDecoderConfig(const char* file, esp_audio_simple_dec_cfg_t* cfg)
		{
			const char* ext = strrchr(file, '.');
			if(!ext) { return false; }
			simp_dec_all_t* all_cfg = (simp_dec_all_t*)cfg->dec_cfg;
			if(strcasecmp(ext, ".aac") == 0)
			{
				cfg->dec_type = ESP_AUDIO_SIMPLE_DEC_TYPE_AAC;
				all_cfg->aac_cfg.aac_plus_enable = true;
				cfg->cfg_size = sizeof(all_cfg->aac_cfg);
			}
			else if(strcasecmp(ext, ".mp3") == 0)
			{
				cfg->dec_type = ESP_AUDIO_SIMPLE_DEC_TYPE_MP3;
			}
			else if(strcasecmp(ext, ".flac") == 0)
			{
				cfg->dec_type = ESP_AUDIO_SIMPLE_DEC_TYPE_FLAC;
			}
			else if(strcasecmp(ext, ".wav") == 0)
			{
				cfg->dec_type = ESP_AUDIO_SIMPLE_DEC_TYPE_WAV;
			}
			else if((strcasecmp(ext, ".mp4") == 0) || (strcasecmp(ext, ".m4a") == 0))
			{
				cfg->dec_type = ESP_AUDIO_SIMPLE_DEC_TYPE_M4A;
				all_cfg->m4a_cfg.aac_plus_enable = true;
				cfg->cfg_size = sizeof(all_cfg->m4a_cfg);
			}
			else if(strcasecmp(ext, ".ogg") == 0)
			{
				cfg->dec_type = ESP_AUDIO_SIMPLE_DEC_TYPE_OGG;
			}
			else { return false; }
			return true;
		}
		static bool isValidPcmFormat(const esp_audio_simple_dec_info_t& inf)
		{
			if(!inf.sample_rate) { ESP_LOGW(Tag, "invalid pcm format: sample_rate=%u", inf.sample_rate); return false; }
			if(!inf.bits_per_sample) { ESP_LOGW(Tag, "invalid pcm format: bits_per_sample=%u", inf.bits_per_sample); return false; }
			if(!inf.channel) { ESP_LOGW(Tag, "invalid pcm format: channel=%u", inf.channel); return false; }
			return true;
		}
		static constexpr uint32_t read_size = 512;
		static constexpr uint32_t max_out_size_default = 4096;
		uint32_t max_out_size = max_out_size_default;
		uint8_t* in_buf = nullptr;
		uint8_t* out_buf = nullptr;
		esp_audio_simple_dec_handle_t decoder = NULL;
		esp_audio_simple_dec_raw_t in_raw{};
		esp_audio_simple_dec_out_t out_frame{};
		esp_audio_simple_dec_info_t dec_info = {};
		FILE* pfile = nullptr;
	public:
		AudioFileReader()
		{
			oneTimeInit();
			in_buf = (uint8_t*)xheap::memAlloc(read_size);
			out_buf = (uint8_t*)xheap::memAlloc(max_out_size);
		}
		~AudioFileReader()
		{
			close();
			xheap::memFree(out_buf);
			xheap::memFree(in_buf);
		}
	private:
		// cf. simple_decder_test.c: audio_simple_decoder_test()
		bool pullSource()
		{
			while(!out_frame.decoded_size)
			{
				if(!in_raw.len)
				{
					uint32_t cbr = (uint32_t)fread(in_buf, 1, read_size, pfile);
					if(!cbr) { return false; }
					in_raw.buffer = in_buf;
					in_raw.len = cbr;
					in_raw.eos = cbr < read_size;
				}
				out_frame = {};
				out_frame.buffer = out_buf;
				out_frame.len = max_out_size;
				esp_audio_err_t r = esp_audio_simple_dec_process(decoder, &in_raw, &out_frame);
                if(r == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH)
				{
                    uint8_t* p = (uint8_t*)xheap::memRealloc(out_buf, out_frame.needed_size);
                    if(!p) { return false; }
                    out_buf = p;
                    max_out_size = out_frame.needed_size;
                    out_frame.buffer = out_buf;
                    out_frame.len = max_out_size;
					r = esp_audio_simple_dec_process(decoder, &in_raw, &out_frame);
                }
				if(r != ESP_AUDIO_ERR_OK) { ESP_LOGE(Tag, "esp_audio_simple_dec_process() failed, %d", r); return false; }
                in_raw.buffer += in_raw.consumed;
				in_raw.len -= in_raw.consumed;
			}
			return true;
		}
	public:
		// cf. simple_decder_test.c: audio_simple_decoder_test()
		bool open(const char* file)
		{
			close();
			pfile = fopen(file, "rb");
			if(!pfile) { ESP_LOGI(Tag, "fopen(%s) failed", file); close(); return false; }
			esp_audio_simple_dec_cfg_t dec_cfg{};
			simp_dec_all_t all_cfg = {};
			dec_cfg.dec_cfg = &all_cfg;
			if(!createDecoderConfig(file, &dec_cfg)) { close(); return false; }
			esp_audio_err_t r = esp_audio_simple_dec_open(&dec_cfg, &decoder);
			if(r != ESP_AUDIO_ERR_OK) { ESP_LOGW(Tag, "esp_audio_simple_dec_open() failedd, %d", r); close(); return false; }
			if(!pullSource()) { close(); return false; }
			r = esp_audio_simple_dec_get_info(decoder, &dec_info);
			if(r != ESP_AUDIO_ERR_OK) { ESP_LOGW(Tag, "esp_audio_simple_dec_get_info() failedd, %d", r); close(); return false; }
			if(!isValidPcmFormat(dec_info)) { ESP_LOGW(Tag, "invalid pcm format"); close(); return false; }
			ESP_LOGI(Tag, "AudioFileReader::open(%s) succeeded, fs=%u bps=%u ch=%u", file, dec_info.sample_rate, dec_info.bits_per_sample, dec_info.channel);
			return true;
		}
		void close()
		{
			if(decoder) { esp_audio_simple_dec_close(decoder); decoder = NULL; }
			if(pfile) { fclose(pfile); pfile = nullptr; }
			in_raw = {};
			out_frame = {};
			dec_info = {};
		}
		bool isOpen() const
		{
			return decoder != NULL;
		}
		const esp_audio_simple_dec_info_t& getPcmFormat() const
		{
			return dec_info;
		}
		void seekToStart()
		{
			esp_audio_simple_dec_reset(decoder);
			fseek(pfile, 0, SEEK_SET);
			in_raw = {};
			out_frame = {};
			pullSource();
		}
		uint32_t read(uint8_t* pbdst, uint32_t cbdst)
		{
			uint32_t cbpos = 0; while(0 < cbdst)
			{
				if(!out_frame.decoded_size) { if(!pullSource()) { break; } }
				uint32_t cbseg = std::min(out_frame.decoded_size, cbdst);
				memcpy(pbdst, out_frame.buffer, cbseg);
				cbpos += cbseg;
				pbdst += cbseg;
				cbdst -= cbseg;
				out_frame.buffer += cbseg;
				out_frame.decoded_size -= cbseg;
			}
			return cbpos;
		}
	};
} // namespace alarmsndimpl

using namespace alarmsndimpl;

class AlarmSounderImpl : public AlarmSounder
{
private:
	// It should be set to a higher value than the LVGL task priority.
	static constexpr UBaseType_t TaskPriority = 5;
	static constexpr uint32_t TaskStackDepth = 4096;
	TaskHandle_t hTask = NULL;
	static constexpr uint32_t BitReqQuit = 0x01;
	static constexpr uint32_t BitDidQuit = 0x02;
	EventGroupHandle_t hEventGroup = NULL;
	ListenerListT<Listener> listenerList;
	strutil::Str<256> mediaFile;
	AudioFileReader reader;
	int volume = AudioOutDevice::DefVolume;
	bool looped = true;
	bool enabled = true;
	bool playOn = false;
	bool auditionOn = false;
public:
	AlarmSounderImpl()
	{
		hEventGroup = xEventGroupCreate();
	}
	~AlarmSounderImpl() override
	{
		enabled = false;
		internalUpdateRunningState();
		if(hEventGroup) { vEventGroupDelete(hEventGroup); }
	}
private:
	static void taskProc(void* arg)
	{
		if(AlarmSounderImpl* p = (AlarmSounderImpl*)arg) { p->runTask(); }
	}
	void runTask()
	{
		// ESP_LOGI(Tag, "audio thread begin");
		AudioOutDevice::setMute(false);
		AudioOutDevice::setVolume(volume);
		const auto& fmt = reader.getPcmFormat();
		const uint32_t cbblock = (uint32_t)fmt.channel * (uint32_t)fmt.bits_per_sample / 8;
		const uint32_t cbbuf = cbblock * 256;
		uint8_t* pbuf = (uint8_t*)xheap::memAlloc(cbbuf);
		while(!(xEventGroupGetBits(hEventGroup) & BitReqQuit))
		{
			uint32_t cbr = reader.read(pbuf, cbbuf);
			if(cbr) { AudioOutDevice::writeSound(pbuf, cbr); }
			if(cbr < cbbuf)
			{
				if(looped) { reader.seekToStart(); }
				else
				{
					Dispatcher::getMainDispatcher()->callAsync([this]()
					{
						setPlayOn(false);
						listenerList.call(&Listener::alertSounderReachedToEnd, this);
					});
					break;
				}
			}
		}
		reader.seekToStart();
		xheap::memFree(pbuf);
		AudioOutDevice::setMute(true);
		// ESP_LOGI(Tag, "audio thread end");
		xEventGroupSetBits(hEventGroup, BitDidQuit);
		vTaskSuspend(NULL);
	}
	void internalUpdateRunningState()
	{
		bool shouldrun = reader.isOpen() && enabled && (playOn || auditionOn) && AudioOutDevice::isReady();
		if(shouldrun && !hTask)
		{
			xEventGroupClearBits(hEventGroup, BitReqQuit | BitDidQuit);
			if(xTaskCreate(&taskProc, "audio", TaskStackDepth, this, TaskPriority, &hTask) != pdPASS)
			{
				ESP_LOGW(Tag, "xTaskCreate() failed");
			}
		}
		else if(!shouldrun && hTask)
		{
			xEventGroupSetBits(hEventGroup, BitReqQuit);
			xTaskAbortDelay(hTask);
			xEventGroupWaitBits(hEventGroup, BitDidQuit, pdFALSE, pdFALSE, pdMS_TO_TICKS(100));
			vTaskDelete(hTask);
			hTask = NULL;
		}
	}
public:
	void addListener(Listener* p) override
	{
		listenerList.add(p);
	}
	void removeListener(Listener* p) override
	{
		listenerList.remove(p);
	}
	// settings
	int getVolume() const override
	{
		return volume;
	}
	void setVolume(int v) override
	{
		if(volume == v) { return; }
		volume = std::max(AudioOutDevice::MinVolume, std::min(AudioOutDevice::MaxVolume, v));
		AudioOutDevice::setVolume(volume);
		listenerList.call(&Listener::alertSounderSettingsDidChange, this, volume, looped, enabled);
	}
	bool isLooped() const override
	{
		return looped;
	}
	void setLooped(bool v) override
	{
		if(looped == v) { return; }
		looped = v;
		listenerList.call(&Listener::alertSounderSettingsDidChange, this, volume, looped, enabled);
	}
	bool isEnabled() const override
	{
		return enabled;
	}
	void setEnabled(bool v) override
	{
		if(enabled == v) { return; }
		enabled = v;
		internalUpdateRunningState();
		listenerList.call(&Listener::alertSounderSettingsDidChange, this, volume, looped, enabled);
	}
	// transport
	const char* getMediaFile() const override
	{
		return mediaFile;
	}
	void setMediaFile(const char* v) override
	{
		if(mediaFile == v) { return; }
		reader.close();
		internalUpdateRunningState();
		mediaFile = v;
		if(!mediaFile.isEmpty() && reader.open(mediaFile))
		{
			const auto& fmt = reader.getPcmFormat();
			if(AudioOutDevice::setPcmFormat(fmt.sample_rate, fmt.bits_per_sample, fmt.channel) != ESP_OK) { reader.close(); }
			internalUpdateRunningState();
		}
	}
	bool isPlayOn() const override
	{
		return playOn;
	}
	void setPlayOn(bool v) override
	{
		playOn = v;
		internalUpdateRunningState();
	}
	bool isAuditionOn() const override
	{
		return auditionOn;
	}
	void setAuditionOn(bool v) override
	{
		auditionOn = v;
		internalUpdateRunningState();
	}
};

void AlarmSounder::platformInit()
{
	alarmsndimpl::AudioOutDevice::initExplicitly();
}

xheap::uniquePtr<AlarmSounder> AlarmSounder::createInstance()
{
	return xheap::makeUnique<AlarmSounder, AlarmSounderImpl>();
}

#else
#error unknown platform
#endif // defined(_WINDOWS) / defined(ESP_PLATFORM)
