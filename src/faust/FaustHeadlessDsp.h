#ifndef FAUSTHEADLESSDSP
#define FAUSTHEADLESSDSP

#define FAUSTFLOAT iplug::sample

#include "IPlugConstants.h"
#include "src/misc/constants.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/node/Node.h"


struct Meta {
  virtual void declare(const char* key, const char* value) = 0;
};

/** 
 * This is a shim to collect pointers to all the properties/parameters from the faust DSP code
 */
struct UI {
  WDL_PtrList<ParameterCoupling> params;
  const char* name;

  UI() {
    name = DEFAULT_NODE_NAME;
  }

  void openVerticalBox(const char* key) {
    // NOTE This only is the name of the module if it has one box!
    if (name != DEFAULT_NODE_NAME) {
      WDBGMSG("openVerticalBox called multiple times. The node Type might be wrong!");
      assert(false);
    }
    name = key;
  };
  void openHorizontalBox(const char* key) {};
  void closeBox() {};
  void declare(FAUSTFLOAT*, const char*, const char*) {};

  void addHorizontalSlider(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT p_default, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize) {
    // For every new there should be a delete eh?
    // Well these will get cleaned up in the node (hopefully)
    params.Add(new ParameterCoupling(name, prop, p_default, min, max, stepSize));
  }

  void addVerticalSlider(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT p_default, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize) {
    params.Add(new ParameterCoupling(name, prop, p_default, min, max, stepSize));
  }

  void addCheckButton(const char* name, FAUSTFLOAT* prop) {
    params.Add(new ParameterCoupling(name, prop, 0, 0, 1, 1));
  }

  void addHorizontalBargraph(const char* name, FAUSTFLOAT* value, FAUSTFLOAT min, FAUSTFLOAT max) {};

};


/**
 * The faust DSP code will derive from this
 */
class FaustHeadlessDsp: public Node {
public:
  UI faustUi;

  // These three will be overridden by the generated faust code
  virtual void init(int samplingFreq) = 0;
  virtual void buildUserInterface(UI* ui_interface) = 0;
  virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;
  virtual int getNumInputs() = 0;
  virtual int getNumOutputs() = 0;
  virtual void instanceConstants(int samplingFreq) = 0;
  virtual void instanceClear() = 0;

  void setup(MessageBus::Bus* pBus, const int pSamplerate = 48000, const int pMaxBuffer = MAX_BUFFER, const int pChannels = 2, int pInputs = 1, int pOutputs = 1) {
    Node::setup(pBus, pSamplerate, pMaxBuffer, pChannels, getNumInputs() / pChannels, getNumOutputs() / pChannels);
    
    /**
     * This will use the UI shim to create ParameterCouplings between the faust dsp and iplug iControls
     * However they will not be registered to the daw yet, since loading a preset will need them to claim
     * the right ones so the automation will affect the correct parameters
     */
    buildUserInterface(&faustUi);
    init(pSamplerate);
    if (mType == DEFAULT_NODE_NAME) {
      mType = faustUi.name;
    }

    addByPassParam();

    for (int i = 0, pos = 0; i < faustUi.params.GetSize(); i++) {
      ParameterCoupling* p = faustUi.params.Get(i);
      if (p->name == "Stereo") {
        addStereoParam(p);
        continue;
      }
      p->y = p->h * pos - 80;
      mParameters.Add(p);
      pos++;
    }
  }

  void OnChannelsChanged(int p_channels) override {
    if (mBuffersOut != nullptr) {
      WDBGMSG("Warning trying to change the channelcount on a faust node!\n");
    }
    else {
      Node::OnChannelsChanged(p_channels);
    }
  }

  void OnSamplerateChanged(int p_samplerate) override {
    instanceConstants(p_samplerate);
  }

  void OnTransport() override {
    instanceClear();
  }

  /**
   * The faust uses a fairly similar way of processing blocks, however
   * A node might have multiple inputs so the right ones have to be forwarded
   */
  virtual void ProcessBlock(int nFrames) {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    for (int i = 1; i < mParameters.GetSize(); i++) {
      mParameters.Get(i)->update();
    }
    compute(nFrames, mSocketsIn.Get(0)->mConnectedTo->mParentBuffer, mBuffersOut[0]);
    mIsProcessed = true;
  }
};

#endif 
