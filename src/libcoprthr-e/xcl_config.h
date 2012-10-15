/* xcl_config.h
 *
 * Copyright (c) 2009-2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

/* Implementation configuration settings should be collected here -DAR */

#ifndef _XCL_CONFIG_H
#define _XCL_CONFIG_H


/* select one and only one execution model */
//#define USE_E32SER /* select serialized execution model */
//#define USE_E32SL /* select SJ/LJ threads execution model */
#define USE_E32PTH /* select parallel threads execution model */


#if !defined(USE_E32SER) && !defined(USE_E32SL) && !defined(USE_E32PTH)
#error no execution model selected
#endif


/* enable e32 OpenCL extensions */
#ifdef USE_E32PTH /* XXX developed for PTH model */
#define USE_E32_OPENCL_EXT
#endif


#endif


