//
// ptroop.cpp
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

#include "ptroop.h"
#include "../Server.h"
#include "../Client.h"
#include "../City.h"
#include "../Hero.h"


ptroop::ptroop(Server * server, request & req, amf3object & obj)
	: packet(server, req, obj)
{

}

ptroop::~ptroop()
{

}

void ptroop::process()
{
	obj2["data"] = amf3object();
	amf3object & data2 = obj2["data"];

	if ((command == "getProduceQueue"))
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		uint32_t castleid = data["castleId"];

		obj2["cmd"] = "troop.getProduceQueue";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		client->lists.lock();
		amf3array producequeue = amf3array();
		for (int i = 0; i < 35; ++i)
		{
			stBuilding * building = city->GetBuilding(i);
			if ((building) && (building->type == B_BARRACKS))
			{
				stTroopQueue * train = city->GetBarracksQueue(i);

				if (train == 0)
				{
					gserver->consoleLogger->information(Poco::format("Crash! PlayerCity::stTroopQueue * train = city->GetBarracksQueue(%?d); - %?d", i, __LINE__));
					client->lists.unlock();
					return;
				}

				amf3object producequeueobj = amf3object();
				amf3array producequeueinner = amf3array();
				amf3object producequeueinnerobj = amf3object();

				std::list<stTroopTrain>::iterator iter;

				if (train->queue.size() > 0)
				{
					for (iter = train->queue.begin(); iter != train->queue.end(); ++iter)
					{
						producequeueinnerobj["num"] = iter->count;
						producequeueinnerobj["queueId"] = iter->queueid;
						producequeueinnerobj["endTime"] = (iter->endtime > 0) ? (iter->endtime - client->m_lag) : (iter->endtime);// HACK: attempt to fix "lag" issues
						producequeueinnerobj["type"] = iter->troopid;
						producequeueinnerobj["costTime"] = iter->costtime / 1000;
						producequeueinner.Add(producequeueinnerobj);
					}
				}
				producequeueobj["allProduceQueue"] = producequeueinner;
				producequeueobj["positionId"] = building->id;
				producequeue.Add(producequeueobj);
			}
		}

		client->lists.unlock();

		data2["allProduceQueue"] = producequeue;

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "getTroopProduceList"))
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		uint32_t castleid = data["castleId"];

		obj2["cmd"] = "troop.getTroopProduceList";
		amf3array trooplist = amf3array();


		for (int i = 0; i < 20; ++i)
		{
			if (gserver->m_troopconfig[i].inside != 1)
				continue;
			if (gserver->m_troopconfig[i].time > 0)
			{
				amf3object parent;
				amf3object conditionbean;

				double costtime = gserver->m_troopconfig[i].time;
				double mayorinf = 1;
				if (city->m_mayor)
					mayorinf = pow(0.995, city->m_mayor->GetPower());

				switch (i)
				{
					case TR_CATAPULT:
					case TR_RAM:
					case TR_TRANSPORTER:
					case TR_BALLISTA:
						costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_METALCASTING)));
						break;
					default:
						costtime = (costtime)* (mayorinf)* (pow(0.9, client->GetResearchLevel(T_MILITARYSCIENCE)));
						break;
				}

				conditionbean["time"] = floor(costtime);
				conditionbean["destructTime"] = 0;
				conditionbean["wood"] = gserver->m_troopconfig[i].wood;
				conditionbean["food"] = gserver->m_troopconfig[i].food;
				conditionbean["iron"] = gserver->m_troopconfig[i].iron;
				conditionbean["gold"] = gserver->m_troopconfig[i].gold;
				conditionbean["stone"] = gserver->m_troopconfig[i].stone;

				amf3array buildings = amf3array();
				amf3array items = amf3array();
				amf3array techs = amf3array();

				for_each(gserver->m_troopconfig[i].buildings.begin(), gserver->m_troopconfig[i].buildings.end(), [&](stPrereq & req)
				{
					if (req.id > 0)
					{
						amf3object ta = amf3object();
						ta["level"] = req.level;
						int temp = city->GetBuildingLevel(req.id);
						ta["curLevel"] = temp;
						ta["successFlag"] = temp >= req.level ? true : false;
						ta["typeId"] = req.id;
						buildings.Add(ta);
					}
				});
				for_each(gserver->m_troopconfig[i].items.begin(), gserver->m_troopconfig[i].items.end(), [&](stPrereq & req)
				{
					if (req.id > 0)
					{
						amf3object ta = amf3object();
						int temp = client->m_items[req.id].count;
						ta["curNum"] = temp;
						ta["num"] = req.level;
						ta["successFlag"] = temp >= req.level ? true : false;
						ta["id"] = gserver->m_items[req.id].name;
						items.Add(ta);
					}
				});
				for_each(gserver->m_troopconfig[i].techs.begin(), gserver->m_troopconfig[i].techs.end(), [&](stPrereq & req)
				{
					if (req.id > 0)
					{
						amf3object ta = amf3object();
						ta["level"] = req.level;
						int temp = client->GetResearchLevel(req.id);
						ta["curLevel"] = temp;
						ta["successFlag"] = temp >= req.level ? true : false;
						ta["typeId"] = req.id;
						buildings.Add(ta);
					}
				});


				conditionbean["buildings"] = buildings;
				conditionbean["items"] = items;
				conditionbean["techs"] = techs;
				conditionbean["population"] = gserver->m_troopconfig[i].population;
				parent["conditionBean"] = conditionbean;
				parent["permition"] = false;
				parent["typeId"] = i;
				trooplist.Add(parent);
			}
		}


		data2["troopList"] = trooplist;
		data2["ok"] = 1;
		data2["packageId"] = 0.0f;

		gserver->SendObject(client, obj2);
		return;
	}
	if ((command == "produceTroop"))
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		uint32_t castleid = data["castleId"];


		obj2["cmd"] = "troop.produceTroop";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;

		int trooptype = data["troopType"];
		bool isshare = data["isShare"];
		bool toidle = data["toIdle"];
		int positionid = data["positionId"];
		int num = data["num"];


		stResources res;
		res.food = gserver->m_troopconfig[trooptype].food * num;
		res.wood = gserver->m_troopconfig[trooptype].wood * num;
		res.stone = gserver->m_troopconfig[trooptype].stone * num;
		res.iron = gserver->m_troopconfig[trooptype].iron * num;
		res.gold = gserver->m_troopconfig[trooptype].gold * num;


		if ((res.food > city->m_resources.food)
			|| (res.wood > city->m_resources.wood)
			|| (res.stone > city->m_resources.stone)
			|| (res.iron > city->m_resources.iron)
			|| (res.gold > city->m_resources.gold))
		{
			gserver->SendObject(client, gserver->CreateError("troop.produceTroop", -1, "Not enough resources."));
			return;
		}

		if (isshare || toidle)
		{
			gserver->SendObject(client, gserver->CreateError("troop.produceTroop", -99, "Not supported action."));
			return;
		}

		city->m_resources -= res;
		city->ResourceUpdate();

		LOCK(M_TIMEDLIST);
		if (city->AddToBarracksQueue(positionid, trooptype, num, isshare, toidle) == -1)
		{
			UNLOCK(M_TIMEDLIST);
			gserver->SendObject(client, gserver->CreateError("troop.produceTroop", -99, "Troops could not be trained."));
			return;
		}
		UNLOCK(M_TIMEDLIST);




		gserver->SendObject(client, obj2);

		client->SaveToDB();
		city->SaveToDB();

		return;
	}
	if ((command == "cancelTroopProduce"))
	{
		VERIFYCASTLEID();
		CHECKCASTLEID();

		uint32_t castleid = data["castleId"];
		int positionid = data["positionId"];
		int queueid = data["queueId"];


		obj2["cmd"] = "troop.cancelTroopProduce";
		data2["packageId"] = 0.0f;
		data2["ok"] = 1;


		client->lists.lock();
		stTroopQueue * tq = city->GetBarracksQueue(positionid);
		std::list<stTroopTrain>::iterator iter;

		for (iter = tq->queue.begin(); iter != tq->queue.end();)
		{
			if (iter->queueid == queueid)
			{
				if (iter->endtime > 0)
				{
					//in progress
					//refund 1/3 resources and set next queue to run
					stResources res;
					res.food = (gserver->m_troopconfig[iter->troopid].food * iter->count) / 3;
					res.wood = (gserver->m_troopconfig[iter->troopid].wood * iter->count) / 3;
					res.stone = (gserver->m_troopconfig[iter->troopid].stone * iter->count) / 3;
					res.iron = (gserver->m_troopconfig[iter->troopid].iron * iter->count) / 3;
					res.gold = (gserver->m_troopconfig[iter->troopid].gold * iter->count) / 3;

					city->m_resources += res;
					city->ResourceUpdate();
					tq->queue.erase(iter++);

					iter->endtime = unixtime() + iter->costtime;
				}
				else
				{
					//not in progress
					//refund all resources
					stResources res;
					res.food = gserver->m_troopconfig[iter->troopid].food * iter->count;
					res.wood = gserver->m_troopconfig[iter->troopid].wood * iter->count;
					res.stone = gserver->m_troopconfig[iter->troopid].stone * iter->count;
					res.iron = gserver->m_troopconfig[iter->troopid].iron * iter->count;
					res.gold = gserver->m_troopconfig[iter->troopid].gold * iter->count;

					city->m_resources += res;
					tq->queue.erase(iter++);
					city->ResourceUpdate();
				}

				gserver->SendObject(client, obj2);

				client->lists.unlock();

				client->SaveToDB();
				city->SaveToDB();

				return;
			}
			++iter;
		}
		client->lists.unlock();
	}
}



