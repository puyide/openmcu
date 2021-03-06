
#ifndef _MCU_RTSP_H
#define _MCU_RTSP_H

#include "sockets.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

static const PString METHOD_OPTIONS    = "OPTIONS";
static const PString METHOD_DESCRIBE   = "DESCRIBE";
static const PString METHOD_SETUP      = "SETUP";
static const PString METHOD_PLAY       = "PLAY";
static const PString METHOD_TEARDOWN   = "TEARDOWN";

class ConferenceStreamMember;

////////////////////////////////////////////////////////////////////////////////////////////////////

class MCURtspConnection : public MCUSipConnection
{
  friend class MCURtspServer;

  public:
    virtual BOOL ClearCall(CallEndReason reason = EndedByLocalUser);
    virtual void CleanUpOnCallEnd();

  protected:

    enum RtspStates
    {
      RTSP_NONE = 0,
      RTSP_CONNECT,
      RTSP_DESCRIBE,
      RTSP_SETUP,
      RTSP_SETUP_AUDIO,
      RTSP_SETUP_VIDEO,
      RTSP_PLAY,
      RTSP_PLAYING,
      RTSP_TEARDOWN
    };
    RtspStates rtsp_state;

    MCURtspConnection(MCUH323EndPoint *_ep, PString _callToken);
    ~MCURtspConnection();

    BOOL Connect(PString room, PString address);
    BOOL Connect(MCUSocket *socket, const msg_t *msg);

    void CreateLocalSipCaps();
    BOOL CreateInboundCaps();

    BOOL SendSetup(int pt);
    BOOL SendPlay();
    BOOL SendOptions();
    BOOL SendTeardown();
    BOOL SendDescribe();

    BOOL OnResponseReceived(const msg_t *msg);
    BOOL OnResponseDescribe(const msg_t *msg);
    BOOL OnResponseSetup(const msg_t *msg);
    BOOL OnResponsePlay(const msg_t *msg);

    BOOL OnRequestReceived(const msg_t *msg);
    BOOL OnRequestDescribe(const msg_t *msg);
    BOOL OnRequestSetup(const msg_t *msg);
    BOOL OnRequestPlay(const msg_t *msg);
    BOOL OnRequestTeardown(const msg_t *msg);
    BOOL OnRequestOptions(const msg_t *msg);

    BOOL RtspCheckAuth(const msg_t *msg);
    BOOL ParseTransportStr(SipCapability *sc, PString & transport_str);
    void AddHeaders(char *buffer, PString method_name="");
    BOOL SendRequest(char *buffer);

    int cseq;
    PString rtsp_session_str;
    PString rtsp_path;
    PString luri_str;

    static int OnReceived_wrap(void *context, MCUSocket *socket, PString data)
    { return ((MCURtspConnection *)context)->OnReceived(socket, data); }
    int OnReceived(MCUSocket *socket, PString data);

    MCUListener *listener;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class MCURtspServer
{
  public:
    MCURtspServer(MCUH323EndPoint *ep, MCUSipEndPoint *sep);
    ~MCURtspServer();

    bool CreateConnection(const PString & room, const PString & address, const PString & callToken);

    void StartListeners();
    void AddListener(PString address);
    void RemoveListener(PString address);
    BOOL HasListener(PString host, PString port);
    void RemoveListeners();

  protected:

    BOOL CreateConnection(MCUSocket *socket, const msg_t *msg);
    void SendResponse(MCUSocket *socket, const msg_t *msg, const PString & status_str);

    PString trace_section;

    static int OnReceived_wrap(void *context, MCUSocket *socket, PString data)
    { return ((MCURtspServer *)context)->OnReceived(socket, data); }
    int OnReceived(MCUSocket *socket, PString data);

    typedef std::map<PString /* address */, MCUListener *> ListenersMapType;
    ListenersMapType Listeners;

    MCUH323EndPoint *ep;
    MCUSipEndPoint *sep;

    PMutex rtspMutex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ConferenceStreamMember : public ConferenceMember
{
  PCLASSINFO(ConferenceStreamMember, ConferenceMember);

  public:
    ConferenceStreamMember(Conference *_conference, const PString & _callToken, const PString & _name)
      : ConferenceMember(_conference)
    {
      callToken = _callToken;
      name = _name;
      conference->AddMember(this);
    }
    ~ConferenceStreamMember()
    {
      if(conference)
        conference->RemoveMember(this);
    }

    virtual void Close();

    virtual PString GetName() const
    { return "stream "+name; }

    virtual MemberTypes GetType()
    { return MEMBER_TYPE_STREAM; }

    virtual BOOL IsVisible() const
    { return FALSE; }

};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // _MCU_RTSP_H
