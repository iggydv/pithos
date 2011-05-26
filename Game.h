//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef GAME_H_
#define GAME_H_

#include <omnetpp.h>

#include "Message_m.h"

class Game : public cSimpleModule
{
	private:
		simtime_t writeTime_av;
		simtime_t wait_time;
		simtime_t join_time;
		simtime_t generationTime;
		double objectSize_av;
		cMessage *event;
	public:
		Game();
		virtual ~Game();
	protected:
		virtual void initialize();
		virtual void handleMessage(cMessage *msg);
		void sendRequest();
};

Define_Module(Game);

#endif /* GAME_H_ */
