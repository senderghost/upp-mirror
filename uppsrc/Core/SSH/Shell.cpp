#include "SSH.h"

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

bool SshShell::Run(int mode_, const String& terminal, Size pagesize)
{
	mode  = mode_;
	psize = pagesize;
	ssh->noloop = mode == CONSOLE; // FIXME: Not very pretty, but currently required for console i/o to work.
	
	return ComplexCmd(CHANNEL_SHELL, [=]() mutable {
		SshChannel::Open();
		SshChannel::Terminal(terminal, psize);
		Cmd(CHANNEL_SHELL, [=] { return X11Init(); });
		SshChannel::Shell();
		Cmd(CHANNEL_SHELL, [=] { Unlock(); return ConsoleInit(); });
		Cmd(CHANNEL_SHELL, [=] { return ProcessEvents(queue);  });
		SshChannel::SendRecvEof();
		SshChannel::Close();
		SshChannel::CloseWait();
	});
}

void SshShell::ReadWrite(String& in, const void* out, int out_len)
{
	switch(mode) {
		case GENERIC: {
			if(out_len > 0)
				WhenOutput(out, out_len);
#ifdef PLATFORM_POSIX
			if(xenabled)
				X11Loop();
#endif
			WhenInput();
			break;
		}
		case CONSOLE: {
			if(out_len > 0)
				ConsoleWrite(out, out_len);
#ifdef PLATFORM_POSIX
			if(xenabled)
				X11Loop();
			ConsoleRead();
			// We need to catch the WINCH signal. To this end we'll use a POSIX compliant kernel
			// function: sigtimedwait. To speed up, we'll simply poll for the monitored event.
			sigset_t set;
			sigemptyset(&set);
			sigaddset(&set, SIGWINCH);
			sigprocmask(SIG_BLOCK, &set, NULL);

			struct timespec timeout;
			Zero(timeout); // Instead of waiting, we simply poll.

			auto rc = sigtimedwait(&set, NULL, &timeout);
			if(rc < 0 && errno != EAGAIN)
				SetError(-1, "sigtimedwait() failed.");
			if(rc > 0)
				LLOG("SIGWINCH received.");
			resized = rc > 0;
#elif PLATFORM_WIN32
			// This part is a little bit tricky. We need to handle Windows console events here.
			// But we cannot simply ignore the events we don't look for. We need to actively
			// remove them from the event queue. Otherwise they'll cause erratic behaviour, and
			// a lot of head ache. Thus to filter out these unwanted events, or the event's that
			// we don't want to get in our way, we'll first peek at the console event queue to
			// see if they met our criteria and remove them one by one as we encounter, using
			// the ReadConsoleInput method.

			auto rc = WaitForSingleObject(stdinput, 10);
			switch(rc) {
				case WAIT_OBJECT_0:
					break;
				case WAIT_TIMEOUT:
				case WAIT_ABANDONED:
					return;
				default:
					SetError(-1, "WaitForSingleObject() failed.");
			}

			DWORD n = 0;
			INPUT_RECORD ir[1];

			if(!PeekConsoleInput(stdinput, ir, 1, &n))
				SetError(-1, "Unable to peek console input events.");
			if(n) {
				switch(ir[0].EventType) {
					case KEY_EVENT:
						// Ignore key-ups.
						if(!ir[0].Event.KeyEvent.bKeyDown)
							break;
						ConsoleRead();
						return;
					case WINDOW_BUFFER_SIZE_EVENT:
						LLOG("WINDOW_BUFFER_SIZE_EVENT received.");
						resized = true;
						break;
					case MENU_EVENT:
					case MOUSE_EVENT:
					case FOCUS_EVENT:
						break;
					default:
						SetError(-1, "Unknown console event type encountered.");
				}
				if(!ReadConsoleInput(stdinput, ir, 1, &n))
					SetError(-1, "Unable to filter console input events.");
			}
#endif
			break;
		}
		default:
			NEVER();
	}
	if(resized)
		Resize();
}

void SshShell::Resize()
{
	if(mode == CONSOLE)
		PageSize(GetConsolePageSize());

	int n = 0;
	do {
		n = SetPtySz(psize);
		Wait();
	}
	while(!IsTimeout() && !IsEof() && n < 0);
	resized = false;
}

