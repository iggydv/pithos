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

#include "Storage.h"

Storage::Storage()
{
}

Storage::~Storage()
{
	/*int i;
	int length = storage.getLength();
	Message *storage_msg;

	for (i = 0 ; i < length ; i++)
	{
		assert(storage.front() != NULL);
		storage_msg = ((Message *)storage.pop());
		delete(storage_msg);
	}*/
}

void Storage::initialize()
{
	storage.setName("queue");
	take(&storage);

	//Initialise queue statistics collection
	qlenSignal = registerSignal("qlen");
	queueingTimeSignal = registerSignal("queueingTime");
	emit(qlenSignal, storage.length());
}

void Storage::handleMessage(cMessage *msg)
{
		Message *storage_msg = check_and_cast<Message *>(msg);

		EV << getName() << " " << getIndex() << " received write command of size " << storage_msg->getValue() << "\n";

		storage.insert(storage_msg);		//This cast is important, otherwise a segfault occurs when calling the cQueue destructor

		//test_o = (go *)storage.pop();

		//EV << "Test size = " << test_o->getSize() << "\n";

		emit(qlenSignal, storage.length());
}

int Storage::getStorageBytes()
{
	int i;
	int total_size = 0;

	//This is inefficient, since a sequential search will be done for every element in the queue.
	//TODO: The "forEachChild" method should rather be implemented with an appropriate visitor class.
	for (i = 0 ; i < storage.getLength() ; i++)
	{
		total_size += ((Message *)storage.get(i))->getValue();
	}

	return total_size;
}

int Storage::getStorageFiles()
{
	return storage.getLength();
}