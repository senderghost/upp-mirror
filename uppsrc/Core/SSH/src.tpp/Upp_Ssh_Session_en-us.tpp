topic "Session";
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[l288;i1121;b17;O9;~~~.1408;2 $$7,0#10431211400427159095818037425705:param]
[i448;b42;O9;2 $$8,8#61672508125594000341940100500538:tparam]
[b42;2 $$9,9#13035079074754324216151401829390:normal]
[2 $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@(113.42.0) [s0;%% [*@7;4 SshSession]]}}&]
[s3; &]
[s1;:Upp`:`:SshSession`:`:class: [@(0.0.255)3 class][3 _][*3 SshSession][3 _:_][@(0.0.255)3 publ
ic][3 _][*@3;3 Ssh]&]
[s0;#l288;%% This class encapsulates a secure shell (version 2) client 
session. It is responsible for establishing, authenticating, 
and managing a cryptographically secured shell session on an 
SSH2 compliant server. SshSession class is derived from Ssh base 
class, and has pick semantics.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:SshSession`:`:Timeout`(int`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&
]_[* Timeout]([@(0.0.255) int]_[*@3 ms])&]
[s2;%% Sets timeout value in miliseconds. Setting the timeout value 
to 0 puts the SshSession object into non`-blocking mode (same 
as NonBlocking(true)). Returns `*this for methods chaining. Note 
that ssh subsystems and channels inherit their default timeout 
values from their session.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:NonBlocking`(bool`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&
]_[* NonBlocking]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Activates or deactivates non`-blocking mode. Default is blocking 
mode. Returns `*this for methods chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:WaitStep`(int`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&
]_[* WaitStep]([@(0.0.255) int]_[*@3 ms])&]
[s2;%% Sets the periodicity of calling [^topic`:`/`/SSH`/src`/Upp`_Ssh`_Session`_en`-us`#Upp`:`:SshSession`:`:WhenWait^ W
henWait] in millisecond between calls. Default value is 10ms 
(100hz). Returns `*this for method chaining. Note that ssh subsystems 
and channels inherit their default waitstep values from their 
session.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:Keys`(const Upp`:`:String`&`,const Upp`:`:String`&`,const Upp`:`:String`&`,bool`): [_^Upp`:`:SshSession^ S
shSession][@(0.0.255) `&]_[* Keys]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 prikey], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 pubkey], 
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 phrase]_`=_Null, 
[@(0.0.255) bool]_[*@3 fromfile]_`=_[@(0.0.255) true])&]
[s2;%% Sets the asymmetric encryption keys to be used to authenticate 
the session. [%-*@3 phrase] can be used to decipher the private 
key. If phrase is not specified, supplied password will be used 
instead. Returns `*this for methods chaining. Note that when 
the [%-*@3 fromfile] parameter is true, [%-*@3 prikey] and [%-*@3 pubkey] 
strings will be treated as file paths to the respective key files. 
This is the default behaviour. Otherwise they will be treated 
as memory buffers containing the actual keys.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:Method`(int`,Upp`:`:Value`): [_^Upp`:`:SshSession^ SshSession
][@(0.0.255) `&]_[* Method]([@(0.0.255) int]_[*@3 type], [_^Upp`:`:Value^ Value]_[*@3 method])
&]
[s2;%% Sets preferred transport method for the method [%-*@3 type]. 
Transport [%-*@3 method] .parameter may contain a single method, 
or a list of methods, with the most preferred method listed as 
first and the least preferred as last. Returns `*this for methods 
chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:Methods`(Upp`:`:ValueMap`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&
]_[* Methods]([_^Upp`:`:ValueMap^ ValueMap]_[*@3 methods])&]
[s2;%% Sets transport [%-*@3 methods]. Methods list should contain 
key`-value pairs, where keys represent possible method types, 
and values represent a single or multiple transport method(s), 
with the most preferred listed as first and the least preferred 
as last. Returns `*this for methods chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:PasswordAuth`(`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&
]_[* PasswordAuth]()&]
[s2;%% Sets the authentication method to basic password authentication. 
Returns `*this for methods chaining.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:PublicKeyAuth`(`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&
]_[* PublicKeyAuth]()&]
[s2;%% Sets the authentication method to public key authentication. 
Returns `*this for methods chaining.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:HostBasedAuth`(`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&
]_[* HostBasedAuth]()&]
[s2;%% Sets the authentication method to host based authentication. 
Returns `*this for methods chaining.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:KeyboardAuth`(`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&
]_[* KeyboardAuth]()&]
[s2;%% Sets the authentication method to keyboard`-interactive (challenge/response) 
authentication. Returns `*this for methods chaining.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:AgentAuth`(`): [_^Upp`:`:SshSession^ SshSession][@(0.0.255) `&]_
[* AgentAuth]()&]
[s2;%% Enables using an ssh`-agent for authentication. Returns this 
for methods chaining. This method requires public and private 
keys to be present.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:GetHandle`(`): [_^LIBSSH2`_SESSION^ LIBSSH2`_SESSION][@(0.0.255) `*
]_[* GetHandle]()&]
[s2; Returns the libssh2 session handle on success, NULL on failure.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:GetBanner`(`)const: [_^Upp`:`:String^ String]_[* GetBanner]()_[@(0.0.255) c
onst]&]
[s2;%% Returns the servers banner if available.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:GetFingerprint`(`)const: [_^Upp`:`:String^ String]_[* GetFinger
print]()_[@(0.0.255) const]&]
[s2;%% Returns the computed digest of the server`'s host key (in 
raw bytes) on success, or an empty String on failure.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:GetSocket`(`): [_^Upp`:`:TcpSocket^ TcpSocket][@(0.0.255) `&]_[* G
etSocket]()&]
[s2;%% Returns a reference to the session socket.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:GetMethods`(`): [_^Upp`:`:ValueMap^ ValueMap]_[* GetMethods]()&]
[s0;l288;%% Returns a list of supported transport methods on success, 
and an empty list on failure. Supported method types are represented 
by `"keys`", and available methods, which can be a single string 
or a list of strings, are represented by `"values`". Note that 
all values are in lowercase letters.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:GetAuthMethods`(`): [_^Upp`:`:Vector^ Vector]<[_^Upp`:`:String^ S
tring]>_[* GetAuthMethods]()&]
[s0;l288;%% Returns a list of supported user authentication methods. 
Returns an empty vector on failure. The following four authentication 
methods are currently supported: `"password`", `"publickey`", 
`"hostbased`", `"keyboard`-interactive`".&]
[s3;^Upp`:`:SFtp^ &]
[s4;^Upp`:`:SFtp^ &]
[s5;:Upp`:`:SshSession`:`:CreateSFtp`(`): [_^Upp`:`:SFtp^ SFtp]_[* CreateSFtp]()&]
[s2;%% Creates an sftp channel instance.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:CreateChannel`(`): [_^Upp`:`:SshChannel^ SshChannel]_[* CreateC
hannel]()&]
[s2;%% Creates a generic channel instance.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:CreateExec`(`): [_^Upp`:`:SshExec^ SshExec]_[* CreateExec]()&]
[s2;%% Creates an exec channel instance.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:CreateScp`(`): [_^Upp`:`:Scp^ Scp]_[* CreateScp]()&]
[s2;%% Creates an scp channel instance.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:CreateTcpTunnel`(`): [_^Upp`:`:SshTunnel^ SshTunnel]_[* CreateT
cpTunnel]()&]
[s2;%% Creates a tcp`-ip and port forwarding channel instance.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:CreateShell`(`): [_^Upp`:`:SshShell^ SshShell]_[* CreateShell](
)&]
[s2;%% Creates a remote shell channel instance.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:Connect`(const Upp`:`:String`&`): [@(0.0.255) bool]_[* Connect](
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 url])&]
[s2;%% Connects to a SSH2 server specified by the [%-*@3 url]. Returns 
true on success. &]
[s2;%% Syntax of the URL is as follows: [C@3 `[ssh`|scp`|sftp`|exec`]://`[user:password`@`]
host`[:port`]][C .]&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:Connect`(const Upp`:`:String`&`,int`,const Upp`:`:String`&`,const Upp`:`:String`&`): [@(0.0.255) b
ool]_[* Connect]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 host], 
[@(0.0.255) int]_[*@3 port], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 u
ser], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 password])&]
[s2;%% Connects to a SSH2 server specified at [%-*@3 host] and [%-*@3 port]. 
Returns true on success.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:Disconnect`(`): [@(0.0.255) void]_[* Disconnect]()&]
[s2;%% Disconnects from the SSH2 server.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:WhenWait: [_^Upp`:`:Event^ Event]<>_[* WhenWait]&]
[s2;%% If this event is defined, it is invoked periodically while 
the ssh client performs any socket operations, with the frequency 
of 100Hz. This is intended to give user feedback in interactive 
applications. Note that all ssh channels privately has a similar 
callback defined and redirect their calls to their session`'s 
WhenWait callback.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:WhenConfig: [_^Upp`:`:Event^ Event]<>_[* WhenConfig]&]
[s2;%% This event is emitted at the beginning of the protocol handshake 
phase to allow user to query or set the transport methods.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:WhenAuth: [_^Upp`:`:Event^ Event]<>_[* WhenAuth]&]
[s0;l288;%% This event is emitted at the beginning of the session 
authentication phase to allow user to query or set the authentication 
method(s)&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:WhenVerify: [_^Upp`:`:Gate^ Gate]<>_[* WhenVerify]&]
[s0;l288;%% This gate is invoked after a successful protocol handshake 
to allow user to verify the host against a list of known (trusted) 
hosts. Returning false halts the connection process.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:WhenProxy: [_^Upp`:`:Gate^ Gate]<>_[* WhenProxy]&]
[s2;%% If this gate is defined it will be invoked right before the 
initial socket connection. It is meant to allow utilizing third 
party `-`"plug`-in`" network proxy adapters on`-demand. Returning 
false halts the connection process.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:WhenX11: [_^Upp`:`:Event^ Event]<SshX11Request[@(0.0.255) `*]>_
[* WhenX11]&]
[s6; POSIX only. Requires a running X server.&]
[s2;%% This event will forward the X11 connections to the SshShell 
instances that accept X11 requests. See [^topic`:`/`/SSH`/src`/Upp`_Ssh`_Channels`_en`-us`#Upp`:`:SshShell`:`:class^ S
shShell].&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:WhenKeyboard: [_^Upp`:`:Function^ Function]<[_^Upp`:`:String^ S
tring]([_^Upp`:`:String^ String], [_^Upp`:`:String^ String], [_^Upp`:`:String^ String])
>_[* WhenKeyboard]&]
[s0;l288;%% This callback is required by the keyboard`-interactive 
(challenge/response) authentication method. The host will issue 
one or more challenges and require a response for each challenge. 
Therefore this callback, responsible for obtaining user responses, 
may be invoked more than once, and has 3 additional parameters 
(as strings): [*@3 title], [*@3 instructions], and [*@3 prompt]. Title 
and instructions may be empty strings. Returned responses will 
be evaluated by the host. Returning an empty string will halt 
the connection attempt.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:SshSession`:`:SshSession`(`): [* SshSession]()&]
[s2;%% Default constructor.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshSession`:`:`~SshSession`(`): [@(0.0.255) `~][* SshSession]()&]
[s2;%% Default destructor. Automatically closes the session.&]
[s3; &]
[s0;%% &]
[s0; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 Ssh Transport Methods]]}}&]
[s2; &]
[s2; Below is a list of transport method types (keys) and methods 
(values) supported by SSH package.&]
[s2; &]
[s3; &]
[ {{4133:5867l/26r/26t/14b/14@1-1 [s0;=%% [* Key Exchange]]
::l/25r/25t/15b/15@2 [s0; ]
::l/26r/26t/14b/14@(178) [s0;=%% Key]
:: [s0;=%% Value(s)]
::l/25r/25t/15b/15@2|4 [s0;=%% [*C SshSession`::METHOD`_EXCHANGE]]
:: [s0;%% [C diffie`-hellman`-group1`-sha1]]
:: [s0; ]
:: [s0;%% [C diffie`-hellman`-group14`-sha1]]
:: [s0; ]
:: [s0;%% [C diffie`-hellman`-group14`-sha1]]
:: [s0; ]
:: [s0;%% [C diffie`-hellman`-group`-exchange`-sha1]]
:: [s0; ]
:: [s0;%% [C diffie`-hellman`-group`-exchange`-sha256]]
::l/26r/26t/14b/14@1-1 [s0;=%% [* HostKey]]
::l/25r/25t/15b/15@2 [s0; ]
::l/26r/26t/14b/14@(178) [s0;=%% Key]
:: [s0;=%% Value(s)]
::l/25r/25t/15b/15@2|1 [s0;=%% [*C SshSession`::METHOD`_HOSTKEY]]
:: [s0;%% [C ssh`-dss]]
:: [s0; ]
:: [s0;%% [C ssh`-rsa]]
::l/26r/26t/14b/14@1-1 [s0;=%% [* Cipher (Client and/or Server)]]
::l/25r/25t/15b/15@2 [s0; ]
::l/26r/26t/14b/14@(178) [s0;=%% Key]
:: [s0;=%% Value(s)]
::l/25r/25t/15b/15@2|11 [s0;=%% [*C SshSession`::METHOD`_CENCRYPTION]&]
[s0;=%% [*C SshSession`::METHOD`_SENCRYPTION]]
:: [s0;%% [C aes256`-ctr ]]
:: [s0; ]
:: [s0;%% [C aes192`-ctr]]
:: [s0; ]
:: [s0;%% [C aes192`-ctr]]
:: [s0; ]
:: [s0;%% [C aes256`-cbc (rijndael`-cbc`@lysator.liu.se)]]
:: [s0; ]
:: [s0;%% [C aes192`-cbc]]
:: [s0; ]
:: [s0;%% [C aes128`-cbc]]
:: [s0; ]
:: [s0;%% [C 3des`-cbc]]
:: [s0; ]
:: [s0;%% [C 3des`-cbc]]
:: [s0; ]
:: [s0;%% [C cast128`-cbc]]
:: [s0; ]
:: [s0;%% [C arcfour]]
:: [s0; ]
:: [s0;%% [C arcfour128]]
:: [s0; ]
:: [s0;%% [C none]]
::l/26r/26t/14b/14@1-1 [s0;=%% [* MAC Hashing (Client and/or Server)]]
::l/25r/25t/15b/15@2 [s0; ]
::l/26r/26t/14b/14@(178) [s0;=%% Key]
:: [s0;=%% Value(s)]
::l/25r/25t/15b/15@2|7 [s0;=%% [*C SshSession`::METHOD`_CMAC]&]
[s0;=%% [*C SshSession`::METHOD`_SMAC]]
:: [s0;%% [C hmac`-sha2`-256]]
:: [s0; ]
:: [s0;%% [C hmac`-sha2`-512]]
:: [s0; ]
:: [s0;%% [C hmac`-sha1]]
:: [s0; ]
:: [s0;%% [C hmac`-sha1`-96]]
:: [s0; ]
:: [s0;%% [C hmac`-md5]]
:: [s0; ]
:: [s0;%% [C hmac`-md5`-96]]
:: [s0; ]
:: [s0;%% [C hmac`-ripemd160 (hmac`-ripemd160`@openssh.com)]]
:: [s0; ]
:: [s0;%% [C none]]}}&]
[s0;%% ]]