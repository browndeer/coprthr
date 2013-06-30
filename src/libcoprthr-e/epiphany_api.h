/* epiphany_api.h
 *
 * Copyright (c) 2009-2013 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* DAR */


#ifndef _epiphany_api_h
#define _epiphany_api_h

#if defined(ENABLE_EMEK_BUILD)
#include "epiphany_api_emek.h"
#elif defined(USE_OLD_ESDK)
#include "epiphany_api_4130104.h"
#else
#include "epiphany_api_4130330.h"
#endif

#endif

