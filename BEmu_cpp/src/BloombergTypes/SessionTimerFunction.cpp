//------------------------------------------------------------------------------------------------
// <copyright project="BEmu_cpp" file="src/BloombergTypes/SessionTimerFunction.cpp" company="Jordan Robinson">
//     Copyright (c) 2013 Jordan Robinson. All rights reserved.
//
//     The use of this software is governed by the Microsoft Public License
//     which is included with this distribution.
// </copyright>
//------------------------------------------------------------------------------------------------

#include "BloombergTypes/SessionTimerFunction.h"
#include "BloombergTypes/Session.h"
#include "BloombergTypes/EventHandler.h"
#include "Types/BEmuTimer.h"

#include "Types/RandomDataGenerator.h"
#include "MarketDataRequest/EventMarket.h"

namespace BEmu
{
	SessionTimerFunction::SessionTimerFunction(Session * session, EventHandler * eventHandler)
	{
		this->_session = session;
		this->_eventHandler = eventHandler;
	}

	void SessionTimerFunction::Run(BEmuTimer * timer) //this runs on a separate thread, started in Types/BEmuTimer.cpp
	{
		int conflationIntervalInMilleseconds;
		bool hasConflationInterval = false;

		SubscriptionList subs = this->_session->getSubscriptions();
		SubscriptionList subsToUse;
		std::vector<Subscription> * list = subs.list();

		for(std::vector<Subscription>::const_iterator iter = list->begin(); iter != list->end(); ++iter)
		{
			Subscription current = *iter;
			if(RandomDataGenerator::ShouldIncludeQuote())
			{
				subsToUse.add(current);
			}

			if(current.hasConflationInterval())
			{
				conflationIntervalInMilleseconds = current.conflationInterval() * 1000;
			}
		}

		MarketDataRequest::EventMarket * evt = new MarketDataRequest::EventMarket(Event::SUBSCRIPTION_DATA, 0, subsToUse);
		this->_eventHandler->processEvent(evt, this->_session);

		if(hasConflationInterval)
			timer->reset(conflationIntervalInMilleseconds);
		else
		{
			boost::posix_time::ptime::time_duration_type duration(RandomDataGenerator::TimeBetweenMarketDataEvents());			
			int milliseconds = (int)duration.total_milliseconds();
			timer->reset(milliseconds);
		}
	}
}