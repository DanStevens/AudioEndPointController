#pragma once
#ifndef _GENERALEXCEPTION_
#define _GENERALEXCEPTION_

#include <string>
#include "windows.h"

using namespace std;

namespace AudioDeviceApi
{

	class Exception
	{

	public:
		Exception(char* msg, char* file, int line)
			: msg(msg),file(file), line(line)
		{}
		Exception(char* msg)
			: msg(msg), file(""), line(-1)
		{}
		Exception() :
			msg("An Exception occurred"),
			file(""),
			line(-1)
		{}
		string GetMessage()
		{
			return msg;
		}
		string GetFile()
		{
			return file;
		}
		int GetLine()
		{
			return line;
		}

	protected:
		string msg;
		string file;
		int line;

	};

}

#endif