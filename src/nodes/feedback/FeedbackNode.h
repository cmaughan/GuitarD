#pragma once
#include "src/node/Node.h"
#include "circbuf.h"

class FeedbackNodeUi : public NodeUi {
public:
  FeedbackNodeUi(NodeShared* param) : NodeUi(param) {

  }

};

class FeedbackNode final : public Node {
  sample gain = 0.0;
  WDL_TypedCircBuf<sample> mPrevL;
  WDL_TypedCircBuf<sample> mPrevR;

public:
  FeedbackNode(const std::string pType) {
    shared.type = pType;
  }

  void setup(MessageBus::Bus* pBus, int pSamplerate = 48000, int pMaxBuffer = MAX_BUFFER, int pChannels = 2, int pInputs = 1, int pOutputs = 1) {
    Node::setup(pBus, pSamplerate, pMaxBuffer, pChannels, pInputs, pOutputs);
    addByPassParam();
    ParameterCoupling* p = new ParameterCoupling(
      "Gain", &gain, -40, -130.0, 40.0, 0.1
    );
    p->type = ParameterCoupling::Gain;
    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;

    shared.socketsIn[0]->mX += shared.width - 30;
    //shared.socketsIn[0]->mY = 50;
    shared.socketsOut[0]->mX -= shared.width - 30;
    //shared.socketsOut[0]->mY = 50;
    
  }

  void createBuffers() override {
    Node::createBuffers();
    mPrevL.SetSize(MIN_BLOCK_SIZE);
    mPrevR.SetSize(MIN_BLOCK_SIZE);
  }

  void deleteBuffers() override {
    Node::deleteBuffers();
    mPrevL.Reset();
    mPrevR.Reset();
  }


  void ProcessBlock(int nFrames) {
    if (byPass()) { return; }
    if (mIsProcessed == false) {
      gain = ParameterCoupling::dbToLinear(shared.parameters[1]->getValue());
      if (mPrevL.NbInBuf() > nFrames) {
        mPrevL.Get(mBuffersOut[0][0], nFrames);
        mPrevR.Get(mBuffersOut[0][1], nFrames);
        for (int i = 0; i < nFrames; i++) {
          mBuffersOut[0][0][i] *= gain;
          mBuffersOut[0][1][i] *= gain;
        }
        mIsProcessed = true;
      }
      else {
        outputSilence();
      }
    }

    if (inputsReady()) {
      sample** buffer = shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer;
      mPrevL.Add(buffer[0], nFrames);
      mPrevR.Add(buffer[1], nFrames);
    }
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    shared.graphics = pGrahics;
    mUi = new FeedbackNodeUi(&shared);
    mUi->setColor(Theme::Categories::TOOLS);
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};
