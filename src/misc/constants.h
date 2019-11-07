// Wrap the debug message from iplug in the iplug namespace on windows
// since using iplug::DBGMSG will exapnd to iplug::printf() and fail on linux/osx

#if defined(OS_MAC) || defined(OS_LINUX) || defined(OS_WEB) || defined(OS_IOS) || NDEBUG
  #define WDBGMSG(...) DBGMSG(__VA_ARGS__)
#elif defined OS_WIN
  #define WDBGMSG(...) iplug::DBGMSG(__VA_ARGS__)
#endif


// Max amount of nodes in the graph
#define MAXNODES 128

// Amounts of daw params to register at start since dynamic amounts are not well supported
#define MAXDAWPARAMS 256

// This is the name of a node if it wasn't overridden anywhere
#define DefaultNodeName "DEFAULTNODENAME"

#define MAXBUFFER 512

#define FLOATCONV
#ifdef FLOATCONV
  #define WDL_RESAMPLE_TYPE float
  #define FFTCONVOLVER_USE_SSE
#else
  #define WDL_RESAMPLE_TYPE double
#endif