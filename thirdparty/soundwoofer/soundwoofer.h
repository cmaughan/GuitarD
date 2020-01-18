#pragma once

#ifndef SOUNDWOOFER_CUSTOM_JSON
#include "json.hpp"
#endif

#ifndef SOUNDWOOFER_CUSTOM_HTTP
#include "httplib.h"
#endif

#ifndef SOUNDWOOFER_CUSTOM_WAVE
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#endif

class SoundWoofer {
public:

  enum Status {
    SUCCESS = 0,
    ASYNC,
    SERVER_ERROR,
    PLUGIN_NAME_NOT_SET,
    WAV_ERROR,
    NOT_IMPLEMENTED,
    JSON_ENCODE_ERROR
  };

  /**
   * Soundwoofer structs
   */

  /**
   * Struct based on an Impulse Response from the soundwoofer API
   */
  struct SWImpulse {
    std::string id;
    int micX;
    int micY;
    int micZ;
    std::string micId;
    std::string name;
    std::string rig;
    std::string file;
    std::string description;
    int micPosition;
    std::string userName;
    std::string element;
    int channels = 0;
    int length = 0;
    float** samples = nullptr;
  };

  struct SWMic {
    std::string id;
    // TODO Mic object doesn't exist on the backend
  };

  struct SWRig {
    std::string id;
    std::string name;
  };

  /**
   * Preset Struct Based on the soundwoofer API
   */
  struct SWPreset {
    std::string name;
    std::string id;
    std::string plugin;
    std::string data;
  };

  typedef std::vector<SWImpulse> SWImpulses;
  typedef std::vector<SWRig> SWRigs;
  typedef std::vector<SWPreset> SWPresets;

  /**
   * Callback for async operations which provides a status code
   */
  typedef std::function<void(Status)> Callback;


private:
  // const std::string BACKEND_URL = "svenssj.tech";
  const std::string BACKEND_URL = "localhost";
  const int BACKEND_PORT = 5000;
  std::string mPluginName = ""; // Plugin name used to lable and filter presets by

  SWImpulses mIRlist;
  SWRigs mCabList;
  SWPresets mPresetList;

  /**
   * A function to generalize most tasks used in here
   * Will provide a status code
   */
  typedef std::function<Status()> Task;

  /**
   * Bundles together a task and a callback to call after a task has been finished
   */
  struct TaskBundle {
    Callback callback;
    Task task;
  };

  /**
   * Tasks will be queued up here and processed from back to front by mThread
   */
  std::vector<TaskBundle> mQueue;

  /**
   * This will mutex the mQueue
   */
  std::mutex mMutex;
  std::thread mThread;
  bool mThreadRunning = false;

  SoundWoofer() {
    int i = 0;
  }
public:

  void setPluginName(const std::string &name) {
    mPluginName = name;
  }

  /**
   * Gets a list of all the Impulse responses from the server
   * Will also call clearCachedIRs()
   */
  Status fetchIRs() {
    std::string data = httpGet("/Impulse");
    if (data.empty()) { return SERVER_ERROR; }
    mIRlist = parseIRs(data);
    data = httpGet("/Rig");
    if (data.empty()) { return SERVER_ERROR; }
    clearCachedIRs();
    mCabList = parseRigs(data);
    return SUCCESS;
    // TODO assign them to the cabs an mics
  }

  /**
   * Async version fetchIRs
   */
  Status fetchIRs(Callback callback) {
    startAsync([&]() {
      return this->fetchIRs();
    }, callback);
    return ASYNC;
  }

  /**
   * Fetches presets from the server
   */
  Status fetchPresets() {
    if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
    std::string data = httpGet("/Preset");
    if (data.empty()) { return SERVER_ERROR; }
    mPresetList = parsePresets(data);
    return SUCCESS;
  }

  /**
   * Async version of fetchpresets
   */
  Status fetchPresets(Callback callback) {
    startAsync([&]() {
      return this->fetchPresets();
    }, callback);
    return ASYNC;
  }

  /**
   * Assembles a SWPreset and sends it to the server
   * Doesn't update the cached presets
   */
  Status sendPreset(const std::string name, const char* data, const size_t length) {
    if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
    SWPreset preset = {
      name,
      generateUUID(),
      mPluginName
    };
    preset.data.append(data, length);
    return sendPreset(preset);
  }

