/* environment.c
 *
 * Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
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

#include <stdlib.h>
#include <string.h>

#define min(a,b) ((a<b)?a:b)

int
getenv_token( const char* name, const char* token, char* value, size_t n )
{
   char* envstr = (char*)getenv(name);

   *value  = '\0';

   if (!envstr) return(1);

   char* ptr;
   char* clause = strtok_r(envstr,":",&ptr);

   while (clause) {

      char* sep = strchr(clause,'=');

      if (sep) {

         if (token && !strncasecmp(token,clause,strlen(token))) {

            strncpy(value,sep+1,min(strlen(sep+1)+1,n));
            return(0);

         }

      } else if (!token) {

         strncpy(value,clause,min(strlen(clause)+1,n));
         return(0);

      }

      clause = strtok_r(0,":",&ptr);
   }

   return(2);

}

