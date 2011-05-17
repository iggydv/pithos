//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef PEERDATA_H_
#define PEERDATA_H_

#include <TransportAddress.h>

class PeerData
{
	private:
		TransportAddress address;
	public:
		PeerData();
		PeerData(const PeerData& other);
		virtual ~PeerData();
		PeerData& operator=(const PeerData& other);
		bool operator==(const PeerData &other) const;
		bool operator!=(const PeerData &other) const;

		void setAddress(TransportAddress adr);
		void setAddress(const char* ip_str, int port);
		TransportAddress getAddress();
};

#endif /* PEERDATA_H_ */
