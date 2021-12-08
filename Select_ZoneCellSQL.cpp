
#include "PDBOdbcErr.hpp"
#include "Select_ZoneCellSQL.hpp"

Select_ZoneCellSQL::Select_ZoneCellSQL() {

    arrLen_ = new (std::nothrow) SQLLEN[T_ZONE_CELL_COLUMN_CNT];
    arrBc_  = new (std::nothrow) STBindColumn[T_ZONE_CELL_COLUMN_CNT];

    memset(zoneCode_, 0, sizeof(zoneCode_));
    memset(tblName_,  0, sizeof(tblName_));
    memset(locationId_, 0, sizeof(locationId_));
}

Select_ZoneCellSQL::~Select_ZoneCellSQL() {

}

bool Select_ZoneCellSQL::SetQuery(const char * _tblName, const char * _zoneCode) {

    char buf[256];
    snprintf(buf,
        sizeof(buf),
        "SELECT LOCATION_ID FROM %s WHERE ZONE_CODE='%s' AND STATUS <> 'D'",
        _tblName, _zoneCode);

    if(SetSQL(PDB::eDefDBType::Subscriber, buf, strlen(buf)) == false) {
        E_LOG("SetSQL() fail [%s]", buf);
        return false;
    }

    for(int nLoop=0; nLoop < T_ZONE_CELL_COLUMN_CNT; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBc_[0] = {1, SQL_C_CHAR,  locationId_, DB_LOCATION_ID_LEN + 1, &arrLen_[0]};
    SetBindColumn(arrBc_, 1);

    return true;
}

bool Select_ZoneCellSQL::Execute(SQLHSTMT & _stmt) {

    setLocationIds_.clear();

    sret_ = SQLExecute(_stmt);

    if(sret_ == SQL_NO_DATA)
        return true;

    if(sret_ != SQL_SUCCESS &&
       sret_ != SQL_SUCCESS_WITH_INFO) {
        E_LOG("SQLExecute fail [%d] [%s]",
            sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
        return false;
    }

    while(true) {
        sret_ = SQLFetch(_stmt);

        if(sret_ == SQL_NO_DATA)
            break;

        if(sret_ != SQL_SUCCESS && sret_ != SQL_SUCCESS_WITH_INFO) {
            E_LOG("SQLFetch fail [%d] [%s]",
                sret_, PDB::ODBCErr::GetStringSTMT(_stmt).c_str());
            return false;
        }

        setLocationIds_.emplace(locationId_);
    }

    return true;
}

