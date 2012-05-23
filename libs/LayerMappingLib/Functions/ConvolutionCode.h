// Copyright (c) 2005 - 2007 Marc de Kamps, Johannes Drever, Melanie Dietz
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

#ifndef LAYERMAPPINGLIB_FUNCTIONS_CONVOLUTIONCODE_H
#define LAYERMAPPINGLIB_FUNCTIONS_CONVOLUTIONCODE_H

#include "Convolution.h"

template<class VectorList, class Matrix>
inline Convolution<VectorList, Matrix>::Convolution( int kernel_width, int kernel_height, Matrix& matrix ) :
	_kernel_width( kernel_width ),
	_kernel_height( kernel_height ),
	_matrix( new value_type[ kernel_width * kernel_height ] )
{
	//the matrix is transposed for internal representation
	value_type* i_matrix = _matrix;
	for( typename Matrix::iterator i_row = matrix.begin();
		i_row != matrix.end();
		i_row++, i_matrix++ )
	{
		assert( (unsigned int) i_row->size() == (unsigned int) _kernel_width );
		value_type* m_r = i_matrix;
		for( typename Matrix::value_type::iterator c = i_row->begin();
			c != i_row->end();
			c++, m_r += _kernel_width )
		{
			*m_r = *c;
		}
	}
}


template<class VectorList, class Matrix>
Convolution<VectorList, Matrix>::~Convolution()
{
	delete[] _matrix;
}

template<class VectorList, class Matrix>
void Convolution<VectorList, Matrix>::operator()( vector_iterator input_begin, vector_iterator input_end,
			iterator output_begin, iterator output_end )
{
	value_type squared_sum = 0.0;
	value_type acc = 0.0;
	
	for( vector_iterator v = input_begin;
		v != input_end;
		v++ )
	{
		iterator i = input_begin->begin();
		for( value_type* m = _matrix;
			m != _matrix + _kernel_width * _kernel_height;
			m++, i++ )
		{
			acc += *i * *m;
			squared_sum += (*i) * (*i);
		}
		assert( i == v->end() ); //receptive field size does not match filter matrix dimension
	}

	if( squared_sum == 0.0 )
	{
		fill( output_begin,
			output_end,
			0.0 );
	}
	else
	{
		fill( output_begin, 
			output_end,
			fabs( acc / sqrt( squared_sum ) ) );
	}
}

#ifdef DEBUG
template<class VectorList, class Matrix>
void Convolution<VectorList, Matrix>::debug_print() const
{
	cout << "<Convolution>" << endl;
	for( value_type* i = _matrix;
		i != _matrix + _kernel_width * _kernel_height;
		 )
	{
		copy( i, i + _kernel_width, ostream_iterator<value_type>( cout, " " ) );
		i += _kernel_width;
		cout << endl;
	}
	cout << endl << "</Convolution>" << endl;
}
#endif //DEBUG

#endif //LAYERMAPPINGLIB_FUNCTIONS_CONVOLUTIONCODE_H
