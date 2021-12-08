
#include "PDBOdbcErr.hpp"
#include "Select_ZoneCellNotiSQL.hpp"

Select_ZoneCellNotiSQL::Select_ZoneCellNotiSQL() {
    arrLen_ = new (std::nothrow) SQLLEN[T_ZONE_CELL_NOTI_COLUMN_CNT];
    arrBc_  = new (std::nothrow) STBindColumn[T_ZONE_CELL_NOTI_COLUMN_CNT];

    memset(zoneCode_, 0, sizeof(zoneCode_));
    memset(tblName_,  0, sizeof(tblName_));
    cnt_ = 0;
    memset(uTime_,    0, sizeof(uTime_));

}

Select_ZoneCellNotiSQL::~Select_ZoneCellNotiSQL() {
    if(arrLen_) {
        delete [] arrLen_;
        arrLen_ = nullptr;
    }

    if(arrBc_) {
        delete [] arrBc_;
        arrBc_ = nullptr;
    }
}

bool Select_ZoneCellNotiSQL::Bind() {

    char buf[256];
    snprintf(buf,
        sizeof(buf),
        "select ZONE_CODE, TABLE_INFO, TO_CHAR(DATE, 'YYYYMMDDHHmmss') from T_ZONE_PCF_NOTI "
        "WHERE (ZONE_CODE, DATE) IN (SELECT ZONE_CODE, MAX(DATE) "
        "from T_ZONE_PCF_NOTI GROUP BY ZONE_CODE)");
    if(SetSQL(PDB::eDefDBType::Subscriber, buf, strlen(buf)) == false) {
        E_LOG("SetSQL() fail [%s]", buf);
        return false;
    }

    for(int nLoop=0; nLoop < T_ZONE_PCF_NOTI_COLUMN_CNT; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBc_[0] = {1, SQL_C_CHAR,  zoneCode_, DB_ZONE_CODE_LEN + 1, &arrLen_[0]};
    arrBc_[1] = {2, SQL_C_CHAR,  tblName_,  DB_TBL_NAME_LEN  + 1, &arrLen_[1]};
    arrBc_[2] = {3, SQL_C_CHAR,  uTime_,    DB_UPDATE_TIME_LEN+1, &arrLen_[2]};
    SetBindColumn(arrBc_, 3);

    return true;
}

/*-
bool Select_ZoneCellNotiSQL::Bind() {

    char buf[256];
    snprintf(buf,
        sizeof(buf),
        "SELECT ZONE_CODE, TBL_NAME, CELL_CNT, UPDATE_TIME FROM T_ZONE_CELL_NOTI");

    if(SetSQL(PDB::eDefDBType::Subscriber, buf, strlen(buf)) == false) {
        E_LOG("SetSQL() fail [%s]", buf);
        return false;
    }

    for(int nLoop=0; nLoop < T_ZONE_CELL_NOTI_COLUMN_CNT; ++nLoop)
        arrLen_[nLoop] = SQL_NTS;

    arrBc_[0] = {1, SQL_C_CHAR,  zoneCode_, DB_ZONE_CODE_LEN + 1, &arrLen_[0]};
    arrBc_[1] = {2, SQL_C_CHAR,  tblName_,  DB_TBL_NAME_LEN  + 1, &arrLen_[1]};
    arrBc_[2] = {3, SQL_INTEGER, &cnt_,     sizeof(cnt_),         &arrLen_[2]};
    arrBc_[3] = {4, SQL_C_CHAR,  uTime_,    DB_UPDATE_TIME_LEN+1, &arrLen_[3]};
    SetBindColumn(arrBc_, 4);

    return true;
}
-*/

bool Select_ZoneCellNotiSQL::Execute(SQLHSTMT & _stmt) {

    vecRecords_.clear();

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

        stZoneCellNotiRecord    st;
        st.zoneCode = zoneCode_;
        st.tblName  = tblName_;
        st.cnt      = cnt_;
        st.uTime    = uTime_;

        vecRecords_.emplace_back(st);
    }

    return true;
}