  /**
   * Sends the preset to the server
   * Doesn't update the cached preset list
   */
  Status sendPreset(SWPreset &preset) {
    if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
    const std::string serialized = encodePreset(preset);
    if (serialized.empty()) { return JSON_ENCODE_ERROR; }
    return httpPost("/Preset", serialized.c_str(), serialized.size());
  }

  /**
   * Async version of sendPreset
   */
  Status sendPreset(SWPreset& preset, Callback callback) {
    startAsync([&]() {
      return sendPreset(preset);
    }, callback);
    return ASYNC;
  }

  /**
   * Download and decode a specific IR
   * The decoded IR will be in SWImpulse::samples
   */
  Status downloadIR(SWImpulse& ir) {
    if (ir.samples != nullptr) { return SUCCESS; }
    std::string result = httpGet("/File/Download/" + ir.file);
    if (result.empty()) { return SERVER_ERROR; }
    return loadWave(ir, result.c_str(), result.size());
  }

  /**
   * Async version of downloadIR
   */
  Status downloadIR(SWImpulse& ir, Callback callback) {
    startAsync([&]() {
      return downloadIR(ir);
    }, callback);
    return ASYNC;
  }

  SWImpulses& getIRs() {
    return mIRlist;
  }

  SWPresets& getPresets() {
    return mPresetList;
  }

  /**
   * Clears all the float buffers containing the IRs
   */
  void clearCachedIRs() {
    for (auto& i : mIRlist) {
      if (i.samples != nullptr) {
        for (int c = 0; c < i.channels; c++) {
          delete[] i.samples[c];
        }
        delete[] i.samples;
        i.samples = nullptr;
      }
    }
  }

  /**
   * Singleton stuff
   */
  SoundWoofer(const SoundWoofer&) {};
  SoundWoofer& operator = (const SoundWoofer&) {};

  static SoundWoofer& instance() {
    static SoundWoofer instance;
    return instance;
  }

  virtual ~SoundWoofer() {
    clearCachedIRs();
    if (mThread.joinable()) {
      mThread.join();
    }
  }
private:
  /**
   * This will add a TaskBundle to the queue and set off
   * the thread to work on the queue if it's not already running
   */
  void startAsync(Task task, Callback callback) {
    mMutex.lock();
    mQueue.push_back({
      callback, task
    });
    if (!mThreadRunning) {
      if (mThread.joinable()) {
        mThread.join();
      }
      this->mThreadRunning = true;
      mMutex.unlock();
      mThread = std::thread([&]() {
        while(mQueue.empty()) {
          mMutex.lock();
          TaskBundle t = *mQueue.begin();
          mQueue.erase(mQueue.begin());
          mMutex.unlock();
          t.callback(t.task());
        }
        mThreadRunning = false;
      });
    }
    else {
      mMutex.unlock();
    }
  }

  virtual SWImpulses parseIRs(std::string& data) {
    auto ret = SWImpulses();
    if (data.size() <= 2) { return ret; }
#ifndef SOUNDWOOFER_CUSTOM_JSON
    auto json = nlohmann::json::parse(data);
    for (auto i : json) {
      try {
        ret.push_back({
          i["id"],
          i["micX"], i["micY"], i["micZ"],
          i["micId"],
          i["name"],
          i["rig"],
          i["file"],
          i["description"],
          i["micPosition"],
          i["userName"],
          i["element"]
        });
      }
      catch (...) {
        // TODO some of these values are null which will fail
        int test = 0;
        // assert(false, "Error parsing IR");
      }
    }
#else
    assert(false); // You need to override this function if you want to use a different json parser!
#endif
    return ret;
  }

  virtual SWRigs parseRigs(std::string& data) {
    auto ret = SWRigs();
    if (data.size() <= 2) { return ret; }
#ifndef SOUNDWOOFER_CUSTOM_JSON
    auto json = nlohmann::json::parse(data);
    for (auto i : json) {
      try {
        ret.push_back({
          i["id"],
          i["name"]
        });
      }
      catch (...) {
        assert(false);
      }
    }
#else
    assert(false); // You need to override this function if you want to use a different json parser!
#endif
    return ret;
  }