bool SshShell::ConsoleInit()
{
	if(mode == CONSOLE) {
#ifdef PLATFORM_WIN32
		stdinput = GetStdHandle(STD_INPUT_HANDLE);
		if(!stdinput)
			SetError(-1, "Unable to obtain a handle for stdin.");
		stdoutput = GetStdHandle(STD_OUTPUT_HANDLE);
		if(!stdoutput)
			SetError(-1, "Unable to obtain a handle for stdout.");
#endif
		ConsoleRawMode();
	}
	return true;
}

#ifdef PLATFORM_POSIX
void SshShell::ConsoleRead()
{
	if(!EventWait(STDIN_FILENO, WAIT_READ, 0))
		return;
	Buffer<char> buffer(ssh->chunk_size);
	auto n = read(STDIN_FILENO, buffer, ssh->chunk_size);
	if(n > 0)
		Send(String(buffer, n));
	else
	if(n == -1 && errno != EAGAIN)
		SetError(-1, "Couldn't read input from console.");
}

void SshShell::ConsoleWrite(const void* buffer, int len)
{
	if(!EventWait(STDOUT_FILENO, WAIT_WRITE, 0))
		return;
	auto n = write(STDOUT_FILENO, buffer, len);
	if(n == -1 && errno != EAGAIN)
		SetError(-1, "Couldn't write output to console.");
}

void SshShell::ConsoleRawMode(bool b)
{
	if(!channel || mode != CONSOLE)
		return;

	if(b) {
		termios nflags;
		Zero(nflags);
		Zero(tflags);
		tcgetattr(STDIN_FILENO, &nflags);
		tflags = nflags;
		cfmakeraw(&nflags);
		tcsetattr(STDIN_FILENO, TCSANOW, &nflags);
	}
	else
	if(rawmode)
		tcsetattr(STDIN_FILENO, TCSANOW, &tflags);
	rawmode = b;
}

Size SshShell::GetConsolePageSize()
{
	winsize wsz;
	Zero(wsz);
	if(ioctl(STDIN_FILENO, TIOCGWINSZ, &wsz) == 0)
		return Size(wsz.ws_col, wsz.ws_row);
	LLOG("Warning: ioctl() failed. Couldn't read local terminal page size.");
	return Null;
}

#elif PLATFORM_WIN32

void SshShell::ConsoleRead()
{
	DWORD n = 0;
	const int RBUFSIZE = 1024 * 16;
	Buffer<char> buffer(RBUFSIZE);
	if(!ReadConsole(stdinput, buffer, RBUFSIZE, &n, NULL))
		SetError(-1, "Couldn't read input from console.");
	if(n > 0)
		Send(String(buffer, n));
}

void SshShell::ConsoleWrite(const void* buffer, int len)
{
	DWORD n = 0;
	if(!WriteConsole(stdoutput, buffer, len, &n, NULL))
		SetError(-1, "Couldn't Write output to console.");
}

void SshShell::ConsoleRawMode(bool b)
{
	if(!channel || mode != CONSOLE)
		return;

	if(b) {
		GetConsoleMode(stdinput, &tflags);
		DWORD nflags = tflags;
		nflags &= ~ENABLE_LINE_INPUT;
		nflags &= ~ENABLE_ECHO_INPUT;
		nflags |= ENABLE_WINDOW_INPUT;
		SetConsoleMode(stdinput, nflags);
	}
	else
	if(rawmode)
		SetConsoleMode(stdinput, tflags);
	rawmode = b;
}

Size SshShell::GetConsolePageSize()
{
	CONSOLE_SCREEN_BUFFER_INFO cinf;
	Zero(cinf);
	if(GetConsoleScreenBufferInfo((HANDLE) _get_osfhandle(1), &cinf))
		return Size(cinf.dwSize.X, cinf.dwSize.Y);
	LLOG("Warning: Couldn't read local terminal page size.");
	return Null;
}

#endif

bool SshShell::X11Init()
{
	if(!xenabled)
		return true;
#ifdef PLATFORM_POSIX
	auto rc = libssh2_channel_x11_req(*channel, xscreen);
	if(!WouldBlock(rc) && rc < 0)
		SetError(rc);
	if(rc == 0)
		LLOG("X11 tunnel succesfully initialized.");
	return rc == 0;
#elif PLATFORM_WIN32
	SetError(-1, "X11 tunneling is not (yet) supported on Windows platform");
	return false;
#endif
}

