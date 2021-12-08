
#ifndef UPDATE_SP_CONNECTION_SQL_HPP
#define UPDATE_SP_CONNECTION_SQL_HPP

#include "PDBPreparedSQL.hpp"

class Update_SpConnectionSQL : public : PreparedSQL {
public:
    explicit Update_SpConnectionSQL();
    ~Update_SpConnectionSQL();

    bool Bind();
    bool Execute(SQLHSTMT & _stmt);

private:
    SQLLEN *            arrLen_;
    STBindColumn *      arrBc_;

    char    peerNodeId_[DB_ZONE_CODE_LEN+1];
    char    peerProcId_[DB_TBL_NAME_LEN+1];
    int     connection_;

};

#endif // UPDATE_SP_CONNECTION_SQL_HPP
