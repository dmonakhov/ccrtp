// Copyright (C) 2001,2002 Federico Montesino <p5087@quintero.fie.us.es>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// As a special exception to the GNU General Public License, permission is
// granted for additional uses of the text contained in its release
// of ccRTP.
//
// The exception is that, if you link the ccRTP library with other
// files to produce an executable, this does not by itself cause the
// resulting executable to be covered by the GNU General Public License.
// Your use of that executable is in no way restricted on account of
// linking the ccRTP library code into it.
// 
// This exception does not however invalidate any other reasons why
// the executable file might be covered by the GNU General Public License.
// 
// This exception applies only to the code released under the 
// name ccRTP.  If you copy code from other releases into a copy of
// ccRTP, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
// 
// If you write modifications of your own for ccRTP, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.  

/**
 * @file data.cpp
 * @short AppDataUnit class implementation.
 **/

#include "private.h"
#include <ccrtp/queuebase.h>

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

AppDataUnit::AppDataUnit(const IncomingRTPPkt& packet, const SyncSource& src):
	datablock(&packet), source(&src)
{

}

AppDataUnit::AppDataUnit(const AppDataUnit &origin): 
	datablock(origin.datablock), source(origin.source)
{
	++datablock;
}

AppDataUnit&
AppDataUnit::operator=(const AppDataUnit &rhs)
{
	datablock.operator=(rhs.datablock);
	source = rhs.source;
	return *this;
}

#ifdef	CCXX_NAMESPACES
};
#endif

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset: 8
 * End:
 */

