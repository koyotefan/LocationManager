#ifndef SELECT_ZONE_CELL_NOTI_SQL_HPP
#define SELECT_ZONE_CELL_NOTI_SQL_HPP

#include <vector>

#include "PDBPreparedSQL.hpp"
#include "LmDefine.hpp"


class Select_ZoneCellNotiSQL : public PDB::PreparedSQL {
public:
    explicit Select_ZoneCellNotiSQL();
    ~Select_ZoneCellNotiSQL();

    bool Bind();
    bool Execute(SQLHSTMT & _stmt);
    const std::vector<stZoneCellNotiRecord> & GetRecords() { return vecRecords_; }

private:
    SQLLEN *            arrLen_;
    STBindColumn *      arrBc_;

    char    zoneCode_[DB_ZONE_CODE_LEN+1];
    char    tblName_[DB_TBL_NAME_LEN+1];
    int     cnt_;
    char    uTime_[DB_UPDATE_TIME_LEN+1];

    std::vector<stZoneCellNotiRecord>   vecRecords_;
};



#endif // SELECT_ZONE_CELL_NOTI_SQL_HPP