  virtual SWPresets parsePresets(std::string& data) {
    auto ret = SWPresets();
    if (data.size() <= 2) { return ret; }
#ifndef SOUNDWOOFER_CUSTOM_JSON
    auto json = nlohmann::json::parse(data);
    for (auto i : json) {
      try {
        SWPreset preset = {
          i["name"],
          i["id"],
          i["plugin"],
          i["data"]
        };
        if (preset.plugin == mPluginName) {
          ret.push_back(preset);
        }
      }
      catch (...) {
        assert(false);
      }
    }
#else
    assert(false); // You need to override this function if you want to use a different json parser!
#endif
    return ret;
  }

  virtual std::string encodePreset(SWPreset &preset) {
    std::string ret;
#ifndef SOUNDWOOFER_CUSTOM_JSON
    nlohmann::json json;
    json["name"] = preset.name;
    json["data"] = preset.data;
    json["id"] = preset.id;
    json["plugin"] = preset.plugin;
    ret = json.dump();
#else
    assert(false); // You need to override this function if you want to use a different json parser!
#endif
    return ret;
  }

  /**
   * Will do a synchronous http request to the url + port + the endpoint parameter
   * Returns a string of the response body or an empty string for any error
   */
  virtual std::string httpGet(const std::string endpoint) {
#ifndef SOUNDWOOFER_CUSTOM_HTTP
    httplib::Client cli(BACKEND_URL, BACKEND_PORT);
    auto res = cli.Get(endpoint.c_str());
    if (res && res->status == 200) {
      return res->body;
    }
#else
    assert(false); // You need to override this function if you want to use a different http lib!
#endif
    return std::string();
  }

  virtual Status httpPost(const std::string endpoint, const char* data, const size_t length, const std::string mime = "application/json") {
#ifndef SOUNDWOOFER_CUSTOM_HTTP
    httplib::Client cli(BACKEND_URL, BACKEND_PORT);
    std::string body;
    body.append(data, length);
    // body += "\0"; // make sure it's null terminated
    auto res = cli.Post(endpoint.c_str(), body, mime.c_str());
    if (res && res->status == 200) {
      return SUCCESS;
    }
    return SERVER_ERROR;
#endif
    return NOT_IMPLEMENTED;
  }

  /**
   * Will load the wav file to a float buffer in the SWImpulse struct
   */
  virtual Status loadWave(SWImpulse &ir, const char* waveData, const size_t length) {
#ifndef SOUNDWOOFER_CUSTOM_WAVE
    drwav wav;
    if (!drwav_init_memory(&wav, waveData, length, nullptr)) {
      return WAV_ERROR;
    }

    // interleaved buffer
    float* pSampleData = static_cast<float*>(malloc(
      static_cast<size_t>(wav.totalPCMFrameCount)* wav.channels * sizeof(float)
    ));

    ir.length = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pSampleData);
    if (ir.length == 0) { return WAV_ERROR; }
    ir.channels = wav.channels;
    ir.samples = new float* [ir.channels];
    for (int c = 0; c < ir.channels; c++) {
      ir.samples[c] = new float[ir.length];
    }
    for (int s = 0; s < ir.length * ir.channels; s++) {
      // slow deinterleaving
      const int channel = s % ir.channels;
      const size_t sample = s / ir.channels;
      ir.samples[channel][sample] = pSampleData[s];
    }
    free(pSampleData);
    drwav_uninit(&wav);
    return SUCCESS;
#endif
    return NOT_IMPLEMENTED;
  }

  /**
   * Not really up to spec, but this should happen on the backend anyways
   */
  static std::string generateUUID() {
    srand(time(nullptr));
    const int charLength = 10 + 26;
    const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    const int UUIDLength = 36;
    char out[UUIDLength];
    for (int i = 0; i < UUIDLength; i++) {
      out[i] = chars[rand() % charLength];
    }
    out[9] = out[14] = out[19] = out[24] = '-';
    std::string ret;
    ret.append(out, UUIDLength);
    return ret;
  }
};