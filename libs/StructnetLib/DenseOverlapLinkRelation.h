// Copyright (c) 2005 - 2010 Marc de Kamps
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
//      If you use this software in work leading to a scientific publication, you should cite
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net

#ifndef DENSEOVERLAPINCLUDEGUARD
#define DENSEOVERLAPINCLUDEGUARD

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include "LinkRelation.h"

using std::vector;
using std::ostream;

namespace StructnetLib
{
	class DenseOverlapLinkRelation : public AbstractLinkRelation 
	{
	public:

		DenseOverlapLinkRelation();
		DenseOverlapLinkRelation(istream&);
		DenseOverlapLinkRelation(const vector<LayerDescription>& );

		// virtual destructor
		virtual	~DenseOverlapLinkRelation();

		// overload for actual lin relation function
		virtual bool operator()( const PhysicalPosition&, const PhysicalPosition& ) const;

		bool	ToStream( ostream& ) const;
		bool    Consistent() const;
		virtual vector<PhysicalPosition> VecLayerStruct() const;

		virtual	const vector<LayerDescription>& VectorLayerDescription() const;

	private:

		vector<LayerDescription> _vector_layer_description;
	};

} // end of Strucnet


#endif // include guard


