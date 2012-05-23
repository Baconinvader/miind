// Copyright (c) 2005 - 2008 Marc de Kamps
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
#ifndef _CODE_LIBS_NETLIB_SIGMOIDPARAMETER_INCLUDE_GUARD
#define _CODE_LIBS_NETLIB_SIGMOIDPARAMETER_INCLUDE_GUARD

#ifdef WIN32
#pragma warning(disable: 4786)
#endif


#include <iostream>
#include "AbstractSquashingParameter.h"

using std::istream;
using std::ostream;

namespace NetLib {

	//! facilitates access to InsertParameter, GetParameter

	enum SigmoidIndex {MININUM_ACTIVITY, MAXIMUM_ACTIVITY, NOISE };

	//! SigmoidParameter
	class SigmoidParameter : public AbstractSquashingParameter 
	{
	public:

		//! default constructor, intialization to zero
		SigmoidParameter();

		//! explicit constructor
		SigmoidParameter
			(
				double f_min_act, 
				double f_max_act, 
				double f_noise
			):_f_min_act(f_min_act),
			  _f_max_act(f_max_act),
			  _f_noise(f_noise)
		{
		}

		//! virtual destructor
		virtual ~SigmoidParameter();

		double DMinimumActivity() const {return _f_min_act;}
		double DMaximumActivity() const {return _f_max_act;}
		double DNoise          () const {return _f_noise;}

		virtual bool FromStream(istream&);
		virtual bool ToStream  (ostream&) const;

		virtual string Tag() const;

	private:

		double _f_min_act;
		double _f_max_act;
		double _f_noise;

	}; // end of SigmoidParameter

} // end of ImplementationLib

#endif // include guard
