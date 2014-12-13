//
// ptrade.cpp
// Project Spitfire
//
// Copyright (c) 2014 Daizee (rensiadz at gmail dot com)
//
// This file is part of Spitfire.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "ptrade.h"
#include "../Server.h"
#include "../Client.h"
#include "../City.h"


ptrade::ptrade(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

ptrade::~ptrade()
{

}

void ptrade::process()
{
	if ((command == "searchTrades"))
	{
		int restype = data["resType"];

		if (restype < 0 || restype > 3)
		{
			gserver->SendObject(client, gserver->CreateError("trade.searchTrades", -99, "Not a valid resource type."));
			return;
		}


		obj2["cmd"] = "trade.searchTrades";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		gserver->SendObject(client, obj2);
		return;
	}
}



