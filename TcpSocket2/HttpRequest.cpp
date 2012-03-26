#include <Core/Core.h>
#include "TcpSocket.h"

NAMESPACE_UPP

bool HttpRequest_Trace__;

#ifdef _DEBUG
#define LLOG(x)      LOG(x)
#else
#define LLOG(x)      if(HttpRequest_Trace__) RLOG(x); else;
#endif

void HttpRequest::Trace(bool b)
{
	HttpRequest_Trace__ = b;
}

void HttpRequest::Init()
{
	port = 0;
	timeout_msecs = DEFAULT_TIMEOUT_MSECS;
	max_header_size = DEFAULT_MAX_HEADER_SIZE;
	max_content_size = DEFAULT_MAX_CONTENT_SIZE;
	keepalive = false;
	force_digest = false;
	std_headers = true;
	hasurlvar = false;
	method = METHOD_GET;
}


HttpRequest::HttpRequest()
{
	Init();
}

HttpRequest::HttpRequest(const char *url)
{
	Init();
	URL(url);
}

HttpRequest& HttpRequest::URL(const char *u)
{
	const char *t = u;
	while(*t && *t != '?')
		if(*t++ == '/' && *t == '/') {
			u = ++t;
			break;
		}
	t = u;
	while(*u && *u != ':' && *u != '/' && *u != '?')
		u++;
	if(*u == '?' && u[1])
		hasurlvar = true;
	host = String(t, u);
	port = 0;
	if(*u == ':')
		port = ScanInt(u + 1, &u);
	path = u;
	int q = path.Find('#');
	if(q >= 0)
		path.Trim(q);
	return *this;
}

HttpRequest& HttpRequest::Proxy(const char *p)
{
	const char *t = p;
	while(*p && *p != ':')
		p++;
	proxy_host = String(t, p);
	proxy_port = 80;
	if(*p++ == ':' && IsDigit(*p))
		proxy_port = ScanInt(p);
	return *this;
}

HttpRequest& HttpRequest::Post(const char *id, const String& data)
{
	Post();
	if(postdata.GetCount())
		postdata << '&';
	postdata << id << '=' << UrlEncode(data);
	return *this;
}

HttpRequest& HttpRequest::UrlVar(const char *id, const String& data)
{
	int c = *path.Last();
	if(hasurlvar && c != '&')
		path << '&';
	if(!hasurlvar && c != '?')
		path << '?';
	path << id << '=' << UrlEncode(data);
	hasurlvar = true;
	return *this;
}

String HttpRequest::CalculateDigest(String authenticate) const
{
	const char *p = authenticate;
	String realm, qop, nonce, opaque;
	while(*p) {
		if(!IsAlNum(*p)) {
			p++;
			continue;
		}
		else {
			const char *b = p;
			while(IsAlNum(*p))
				p++;
			String var = ToLower(String(b, p));
			String value;
			while(*p && (byte)*p <= ' ')
				p++;
			if(*p == '=') {
				p++;
				while(*p && (byte)*p <= ' ')
					p++;
				if(*p == '\"') {
					p++;
					while(*p && *p != '\"')
						if(*p != '\\' || *++p)
							value.Cat(*p++);
					if(*p == '\"')
						p++;
				}
				else {
					b = p;
					while(*p && *p != ',' && (byte)*p > ' ')
						p++;
					value = String(b, p);
				}
			}
			if(var == "realm")
				realm = value;
			else if(var == "qop")
				qop = value;
			else if(var == "nonce")
				nonce = value;
			else if(var == "opaque")
				opaque = value;
		}
	}
	String hv1, hv2;
	hv1 << username << ':' << realm << ':' << password;
	String ha1 = MD5String(hv1);
	hv2 << (method == METHOD_GET ? "GET" : method == METHOD_PUT ? "PUT" : method == METHOD_POST ? "POST" : "READ")
	<< ':' << path;
	String ha2 = MD5String(hv2);
	int nc = 1;
	String cnonce = FormatIntHex(Random(), 8);
	String hv;
	hv << ha1
	<< ':' << nonce
	<< ':' << FormatIntHex(nc, 8)
	<< ':' << cnonce
	<< ':' << qop << ':' << ha2;
	String ha = MD5String(hv);
	String auth;
	auth
	<< "username=" << AsCString(username)
	<< ", realm=" << AsCString(realm)
	<< ", nonce=" << AsCString(nonce)
	<< ", uri=" << AsCString(path)
	<< ", qop=" << AsCString(qop)
	<< ", nc=" << AsCString(FormatIntHex(nc, 8))
	<< ", cnonce=" << cnonce
	<< ", response=" << AsCString(ha);
	if(!IsNull(opaque))
		auth << ", opaque=" << AsCString(opaque);
	return auth;
}

