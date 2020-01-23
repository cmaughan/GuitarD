#pragma once

// Some structs used to pass around bundled data with the MessageBus

struct Coord2D {
  float x = 0;
  float y = 0;
};

class Node;
struct NodeSocket;

struct QuickConnectRequest {
  Coord2D pos;
  NodeSocket* from;
};

struct NodeSpliceInPair {
  Node* node = nullptr;
  NodeSocket* socket = nullptr;
};

struct GraphStats {
  long long executionTime = 0;
  int nodeCount = 0;
  bool valid = true;
};

struct SocketConnectRequest {
  NodeSocket* from = nullptr;
  NodeSocket* to = nullptr;
};

struct ConnectionDragData {
  bool dragging = false;
  float startY = 0;
  float startX = 0;
  float currentY = 0;
  float currentX = 0;
};

struct AutomationAttachRequest {
  Node* automationNode = nullptr;
  iplug::igraphics::IControl* targetControl = nullptr;
};

struct NodeDragEndData {
  Node* node;
  bool addCombineNode;
};


/**
 * Bundles an impulse response with some meta data about it
 */
struct IRBundle {
  WDL_String name;
  int channelCount = 1;
  int sampleRate = 48000;
  // Samplecount for a single channel
  size_t sampleCount = 0;
  float** samples = nullptr;
  // This is only set if it's a user IR
  WDL_String path;
};

struct BlockSizeEvent {
  int blockSize;
  bool set = false;
};