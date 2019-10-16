#pragma once
#include "mutex.h"
#include "IPlugConstants.h"
#include "src/graph/Node.h"
#include "src/graph/nodes/DummyNode.h"
#include "src/constants.h"
#include "src/logger.h"
#include "thirdparty/json.hpp"
#include "src/graph/ui/GraphBackground.h"
#include "src/graph/misc/Serializer.h"
#include "src/graph/misc/ParameterManager.h"


class Graph {
  iplug::igraphics::IGraphics* graphics;
  /** Holds all the nodes in the processing graph */
  WDL_PtrList<Node> nodes;
  WDL_Mutex isProcessing;
  /** Dummy nodes to get the audio blocks in and out of the graph */
  DummyNode* input;
  DummyNode* output;
  int channelCount;

  GraphBackground* background;
public:
  ParameterManager paramManager;


  Graph(int p_sampleRate, int p_channles = 2) {
    graphics = nullptr;
    background = nullptr;
    sampleRate = p_sampleRate;
    channelCount = p_channles;
    input = new DummyNode();
    output = new DummyNode();
    output->channelCount = channelCount;
    output->inputs[0] = input;
  }

  ~Graph() {
    // TODO get rid of all the things
    // not a priority since there is no use case for multiple/dynamic graphs
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, int nFrames) {
    /**
     * I don't really like the mutex here, but it should only be locked if a change to the
     * processing chain is made, which will cause some artifacts anyways
     */
    WDL_MutexLock lock(&isProcessing);
    input->outputs[0] = in;
    input->isProcessed = true;
    int nodeCount = nodes.GetSize();
    for (int n = 0; n < nodeCount; n++) {
      nodes.Get(n)->isProcessed = false;
    }

    if (output->inputs[0] == nullptr) {
      // no out connected, so output nothing
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = 0;
        }
      }
      return;
    }
    // TODO multiple passes to ensure all the nodes are computed is super dumb
    while (!output->inputs[0]->isProcessed) {
      for (int n = 0; n < nodeCount; n++) {
        nodes.Get(n)->ProcessBlock(nFrames);
      }
    }
    output->CopyOut(out, nFrames);
  }

  void testAdd() {
    WDL_MutexLock lock(&isProcessing);
    if (nodes.GetSize() == 0) {
      Node* stereo = new StereoToolNode();
      addNode(stereo, input, 0);
      //Node* delay = new SimpleDelayNode();
      //addNode(delay, stereo, 200);
      Node* drive = new SimpleDriveNode();
      addNode(drive, stereo, 400);
      //Node* baby = new CryBabyNode();
      //addNode(baby, drive, 600);
      Node* cab = new SimpleCabNode();
      addNode(cab, drive, 800);
      output->inputs[0] = cab;
    }
    else {
      removeAllNodes();
        output->inputs[0] = input;
    }
  }

  /** The graph needs to know about the graphics context to add and remove the controlls for the nodes */
  void setupUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    background = new GraphBackground(pGraphics, [&](float x, float y, float scale) {
      this->onViewPortChange(x, y, scale);
    });
    pGraphics->AttachControl(background);

    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
    }
    for (int n = 0; n < nodes.GetSize(); n++) {
        nodes.Get(n)->setupUi(pGraphics);
    }
  }

  void cleanupUi() {
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->cleanupUi(graphics);
    }
    graphics = nullptr;
  }

  void onViewPortChange(float x, float y, float scale) {
    for (int i = 0; i < nodes.GetSize(); i++) {
      nodes.Get(i)->translate(x, y);
    }
    // WDBGMSG("x %f y %f s %f\n", x, y, scale);
  }

  void layoutUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
      // Todo find out whether the context ever changes
    }
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->layoutChanged();
    }
  }

  void serialize(nlohmann::json &json) {
    WDL_MutexLock lock(&isProcessing);
    json["output"] = { nodes.Find(output->inputs[0]), 0 };
    serializer::serialize(json, nodes, input);

  }

  void deserialize(nlohmann::json& json) {
    WDL_MutexLock lock(&isProcessing);
    removeAllNodes();
    serializer::deserialize(json, nodes, output, input, sampleRate, &paramManager, graphics);
  }

  void addNode(Node* node, Node* pInput, float y) {
    node->setup(sampleRate);
    node->L = y;
    paramManager.claimNode(node);
    node->setupUi(graphics);
    node->connectInput(pInput);
    nodes.Add(node);
  }

  void removeAllNodes() {
    while (nodes.GetSize()) {
      removeNode(0);
    }
  }

  void removeNode(int index) {
    Node* node = nodes.Get(index);
    if (node == output->inputs[0]) {
      output->inputs[0] = nullptr;
    }
    if (node != nullptr) {
      node->cleanupUi(graphics);
      paramManager.releaseNode(node);
      nodes.DeletePtr(node, true);
    }
  }

private:
  int sampleRate;
};
