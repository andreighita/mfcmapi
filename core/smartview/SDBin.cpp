#include <core/stdafx.h>
#include <core/smartview/SDBin.h>
#include <core/mapi/extraPropTags.h>
#include <core/interpret/flags.h>
#include <core/interpret/sid.h>
#include <core/mapi/mapiFunctions.h>

namespace smartview
{
	SDBin::~SDBin()
	{
		if (m_lpMAPIProp) m_lpMAPIProp->Release();
	}

	void SDBin::Init(_In_opt_ LPMAPIPROP lpMAPIProp, bool bFB)
	{
		if (m_lpMAPIProp) m_lpMAPIProp->Release();
		m_lpMAPIProp = lpMAPIProp;
		if (m_lpMAPIProp) m_lpMAPIProp->AddRef();
		m_bFB = bFB;
	}

	void SDBin::Parse() { m_SDbin = m_Parser.GetBYTES(m_Parser.RemainingBytes()); }

	void SDBin::ParseBlocks()
	{
		auto acetype = sid::acetypeMessage;
		switch (mapi::GetMAPIObjectType(m_lpMAPIProp))
		{
		case MAPI_STORE:
		case MAPI_ADDRBOOK:
		case MAPI_FOLDER:
		case MAPI_ABCONT:
			acetype = sid::acetypeContainer;
			break;
		}

		if (m_bFB) acetype = sid::acetypeFreeBusy;

		const auto sd = SDToString(m_SDbin, acetype);

		setRoot(L"Security Descriptor:\r\n");
		addHeader(L"Security Info: ");
		addBlock(m_SDbin, sd.info);

		terminateBlock();
		const auto sdVersion = SECURITY_DESCRIPTOR_VERSION(m_SDbin.data());
		auto szFlags = flags::InterpretFlags(flagSecurityVersion, sdVersion);
		addBlock(m_SDbin, L"Security Version: 0x%1!04X! = %2!ws!\r\n", sdVersion, szFlags.c_str());
		addHeader(L"Descriptor:\r\n");
		addBlock(m_SDbin, sd.dacl);
	}
} // namespace smartview