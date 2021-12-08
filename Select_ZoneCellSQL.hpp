
#ifndef SELECT_ZONE_CELL_SQL_HPP
#define SELECT_ZONE_CELL_SQL_HPP

#include <set>

#include "PDBPreparedSQL.hpp"
#include "LmDefine.hpp"

class Select_ZoneCellSQL : public PDB::PreparedSQL {
public:
    explicit Select_ZoneCellSQL();
    ~Select_ZoneCellSQL();

    bool SetQuery(const char * _tblName, const char * _zoneCode);
    bool Execute(SQLHSTMT & _stmt);
    const std::set<std::string> & GetRecords() { return setLocationIds_; }

private:
    SQLLEN *            arrLen_;
    STBindColumn *      arrBc_;

    char    zoneCode_[DB_ZONE_CODE_LEN+1];
    char    tblName_[DB_TBL_NAME_LEN+1];
    char    locationId_[DB_LOCATION_ID_LEN+1];

    std::set<std::string>   setLocationIds_;

};


#endif // SELECT_ZONE_CELL_SQL_HPP