HttpRequest& HttpRequest::Header(const char *id, const String& data)
{
	client_headers << id << ": " << data << "\r\n";
	return *this;
}

void HttpRequest::HttpError(const char *s)
{
	error = NFormat(t_("%s:%d: ") + String(s), host, port);
	Close();
}

bool HttpRequest::Problem()
{
	if(IsTimeout()) {
		HttpError("connection timed out");
		return true;
	}
	if(IsAbort()) {
		HttpError("connection was aborted");
		return true;
	}
	if(IsSocketError()) {
		Close();
		return true;
	}
	return false;
}

String HttpRequest::Execute0()
{
	server_headers = Null;
	status_line = Null;
	status_code = 0;
	is_redirect = false;
	redirect_url = Null;
	if(IsOpen())
		Close();
	error = Null;
	use_proxy = !IsNull(proxy_host);
	socket_host = (use_proxy ? proxy_host : host);
	socket_port = (use_proxy ? proxy_port : port);

	LLOG("socket host = " << socket_host << ":" << socket_port);

	if(!Connect(socket_host, socket_port ? socket_port : DEFAULT_HTTP_PORT)) {
		return String::GetVoid();
	}

	if(!WaitWrite()) {
		error = NFormat(t_("%s:%d: connecting to host timed out"), socket_host, socket_port);
		Close();
		return String::GetVoid();
	}

	String request;
	String ctype = contenttype;
	switch(method) {
		case METHOD_GET:  request << "GET "; break;
		case METHOD_POST:
			request << "POST ";
			if(IsNull(ctype))
				ctype = "application/x-www-form-urlencoded";
			break;
		case METHOD_PUT:
			request << "PUT ";
			if(IsNull(ctype))
				ctype = "application/x-www-form-urlencoded";
			break;
		case METHOD_HEAD: request << "HEAD "; break;
		default: NEVER(); // invalid method
	}
	String host_port = host;
	if(port)
		host_port << ':' << port;
	String url;
	url << "http://" << host_port << Nvl(path, "/");
	if(use_proxy)
		request << url;
	else
		request << Nvl(path, "/");
	request << " HTTP/1.1\r\n";
	if(std_headers) {
		request
			<< "URL: " << url << "\r\n"
			<< "Host: " << host_port << "\r\n"
			<< "Connection: " << (keepalive ? "keep-alive" : "close") << "\r\n";
		if(keepalive)
			request << "Keep-alive: 300\r\n"; // 5 minutes (?)
		request << "Accept: " << Nvl(accept, "*/*") << "\r\n";
		request << "Accept-Encoding: gzip\r\n";
		request << "Agent: " << Nvl(agent, "Ultimate++ HTTP client") << "\r\n";
		if(method == METHOD_POST || method == METHOD_PUT)
			request << "Content-Length: " << postdata.GetLength() << "\r\n";
		if(ctype.GetCount())
			request << "Content-Type: " << ctype << "\r\n";
	}
	if(use_proxy && !IsNull(proxy_username))
		 request << "Proxy-Authorization: Basic " << Base64Encode(proxy_username + ':' + proxy_password) << "\r\n";
	if(!IsNull(digest))
		request << "Authorization: Digest " << digest << "\r\n";
	else if(!force_digest && (!IsNull(username) || !IsNull(password)))
		request << "Authorization: Basic " << Base64Encode(username + ":" + password) << "\r\n";
	request << client_headers << "\r\n" << postdata;
	LLOG("host = " << host << ", port = " << port);
	LLOG("request: " << request);

	if(!PutAll(request)) {
		Problem();
		return String::GetVoid();
	}

	bool expect_status = true;
	
	int content_length = -1;
	bool tc_chunked = false;
	bool ce_gzip = false;
	for(;;) {
		String line = GetLine();
		LLOG("< " << line);
		if(Problem())
			return String::GetVoid();
		if(expect_status) {
			status_line = line;
			if(status_line.GetLength() < 5 || MemICmp(status_line, "HTTP/", 5)) {
				HttpError("invalid server response: " + status_line);
				return String::GetVoid();
			}
	
			status_code = 0;
			const char *p = status_line.Begin() + 5;
			while(*p && *p != ' ')
				p++;
			if(*p == ' ' && IsDigit(*++p))
				status_code = stou(p);
			
			is_redirect = (status_code >= 300 && status_code < 400);
			expect_status = false;
			continue;
		}

		const char *p = line;
		for(p = line; *p && (byte)*p <= ' '; p++)
			;
		const char *b = p, *e = line.End();
		while(e > b && (byte)e[-1] < ' ')
			e--;
		if(b >= e) {
			if(status_code == 100) {
				expect_status = true;
				continue;
			}
			break;
		}
		static const char cl[] = "content-length:";
		static const char ce[] = "content-encoding:";
		static const char te[] = "transfer-encoding:";
		static const char lo[] = "location:";
		static const char au[] = "www-authenticate:";
		static const int CL_LENGTH = sizeof(cl) - 1;
		static const int CE_LENGTH = sizeof(ce) - 1;
		static const int TE_LENGTH = sizeof(te) - 1;
		static const int LO_LENGTH = sizeof(lo) - 1;
		static const int AU_LENGTH = sizeof(au) - 1;
		if(!MemICmp(p, cl, CL_LENGTH)) {
			for(p += CL_LENGTH; *p == ' '; p++)
				;
			if(IsDigit(*p)) {
				content_length = stou(p);
				if(content_length > max_content_size) {
					error = NFormat(t_("%s:%d: maximum data length exceeded (%d B)"), host, port, max_content_size);
					Close();
					return String::GetVoid();
				}
			}
		}
		else if(!MemICmp(p, ce, CE_LENGTH)) {
			for(p += CE_LENGTH; *p == ' '; p++)
				;
			static const char gzip[] = "gzip";
			if(e - p == sizeof(gzip) - 1 && !memcmp(p, gzip, sizeof(gzip) - 1))
				ce_gzip = true;
		}
		else if(!MemICmp(p, te, TE_LENGTH)) {
			for(p += TE_LENGTH; *p == ' '; p++)
				;
			static const char ch[] = "chunked";
			if(e - p == sizeof(ch) - 1 && !memcmp(p, ch, sizeof(ch) - 1))
				tc_chunked = true;
		}
		else if(!MemICmp(p, lo, LO_LENGTH)) {
			for(p += LO_LENGTH; *p == ' '; p++)
				;
			redirect_url = String(p, e);
			int q = redirect_url.Find('?');
			int p = path.Find('?');
			if(p >= 0 && q < 0)
				redirect_url.Cat(path.GetIter(p));
		}
		else if(!MemICmp(p, au, AU_LENGTH)) {
			for(p += AU_LENGTH; *p == ' '; p++)
				;
			authenticate = String(p, e);
		}
		if(server_headers.GetLength() + (e - b) + 2 > max_header_size) {
			error = NFormat(t_("%s:%d: maximum header length exceeded (%d B)"), host, port, max_header_size);
			Close();
			return String::GetVoid();
		}
		server_headers.Cat(b, int(e - b));
		server_headers.Cat("\r\n");
	}

	if(method == METHOD_HEAD) {
		Close();
		return String::GetVoid();
	}
	String chunked;
	body.Clear();

	while(body.GetLength() < content_length || content_length < 0 || tc_chunked) {
		if(Problem())
			return String::GetVoid();
		String part = TcpSocket::Get(2048);
		LLOG("received part: " << part.GetLength());
		if(!part.IsEmpty()) {
			if(body.GetLength() + part.GetLength() > max_content_size) {
				HttpError("maximum content size exceeded: " + AsString(body.GetLength() + part.GetLength()));
				goto EXIT;
			}
			body.Cat(part);
			if(tc_chunked)
				for(;;) {
					const char *p = body.Begin(), *e = body.End();
					while(p < e && *p != '\n')
						p++;
					if(p >= e)
						break;
					int nextline = int(p + 1 - body.Begin());
					p = body.Begin();
					int part_length = ctoi(*p);
					if((unsigned)part_length >= 16) {
						body.Remove(0, nextline);
						continue;
					}
					for(int i; (unsigned)(i = ctoi(*++p)) < 16; part_length = part_length * 16 + i)
						;
					body.Remove(0, nextline);
					if(part_length <= 0) {
						for(;;) {
							const char *b = body.Begin();
							p = b;
							while(*p && *p != '\n')
								p++;
							if(!*p && !IsEof()) {
								if(Problem())
									return String::GetVoid();
								if(body.GetLength() > 3)
									body.Remove(0, body.GetLength() - 3);
								String part = TcpSocket::Get(2048);
								body.Cat(part);
								continue;
							}
							const char *l = p;
							if(*p == '\n')
								p++;
							if(l > b && l[-1] == '\r')
								l--;
							if(l == b)
								break;
							server_headers.Cat(b, int(p - b));
						}
						goto EXIT;
					}
					if(body.GetLength() >= part_length) {
						chunked.Cat(body, part_length);
						body.Remove(0, part_length);
						continue;
					}
					for(;;) {
						String part = TcpSocket::Get(2048);
						DLOG("Chunked part reading len: " << part.GetCount());
						if(Problem())
							return String::GetVoid();
						body.Cat(part);
						if(body.GetLength() >= part_length) {
							chunked.Cat(body, part_length);
							body.Remove(0, part_length);
							break;
						}
					}
				}
		}

		if(!IsOpen() || Problem()) {
			LLOG("-> partial input: open = " << IsOpen()
				<< ", error " << IsError() << " (" << GetErrorDesc() << ")" << ", eof " << IsEof());
			if(!tc_chunked && content_length > 0 && body.GetLength() < content_length)
				HttpError(Format(t_("Partial input: %d out of %d"), body.GetLength(), content_length));
			break;
		}
		DDUMP(body.GetLength());
		DDUMP(content_length);
	}

EXIT:
	Problem();
	if(!keepalive)
		Close();

	if(tc_chunked)
		body = chunked;

	if(ce_gzip)
		body = GZDecompress(body);
	return body;
}

String HttpRequest::Execute(int max_redirect, int retries)
{
	int nredir = 0;
	for(;;) {
		String data = Execute0();
		if(status_code == 401 && !IsNull(username) && !IsNull(authenticate)) {
			if(++nredir > max_redirect) {
				error = NFormat("Maximum number of digest authentication attempts exceeded: %d", max_redirect);
				return String::GetVoid();
			}
			Digest(CalculateDigest(authenticate));
			continue;
		}
		if(status_code >= 400 && status_code < 500) {
			error = status_line;
			return String::GetVoid();
		}
		int r = 0;
		while(data.IsVoid()) {
			if(++r >= retries)
				return String::GetVoid();
			data = Execute0();
		}
		if(!IsRedirect())
			return data;
		if(++nredir > max_redirect) {
			error = NFormat("Maximum number of redirections exceeded: %d", max_redirect);
			return String::GetVoid();
		}
		URL(GetRedirectURL());
	}
}

END_UPP_NAMESPACE