void SshShell::X11Loop()
{
#ifdef PLATFORM_POSIX
	if(xrequests.IsEmpty())
		return;

	for(auto i = 0; i < xrequests.GetCount(); i++) {
		auto* chan = xrequests[i].Get<SshX11Connection*>();
		auto  sock = xrequests[i].Get<SOCKET>();

		if(EventWait(sock, WAIT_WRITE, 0)) {
			auto rc = libssh2_channel_read(chan, xbuffer, xbuflen);
			if(!WouldBlock(rc) && rc < 0)
				SetError(-1, "[X11]: Read failed.");
			if(rc > 0)
				write(sock, xbuffer, rc);
		}
		if(EventWait(sock, WAIT_READ, 0)) {
			auto rc =  read(sock, xbuffer, xbuflen);
			if(rc > 0)
				libssh2_channel_write(chan, (const char*) xbuffer, rc);
		}
		if(libssh2_channel_eof(chan) == 1) {
			LLOG("[X11] EOF received.");
			close(sock);
			xrequests.Remove(i);
			i = 0;
		}
	}
#endif
}

SshShell& SshShell::ForwardX11(const String& host, int display, int screen, int bufsize)
{
	if(!xenabled) {
		xenabled = true;
#ifdef PLATFORM_POSIX
		xhost    = host;
		xdisplay = display;
		xscreen  = screen;
		xbuflen  = clamp(bufsize, ssh->chunk_size, INT_MAX);
		xbuffer.Alloc(xbuflen);
#endif
	}
	return *this;
}

bool SshShell::AcceptX11(SshX11Connection* x11conn)
{
#ifdef PLATFORM_POSIX
	if(x11conn && xenabled) {
		auto sock = socket(AF_UNIX, SOCK_STREAM, 0);
		if(sock < 0) {
			LLOG("Couldn't create UNIX socket.");
			return false;
		}
		auto path = Format("%s/.X11-unix/X%d", GetTempPath(), xdisplay);

		struct sockaddr_un addr;
		Zero(addr);
		addr.sun_family = AF_UNIX;
		memcpy(addr.sun_path, ~path, path.GetLength());

		if(connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
			LLOG("Couldn't connect to " << path);
			close(sock);
		}

		LLOG("X11 connection accepted.");

		auto& xr = xrequests.Add();
		xr.Get<SshX11Connection*>() = x11conn;
		xr.Get<SOCKET>() = sock;
		return true;
	}
#endif
	return false;
}

AsyncWork<void> SshShell::AsyncRun(SshSession& session, String terminal, Size pagesize, Event<SshShell&> in, Event<const String&> out)
{
	auto work = Async([=, &session, in = pick(in), out = pick(out)]{
		SshShell worker(session);
		worker.NonBlocking();
	
		bool cancelled = false;
		int  waitstep  = worker.GetWaitStep();
		
		worker.WhenInput = [&worker, &in]() {
			in(worker);
		};
		worker.WhenOutput = [&out](const void* buf, int len) {
			String s((const char*) buf, len);
			out(s);
		};

		worker.Run(terminal, pagesize);
		
		while(worker.Do()) {
			if(!cancelled && CoWork::IsCanceled()) {
				worker.Cancel();
				cancelled = true;
			}
			Sleep(waitstep);
		}
		if(worker.IsError())
			throw Ssh::Error(worker.GetError(), worker.GetErrorDesc());
	});
	return pick(work);
}

SshShell::SshShell(SshSession& session)
: SshChannel(session)
{
	ssh->otype	= SHELL;
	mode		= GENERIC;
	rawmode     = false;
	resized		= false;
	xenabled    = false;
#ifdef PLATFORM_POSIX
	xscreen		= 0;
	xdisplay	= 0;
	xenabled	= false;
	xbuflen		= 1024 * 1024;
#elif PLATFORM_WIN32
	stdinput	= NULL;
	stdoutput	= NULL;
#endif

	Zero(tflags);
}

SshShell::~SshShell()
{
	ConsoleRawMode(false);
}
}