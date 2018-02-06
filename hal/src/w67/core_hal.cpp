/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
#include "core_hal.h"
#include "flash_map.h"
#include "intorobot_macros.h"
#include "params_hal.h"


uint16_t HAL_Core_Get_Subsys_Version(char* buffer, uint16_t len)
{
    char data[32] = "";
    uint16_t templen;

    if (buffer!=NULL && len>0) {
        sprintf(data, "1.0.2.%d", HAL_PARAMS_Get_Boot_boot_version());
        templen = MIN(strlen(data), len-1);
        memset(buffer, 0, len);
        memcpy(buffer, data, templen);
        return templen;
    }
    return 0;
}

