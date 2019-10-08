#include "GuitarD.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "src/graph/TestUiNode.h"
#include "thirdparty/json.hpp"

GuitarD::GuitarD(const InstanceInfo& info)
: Plugin(info, MakeConfig(MAXDAWPARAMS, kNumPrograms))
{

  // TODO on mac garageband NOutChansConnected() reports zero for some reason
  graph = new Graph(GetSampleRate(), 2);

  // Gather a good amount of parameters to expose to the daw based on what nodes are on the canvas
  for (int i = 0; i < MAXDAWPARAMS; i++) {
    graph->paramManager.addParameter(GetParam(i));
  }


#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {


    if (pGraphics->NControls()) {
      this->graph->layoutUi(pGraphics);
      return;
    }
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, true);
    pGraphics->AttachPanelBackground(COLOR_GRAY);

    const IRECT b = pGraphics->GetBounds();
    auto buttonAction = [&, pGraphics, b](IControl* pCaller) {
      SplashClickActionFunc(pCaller);
      graph->testAdd();
    };

    pGraphics->AttachControl(
      new IVButtonControl(b.GetCentredInside(100).GetVShifted(100), buttonAction),
      kNoParameter, "vcontrols"
    );

    this->graph->setupUi(pGraphics);

    //pGraphics->AttachControl(
    //  new UiNode(b.GetCentredInside(100).GetVShifted(200)),
    //  kNoParameter, "vcontrols"
    //);
  };
#endif
}

void GuitarD::OnUIClose() {
  // The gui will be cleaned up when the Iplug window is destructed
  // however doint this manually will be safer and make sure all the nodes can clean up
  // after them selves and set the control in the ParameterCoupling to nullptr
  graph->cleanupUi();
}

bool GuitarD::SerializeState(IByteChunk& chunk) const {
  TRACE;
  nlohmann::json serialized = {
    {"version", PLUG_VERSION_STR},
    {"node_count", graph->nodeCount},
    {"ui_scale", 1.0} // TODO get the proper scale
  };

  // get all the nodes a index, TODO move this over to the constructor since this won't change over lifetime of a node
  for (int i = 0; i < MAXNODES; i++) {
    if (graph->nodes[i] != nullptr) {
      graph->nodes[i]->index = i;
    }
  }
  serialized["output"] = { graph->output->inputs[0]->index, 0 };
  serialized["nodes"] = nlohmann::json::array();
  for (int i = 0, pos = 0; i < MAXNODES; i++) {
    Node* node = graph->nodes[i];
    if (node != nullptr) {
      serialized["nodes"][pos]["idx"] = i;
      serialized["nodes"][pos]["type"] = node->type;
      serialized["nodes"][pos]["inputs"] = nlohmann::json::array();
      for (int prev = 0; prev < node->inputCount; prev++) {
        serialized["nodes"][pos]["inputs"][prev] = node->inputs[prev]->index;
      }
      serialized["nodes"][pos]["parameters"] = nlohmann::json::array();
      for (int p = 0; p < node->parameterCount; p++) {
        serialized["nodes"][pos]["parameters"][p] = {
          { "idx", node->parameters[p]->parameterId },
          { "value", *(node->parameters[p]->value) },
          { "position", { node->x, node->y } }
        };
      }
      pos++;
    }
  }

  chunk.PutStr(serialized.dump(4).c_str());
  return true;
  return IPluginBase::SerializeParams(chunk);
}

int GuitarD::UnserializeState(const IByteChunk& chunk, int startPos) {
  TRACE;
  WDL_String json_string;
  chunk.GetStr(json_string, startPos);

  return 0;
  return IPluginBase::UnserializeParams(chunk, startPos);
}

#if IPLUG_DSP
void GuitarD::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  graph->ProcessBlock(inputs, outputs, nFrames);
}
#endif
