#pragma once

#include "IPlugConstants.h"

class Node
{
public:
 
  Node** inputs;
  iplug::sample*** outputs;

  int inputCount;
  int outputCount;
  bool isProcessed;

  Node(int p_samplerate, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    samplerate = p_samplerate;
    maxBuffer = p_maxBuffer;
    inputCount = p_inputs;
    outputCount = p_outputs;
    isProcessed = false;
    channelCount = p_channles;

    inputs = new Node*[p_inputs];
    for (int i = 0; i < inputCount; i++) {
      inputs[i] = nullptr;
    }

    outputs = new iplug::sample**[p_outputs];
    for (int i = 0; i < p_outputs; i++) {
      outputs[i] = new iplug::sample*[p_channles];
      for (int c = 0; c < p_channles; c++) {
        outputs[i][c] = new iplug::sample[p_maxBuffer];
      }
    }
  };

  ~Node() {
    delete inputs;
    for (int i = 0; i < outputCount; i++) {
      for (int c = 0; c < channelCount; c++) {
        delete outputs[i][c];
      }
      delete outputs[i];
    }
    delete outputs;
  }

  virtual void ProcessBlock(iplug::sample** in, iplug::sample** out, int nFrames) = 0;

  virtual void ConnectInput(Node* in, int inputNumber = 0) {
    if (inputNumber < inputCount) {
      inputs[inputNumber] = in;
    }
  }

  virtual void DisconnectInput(int inputNumber = 0) {
    if (inputNumber < inputCount) {
      inputs[inputNumber] = nullptr;
    }
  }

  int samplerate;
  int channelCount;
  int maxBuffer;
};
