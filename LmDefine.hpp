#ifndef LM_DEFINE_HPP
#define LM_DEFINE_HPP

#include <string>
#include <vector>
#include <set>

#define  T_ZONE_CELL_NOTI_COLUMN_CNT    4
#define  T_ZONE_PCF_NOTI_COLUMN_CNT     3
#define  T_ZONE_CELL_COLUMN_CNT         1

#define  DB_ZONE_CODE_LEN       20
#define  DB_TBL_NAME_LEN        32
#define  DB_UPDATE_TIME_LEN     14
#define  DB_LOCATION_ID_LEN     14


struct stZoneCellNotiRecord {
    std::string     zoneCode;
    std::string     tblName;
    int             cnt = 0;
    std::string     uTime;

    stZoneCellNotiRecord() {

    }

    stZoneCellNotiRecord(const stZoneCellNotiRecord & _st) {
        zoneCode = _st.zoneCode;
        tblName  = _st.tblName;
        cnt      = _st.cnt;
        uTime    = _st.uTime;
    }

    stZoneCellNotiRecord & operator=(const stZoneCellNotiRecord & _st) {
        if(&_st != this) {
            zoneCode = _st.zoneCode;
            tblName  = _st.tblName;
            cnt = _st.cnt;
            uTime = _st.uTime;
        }

        return *this;
    }

};

struct stLocationIds {
    stZoneCellNotiRecord    record;
    std::set<std::string>   setLocationIds;

    stLocationIds() {

    }

    stLocationIds(const stLocationIds & _st) {
        record = _st.record;
        setLocationIds = _st.setLocationIds;
    }

    stLocationIds & operator=(const stLocationIds & _st) {
        if(&_st != this) {
            record = _st.record;
            setLocationIds = _st.setLocationIds;
        }

        return *this;
    }
};

struct stDeltaLocationIds {
    std::string     zoneCode;
    std::vector<std::string>    vecAdditions;
    std::vector<std::string>    vecDeletions;

    stDeltaLocationIds() {

    }

    stDeltaLocationIds(const stDeltaLocationIds & _st) {
        zoneCode = _st.zoneCode;
        vecAdditions = _st.vecAdditions;
        vecDeletions = _st.vecDeletions;
    }

    stDeltaLocationIds & operator=(const stDeltaLocationIds & _st) {
        if(&_st != this) {
            zoneCode = _st.zoneCode;
            vecAdditions = _st.vecAdditions;
            vecDeletions = _st.vecDeletions;
        }

        return *this;
    }
};


#endif // LM_DEFINE_HPP
