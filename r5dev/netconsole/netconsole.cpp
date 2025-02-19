//=====================================================================================//
// 
// Purpose: Lightweight netconsole client.
// 
//=====================================================================================//

#include "core/stdafx.h"
#include "core/termutil.h"
#include "tier1/NetAdr2.h"
#include "tier2/socketcreator.h"
#include "protoc/sv_rcon.pb.h"
#include "protoc/cl_rcon.pb.h"
#include "public/utility/utility.h"
#include "engine/net.h"
#include "netconsole/netconsole.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNetCon::CNetCon(void)
	: m_bInitialized(false)
	, m_bNoColor(false)
	, m_bQuitApplication(false)
	, m_abPromptConnect(true)
	, m_abConnEstablished(false)
	, m_NetAdr2("localhost", "37015")
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNetCon::~CNetCon(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: WSA and NETCON systems init
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CNetCon::Init(void)
{
	WSAData wsaData;
	const int nError = ::WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nError != 0)
	{
		std::cerr << "Failed to start Winsock via WSAStartup: (" << NET_ErrorString(WSAGetLastError()) << ")" << std::endl;
		return false;
	}

	this->TermSetup();

	static std::thread frame([this]()
		{
			for (;;)
			{
				this->RunFrame();
			}
		});
	frame.detach();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: WSA and NETCON systems shutdown
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CNetCon::Shutdown(void)
{
	m_Socket.CloseAllAcceptedSockets();
	m_abConnEstablished = false;

	const int nError = ::WSACleanup();
	if (nError != 0)
	{
		std::cerr << "Failed to stop Winsock via WSACleanup: (" << NET_ErrorString(WSAGetLastError()) << ")" << std::endl;
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: terminal setup
//-----------------------------------------------------------------------------
void CNetCon::TermSetup(void)
{
	DWORD dwMode = NULL;
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	if (!strstr(GetCommandLineA(), "-nocolor"))
	{
		GetConsoleMode(hOutput, &dwMode);
		dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

		if (!SetConsoleMode(hOutput, dwMode)) // Some editions of Windows have 'VirtualTerminalLevel' disabled by default.
		{
			// Warn the user if 'VirtualTerminalLevel' could not be set on users environment.
			MessageBoxA(NULL, "Failed to set console mode 'VirtualTerminalLevel'.\n"
				"Restart the net console with the '-nocolor'\n"
				"parameter if output logging appears distorted.", "SDK Warning",
				MB_ICONEXCLAMATION | MB_OK);
		}
		AnsiColors_Init();
	}
	else
	{
		m_bNoColor = true;
	}

	CONSOLE_SCREEN_BUFFER_INFOEX sbInfoEx{};
	sbInfoEx.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

	GetConsoleScreenBufferInfoEx(hOutput, &sbInfoEx);
	sbInfoEx.ColorTable[0] = 0x0000;
	SetConsoleScreenBufferInfoEx(hOutput, &sbInfoEx);
}

//-----------------------------------------------------------------------------
// Purpose: gets input IP and port for initialization
//-----------------------------------------------------------------------------
void CNetCon::UserInput(void)
{
	std::string svInput;

	if (std::getline(std::cin, svInput))
	{
		if (svInput.compare("nquit") == 0)
		{
			m_bQuitApplication = true;
			return;
		}

		std::lock_guard<std::mutex> l(m_Mutex);
		if (m_abConnEstablished)
		{
			if (svInput.compare("disconnect") == 0)
			{
				this->Disconnect();
				return;
			}

			const std::vector<std::string> vSubStrings = StringSplit(svInput, ' ', 2);
			if (vSubStrings.size() > 1)
			{
				if (vSubStrings[0].compare("PASS") == 0) // Auth with RCON server.
				{
					std::string svSerialized = this->Serialize(vSubStrings[1], "", cl_rcon::request_t::SERVERDATA_REQUEST_AUTH);
					this->Send(svSerialized);
				}
				else if (vSubStrings[0].compare("SET") == 0) // Set value query.
				{
					if (vSubStrings.size() > 2)
					{
						std::string svSerialized = this->Serialize(vSubStrings[1], vSubStrings[2], cl_rcon::request_t::SERVERDATA_REQUEST_SETVALUE);
						this->Send(svSerialized);
					}
				}
				else // Execute command query.
				{
					std::string svSerialized = this->Serialize(svInput, "", cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND);
					this->Send(svSerialized);
				}
			}
			else if (!svInput.empty()) // Single arg command query.
			{
				std::string svSerialized = this->Serialize(svInput, "", cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND);
				this->Send(svSerialized);
			}
		}
		else // Setup connection from input.
		{
			if (!svInput.empty())
			{
				const string::size_type nPos = svInput.find(' ');
				if (nPos > 0
					&& nPos < svInput.size()
					&& nPos != svInput.size())
				{
					std::string svInPort = svInput.substr(nPos + 1);
					std::string svInAdr = svInput.erase(svInput.find(' '));

					if (!this->Connect(svInAdr, svInPort))
					{
						m_abPromptConnect = true;
						return;
					}
				}
			}
			else // Initialize as [127.0.0.1]:37015.
			{
				if (!this->Connect("", ""))
				{
					m_abPromptConnect = true;
					return;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: client's main processing loop
//-----------------------------------------------------------------------------
void CNetCon::RunFrame(void)
{
	if (m_abConnEstablished)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		std::lock_guard<std::mutex> l(m_Mutex);

		this->Recv();
	}
	else if (m_abPromptConnect)
	{
		std::cout << "Enter <IP> <PORT>: ";
		m_abPromptConnect = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: checks if application should be terminated
// Output : true for termination, false otherwise
//-----------------------------------------------------------------------------
bool CNetCon::ShouldQuit(void) const
{
	return this->m_bQuitApplication;
}

//-----------------------------------------------------------------------------
// Purpose: connect to specified address and port
// Input  : *svInAdr - 
//			*svInPort - 
// Output : true if connection succeeds, false otherwise
//-----------------------------------------------------------------------------
bool CNetCon::Connect(const std::string& svInAdr, const std::string& svInPort)
{
	if (!svInAdr.empty() && !svInPort.empty())
	{
		// Default is [127.0.0.1]:37015
		m_NetAdr2.SetIPAndPort(svInAdr, svInPort);
	}

	if (m_Socket.ConnectSocket(m_NetAdr2, true) == SOCKET_ERROR)
	{
		std::cerr << "Failed to connect. Error: (SOCKET_ERROR). Verify IP and PORT." << std::endl;
		return false;
	}
	std::cout << "Connected to: " << m_NetAdr2.GetIPAndPort() << std::endl;

	m_abConnEstablished = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: disconnect from current session
//-----------------------------------------------------------------------------
void CNetCon::Disconnect(void)
{
	m_Socket.CloseAcceptedSocket(0);
	m_abPromptConnect = true;
	m_abConnEstablished = false;
}

//-----------------------------------------------------------------------------
// Purpose: send message
// Input  : *svMessage - 
//-----------------------------------------------------------------------------
void CNetCon::Send(const std::string& svMessage) const
{
	std::ostringstream ssSendBuf;
	const u_long nLen = htonl(svMessage.size());

	ssSendBuf.write(reinterpret_cast<const char*>(&nLen), sizeof(u_long));
	ssSendBuf.write(svMessage.data(), svMessage.size());

	const int nSendResult = ::send(m_Socket.GetAcceptedSocketData(0)->m_hSocket,
		ssSendBuf.str().data(), static_cast<int>(ssSendBuf.str().size()), MSG_NOSIGNAL);
	if (nSendResult == SOCKET_ERROR)
	{
		std::cout << "Failed to send message: (SOCKET_ERROR)" << std::endl;
	}
}

//-----------------------------------------------------------------------------
// Purpose: receive message
//-----------------------------------------------------------------------------
void CNetCon::Recv(void)
{
	static char szRecvBuf[1024];
	CConnectedNetConsoleData* pData = m_Socket.GetAcceptedSocketData(0);

	{//////////////////////////////////////////////
		const int nPendingLen = ::recv(pData->m_hSocket, szRecvBuf, sizeof(char), MSG_PEEK);
		if (nPendingLen == SOCKET_ERROR && m_Socket.IsSocketBlocking())
		{
			return;
		}
		if (nPendingLen <= 0 && m_abConnEstablished) // EOF or error.
		{
			this->Disconnect();
			std::cout << "Server closed connection" << std::endl;
			return;
		}
	}//////////////////////////////////////////////

	u_long nReadLen; // Find out how much we have to read.
	::ioctlsocket(pData->m_hSocket, FIONREAD, &nReadLen);

	while (nReadLen > 0)
	{
		const int nRecvLen = ::recv(pData->m_hSocket, szRecvBuf, MIN(sizeof(szRecvBuf), nReadLen), MSG_NOSIGNAL);
		if (nRecvLen == 0 && m_abConnEstablished) // Socket was closed.
		{
			this->Disconnect();
			std::cout << "Server closed connection" << std::endl;
			break;
		}
		if (nRecvLen < 0 && !m_Socket.IsSocketBlocking())
		{
			std::cout << "RCON Cmd: recv error (" << NET_ErrorString(WSAGetLastError()) << ")" << std::endl;
			break;
		}

		nReadLen -= nRecvLen; // Process what we've got.
		this->ProcessBuffer(szRecvBuf, nRecvLen, pData);
	}
}

//-----------------------------------------------------------------------------
// Purpose: parses input response buffer using length-prefix framing
// Input  : *pRecvBuf - 
//			nRecvLen - 
//			*pData - 
//-----------------------------------------------------------------------------
void CNetCon::ProcessBuffer(const char* pRecvBuf, int nRecvLen, CConnectedNetConsoleData* pData)
{
	while (nRecvLen > 0)
	{
		if (pData->m_nPayloadLen)
		{
			if (pData->m_nPayloadRead < pData->m_nPayloadLen)
			{
				pData->m_RecvBuffer[pData->m_nPayloadRead++] = *pRecvBuf;

				pRecvBuf++;
				nRecvLen--;
			}
			if (pData->m_nPayloadRead == pData->m_nPayloadLen)
			{
				this->ProcessMessage(this->Deserialize(std::string(
					reinterpret_cast<char*>(pData->m_RecvBuffer.data()), pData->m_nPayloadLen)));

				pData->m_nPayloadLen = 0;
				pData->m_nPayloadRead = 0;
			}
		}
		else if (pData->m_nPayloadRead < sizeof(int)) // Read size field.
		{
			pData->m_RecvBuffer[pData->m_nPayloadRead++] = *pRecvBuf;

			pRecvBuf++;
			nRecvLen--;
		}
		else // Build prefix.
		{
			pData->m_nPayloadLen = ntohl(*reinterpret_cast<u_long*>(&pData->m_RecvBuffer[0]));
			pData->m_nPayloadRead = 0;

			if (pData->m_nPayloadLen < 0 ||
				pData->m_nPayloadLen > pData->m_RecvBuffer.max_size())
			{
				std::cout << "RCON Cmd: sync error (" << pData->m_nPayloadLen << ")" << std::endl;
				this->Disconnect(); // Out of sync (irrecoverable).

				break;
			}
			else
			{
				pData->m_RecvBuffer.resize(pData->m_nPayloadLen);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: processes received message
// Input  : *sv_response - 
//-----------------------------------------------------------------------------
void CNetCon::ProcessMessage(const sv_rcon::response& sv_response) const
{
	switch (sv_response.responsetype())
	{
	case sv_rcon::response_t::SERVERDATA_RESPONSE_AUTH:
	{
		if (!sv_response.responseval().empty())
		{
			const long i = strtol(sv_response.responseval().c_str(), NULL, NULL);
			if (!i) // sv_rcon_sendlogs is not set.
			{
				string svLogQuery = this->Serialize("", "", cl_rcon::request_t::SERVERDATA_REQUEST_SEND_CONSOLE_LOG);
				this->Send(svLogQuery);
			}
		}
		[[fallthrough]];
	}
	case sv_rcon::response_t::SERVERDATA_RESPONSE_CONSOLE_LOG:
	{
		std::string svOut = sv_response.responsebuf();
		if (m_bNoColor)
		{
			svOut = std::regex_replace(svOut, std::regex("\\\033\\[.*?m"), "");
		}
		else
		{
			svOut.append(g_svReset);
		}
		std::cout << svOut;
		break;
	}
	default:
	{
		break;
	}
	}
}

//-----------------------------------------------------------------------------
// Purpose: serializes input
// Input  : *svReqBuf - 
//			*svReqVal - 
//			request_t - 
// Output : serialized results as string
//-----------------------------------------------------------------------------
std::string CNetCon::Serialize(const std::string& svReqBuf, const std::string& svReqVal, const cl_rcon::request_t request_t) const
{
	cl_rcon::request cl_request;

	cl_request.set_requestid(-1);
	cl_request.set_requesttype(request_t);

	switch (request_t)
	{
	case cl_rcon::request_t::SERVERDATA_REQUEST_SETVALUE:
	case cl_rcon::request_t::SERVERDATA_REQUEST_AUTH:
	{
		cl_request.set_requestbuf(svReqBuf);
		cl_request.set_requestval(svReqVal);
		break;
	}
	case cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND:
	{
		cl_request.set_requestbuf(svReqBuf);
		break;
	}
	}
	return cl_request.SerializeAsString();
}

//-----------------------------------------------------------------------------
// Purpose: de-serializes input
// Input  : *svBuf - 
// Output : de-serialized object
//-----------------------------------------------------------------------------
sv_rcon::response CNetCon::Deserialize(const std::string& svBuf) const
{
	sv_rcon::response sv_response;
	sv_response.ParseFromArray(svBuf.data(), static_cast<int>(svBuf.size()));

	return sv_response;
}

//-----------------------------------------------------------------------------
// Purpose: entrypoint
// Input  : argc - 
//			*argv - 
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	std::cout << "R5Reloaded TCP net console [Version " << NETCON_VERSION << "]" << std::endl;

	if (!NetConsole()->Init())
	{
		return EXIT_FAILURE;
	}

	if (argc >= 3) // Get IP and Port from command line.
	{
		if (!NetConsole()->Connect(argv[1], argv[2]))
		{
			return EXIT_FAILURE;
		}
	}

	while (!NetConsole()->ShouldQuit())
	{
		NetConsole()->UserInput();
	}

	if (!NetConsole()->Shutdown())
	{
		return EXIT_FAILURE;
	}

	return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------
// singleton
//-----------------------------------------------------------------------------
CNetCon g_NetCon;
inline CNetCon* NetConsole()
{
	return &g_NetCon;
}