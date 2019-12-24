#pragma once

#include "IControl.h"

using namespace iplug;
using namespace igraphics;

class ScrollViewControl : public IControl {
  WDL_PtrList<IControl> mChildren;
  float mScrollY = 0;
  float mContentHeight = 0;
  float mContentWidth = 0;
  IControl* mMouseOver = nullptr;
  float mChildPaddingY = 10;
  float mDistanceDragged = -1;
  float mDragThreshold = 4;
  bool mAttached = false;
  bool mFullWidthChildren = false;
public:
  ScrollViewControl(IRECT bounds) : IControl(bounds) {}

  ScrollViewControl() : IControl({}) {}

  void appendChild(IControl* child) {
    mChildren.Add(child);
    if (mAttached) {
      child->SetDelegate(*GetDelegate());
    }
    mDirty = true;
  }

  void appendChild(IControl& child) {
    appendChild(&child);
  }

  void OnInit() override {
    IControl::OnInit();
    for (int i = 0; i < mChildren.GetSize(); i++) {
      IControl* c = mChildren.Get(i);
      c->SetDelegate(*GetDelegate());
    }
    mAttached = true;
  }

  bool removeChild(IControl& child, const bool wantsDelete = false) {
    int index = mChildren.Find(&child);
    if (index != -1) {
      mChildren.Delete(index, wantsDelete);
      mDirty = true;
      return true;
    }
    return false;
  }

  static void shiftRectY(IRECT& r, const float y) {
    r.T += y;
    r.B += y;
  }

  void setChildPadding(float padding) {
    mChildPaddingY = padding;
    mDirty = true;
  }

  void setFullWidthChildren(bool full) {
    mFullWidthChildren = full;
    mDirty = true;
  }

  void scroll(float y) {
    mScrollY += y;
    mDirty = true;
  }

  void layout() {
    mContentHeight = 0;
    mContentWidth = 0;
    const int childCount = mChildren.GetSize();
    if (childCount == 0) { return; }
    for (int i = 0; i < childCount; i++) {
      const bool isLast = i == childCount - 1;
      IControl* c = mChildren.Get(i);
      IRECT r = c->GetTargetRECT();
      const float height = r.H();
      const float width = mFullWidthChildren ? mRECT.W() : r.W();
      if (width > mContentWidth) { mContentWidth = width; }
      r.L = r.T = 0;
      r.R = width;
      r.B = height;
      shiftRectY(r, mContentHeight);
      mContentHeight += height + (isLast ? 0.f : mChildPaddingY);
      c->SetTargetRECT(r);
      if (isLast) {
        const float scrollLimit = mRECT.H();
        if (r.B - mScrollY < scrollLimit) {
          mScrollY = r.B - scrollLimit;
        }
      }
    }
    if (mContentHeight > mChildPaddingY) {
      mContentHeight -= mChildPaddingY;
    }
    if (mScrollY < 0) {
      mScrollY = 0;
    }
    for (int i = 0; i < mChildren.GetSize(); i++) {
      IControl* c = mChildren.Get(i);
      IRECT r = c->GetTargetRECT();
      shiftRectY(r, -mScrollY);
      r.Translate(mRECT.L, mRECT.T);
      c->SetTargetAndDrawRECTs(r);
    }
  }

  void Draw(IGraphics& g) override {
    layout();
    g.FillRect(COLOR_DARK_GRAY, mRECT);
    for (int i = 0; i < mChildren.GetSize(); i++) {
      mChildren.Get(i)->Draw(g);
    }
  }


  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    mScrollY -= d * 20;
    mDirty = true;
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mScrollY -= dY;
    mDistanceDragged += abs(dX) + abs(dY);
    mDirty = true;
  }

  IControl* getChildAtCoord(const float x, const float y) const {
    IRECT click = { x, y, x, y };
    for (int i = 0; i < mChildren.GetSize(); i++) {
      IControl* c = mChildren.Get(i);
      IRECT r = c->GetTargetRECT();
      if (r.Contains(click)) {
        return c;
      }
    }
    return nullptr;
  }

  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
    IControl* target = getChildAtCoord(x, y);
    if (target != nullptr) {
      target->OnMouseDblClick(x, y, mod);
      mDirty = true;
    }
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    if (mDistanceDragged < mDragThreshold) {
      IControl* target = getChildAtCoord(x, y);
      if (target != nullptr) {
        target->OnMouseDown(x, y, mod);
        mDirty = true;
      }
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    if (mDistanceDragged < mDragThreshold) {
      IControl* target = getChildAtCoord(x, y);
      if (target != nullptr) {
        target->OnMouseUp(x, y, mod);
        mDirty = true;
      }
    }
    mDistanceDragged = 0;
  }

  void OnMouseOver(float x, float y, const IMouseMod& mod) override {
    IControl::OnMouseOver(x, y, mod);
    IControl* target = getChildAtCoord(x, y);
    if (mMouseOver != target && mMouseOver != nullptr) {
      mMouseOver->OnMouseOut();
    }
    if (target != nullptr) {
      target->OnMouseOver(x, y, mod);
      mDirty = true;
    }
    mMouseOver = target;
  }

  //virtual bool OnKeyDown(float x, float y, const IKeyPress& key) { return false; }

  //virtual bool OnKeyUp(float x, float y, const IKeyPress& key) { return false; }

  ~ScrollViewControl() {
    mChildren.Empty(true);
  }
};
