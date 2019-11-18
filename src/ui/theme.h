#pragma once
#include "config.h"
#include "IPlugStructs.h"

#define DEBUGFONT iplug::igraphics::IText { 16, COLOR_BLACK, "Roboto-Regular", iplug::igraphics::EAlign::Center, iplug::igraphics::EVAlign::Middle, 0 }
#define ICONFONT iplug::igraphics::IText {16.f, COLOR_WHITE, "ForkAwesome"}

#define COLORACCENT 233, 140, 36

/** Graph background */
#define COLORBACKGROUND 230, 230, 230
#define COLORBACKGROUNDDETAIL 200, 200, 200
#define BACKGROUNDDETAILDIST  140
#define BACKGROUNDDETAILSIZE  12
#define BACKGROUNDDETAILWIDTH  1.5

/** Node gallery */
#define GALLERYADDCIRCLEDIAMETER 48
#define GALLERYADDCIRCLERADIUS GALLERYADDCIRCLEDIAMETER * 0.5
#define GALLERYADDSIGNSIZE  18
#define GALLERYADDSIGNWIDTH  3
#define GALLERYPADDING 10
#define GALLERYBACKGROUND 180, 180, 180

#define GALLERYELEMENTTITLE iplug::igraphics::IText{ 18, COLOR_WHITE, "Roboto-Regular", EAlign::Center, EVAlign::Bottom, 0 }
#define GALLERYELEMENTTITLEHEIGHT 28
#define GALLERYELEMENTHEIGHT 110
#define GALLERYELEMENTWIDTH 200
#define GALLERYELEMENTPADDING 8

#define GALLERYCATEGORYTITLEBACKROUND 100, 100, 100
#define GALLERYCATEGORYTITLE IText { 24, COLOR_WHITE, "Roboto-Regular", EAlign::Center, EVAlign::Middle, 0 }
#define GALLERYCATEGORYPADDING 8
#define GALLERYCATEGORYBACKGROUND 30, 30, 30



/** Nodes */
#define NODEHEADERSIZE 20
#define NODEHEADERCOLOR 30, 30, 30
#define NODEHEADERBYPASSLEFT 6
#define NODEHEADERBYPASSTOP 2
#define NODEHEADERBYPASSSIZE 16

#define NODEHEADERDISCONNECTRIGHT 26
#define NODEHEADERDISCONNECTTOP 2
#define NODEHEADERDISCONNECTSIZE 20

#define NODEHEADERREMOVERIGHT 6

#define NODESCACHEBG
#define NODESROUNDEDCORNER
#define NODESSHADOW
#define NODESHADOWDIST 2.f
#define NODESHADOWBOUNDS 15.f
#define NODESHADOWBLUR 15.f
#define NODESHADOWROUND 15.f
const iplug::igraphics::IColor NODESHADOWCOLOR(255, 0, 0, 0);

/** NODE SOCKETS */
#define SOCKETDIAMETER 30
#define SOCKETRADIUS SOCKETDIAMETER * 0.5
#define SOCKETATIVESIZE SOCKETRADIUS * 0.7
#define SOCKETOUTLINESIZE 1.1
const iplug::igraphics::IColor SOCKETCOLOR(255, 40, 40, 40);
const iplug::igraphics::IColor SOCKETCOLORINNER(255, 250, 250, 250);
const iplug::igraphics::IColor SOCKETCOLORACITVE(255, 250, 150, 0);

/**   CABLE LAYER */
#define CABLETHICKNESS 5
#define CABLEPREVIEWDASHDIST 20
const iplug::igraphics::IColor CABLECOLOR(255, 40, 40, 40);
const iplug::igraphics::IColor CABLECOLORSPLICE(255, 250, 150, 0);
const iplug::igraphics::IColor CABLECOLORPREVIEW(150, 40, 40, 40);