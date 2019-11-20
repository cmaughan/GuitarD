#pragma once

#include "src/node/Node.h"
#include "src/node/NodeUi.h"

#define TWO_YROOT_TWELVE 1.05946309435929526456

class InputNodeUi : public NodeUi {
  map<float, string> equalPitches;
  const char* mNotes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
  bool mEnableTuner;
  const char* mCurrentQuantizedPitch;
  int mTunerCentsOff;
  float mCurrentFrequency;
  void update() { }
  iplug::igraphics::IText mBlocksizeText;
  string mInfo;
public:
  InputNodeUi(NodeUiParam param) : NodeUi(param) {
    mInfo = "";
    mBlocksizeText = DEBUGFONT;
    mCurrentFrequency = 440.f;
    mCurrentQuantizedPitch = mNotes[0];
    mTunerCentsOff = 0;
    mEnableTuner = true;
    float freq = 16.35159783f;
    for (int i = 0; i < 108; i++) {
      equalPitches.insert(pair<float, const char*>(freq, mNotes[i % 12]));
      freq *= TWO_YROOT_TWELVE;
    }
  }

  void Draw(IGraphics& g) override {
    NodeUi::Draw(g);
    mInfo = "Blocksize: " + to_string(mParentNode->mLastBlockSize) + " Sample-Rate: " + to_string(mParentNode->mSampleRate);
    g.DrawText(mBlocksizeText, mInfo.c_str(), mRECT);
    if (mEnableTuner) {
      // g.DrawRect(IColor(255, 0, 255, 0), mDisconnectAllButton);
      // mDirty = true;
    }
  }
};

class InputNode : public Node {
public:
  InputNode(MessageBus::Bus* pBus) : Node() {
    mLastBlockSize = -1;
    setup(pBus, 48000, MAX_BUFFER, 2, 0, 1);
  }

  void ProcessBlock(int) {}

  void CopyIn(iplug::sample** in, int nFrames) {
    mLastBlockSize = nFrames;
    for (int c = 0; c < mChannelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        mBuffersOut[0][c][i] = in[c][i];
      }
    }
    mIsProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    if (mX == mY && mX == 0) {
      // Place it at the screen edge if no position is set
      mY = pGrahics->Height() / 2;
    }
    mUi = new InputNodeUi(NodeUiParam {
      mBus, pGrahics,
      250, 150,
      &mX, &mY, &mParameters, &mSocketsIn, &mSocketsOut, this
    });
    mUi->setColor(IColor(255, 100, 150, 100));
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};