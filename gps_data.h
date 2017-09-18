#ifndef _GPS_DATA_H_
#define _GPS_DATA_H_




struct GpsData {
    uint8_t status;
    uint16_t speed;
    uint8_t lngDirection;
    uint8_t lngDegree;
    uint8_t lngCent;
    uint32_t lngSec;
    uint8_t latDirection;
    uint8_t latDegree;
    uint8_t latCent;
    uint32_t latSec;
    uint16_t angle;
};

namespace tc
{
	typedef struct
	{
		int		available;	//时间戳有效性 1:有效 0:无效
		int		year;
		int		month;
		int		day;
		int		hour;
		int		minute;
		int		second;
	}TimeStamp;
};




#endif	//end of gpsservice