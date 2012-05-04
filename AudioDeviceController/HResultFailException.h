#pragma once
#ifndef _HRESULTFAILEXCEPTION_
#define _HRESULTFAILEXCEPTION_

#include <string>
#include "Exception.h"
#include "windows.h"

using namespace std;

namespace AudioDeviceApi
{

	class HResultFailException : public Exception
	{

	public:
		HResultFailException(HRESULT hr, char* msg, char* file, int line)
			: hr(hr), Exception(msg, file, line)
		{}
		HResultFailException(HRESULT hr, char* msg)
			: hr(hr), Exception(msg)
		{}
		HResultFailException(HRESULT hr)
			: hr(hr),
			Exception("A function returned a failed HRESULT")
		{}
		HRESULT GetHResult()
		{
			return hr;
		}

	protected:
		HRESULT hr;
	
	};

}

#endif