/*  LiteSQL - C++ Object Persistence Framework
    Copyright (C) 2008  Tero Laitinen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "internal.h"
#include <string.h>
int lsqlAddToArray(void** array, size_t* size, size_t elemSize, void** elem) {

    (*size)++;
    *array = lsqlRealloc(*array, elemSize * (*size));
   
    if (!*array) {
        *elem = NULL;
        return LSQL_MEMORY;
    }

    *elem = &((char*)*array)[((*size)-1)*elemSize];    
    memset(*elem, 0, elemSize);
    return 0;
}
