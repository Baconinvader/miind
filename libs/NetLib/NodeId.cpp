// Copyright (c) 2005 - 2009 Marc de Kamps
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation 
//      and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software 
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY 
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//      If you use this software in work leading to a scientific publication, you should include a reference there to
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net

#include "NodeLink.h"

using namespace std;
using namespace NetLib;

NodeId& NodeId::operator=(const NodeId& id)
{
	_id_value = id._id_value;
	return *this;
}

bool NetLib::operator==(NodeId node_id_1, NodeId node_id_2)
{
	return (node_id_1._id_value == node_id_2._id_value );
}

bool NetLib::operator!=(NodeId node_id_1, NodeId node_id_2)
{
	return (node_id_1._id_value != node_id_2._id_value);
}

bool NetLib::operator<(NodeId node_id_1, NodeId node_id_2)
{
	return (node_id_1._id_value < node_id_2._id_value);
}

bool NetLib::operator<=(NodeId id1, NodeId id2)
{
	return (id1._id_value <= id2._id_value);
}

bool NetLib::operator>=(NodeId id1, NodeId id2)
{
	return (id1._id_value >= id2._id_value);
}
istream& NetLib::operator>>(istream& s, NodeId node_id)
{
	s >> node_id._id_value;
	return s;
}

ostream& NetLib::operator<<(ostream& s, NodeId node_id)
{
	s << node_id._id_value;
	return s;
}
