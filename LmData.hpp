#ifndef LM_DATA_HPP
#define LM_DATA_HPP

#include <string>

#include "LmDefine.hpp"

#include <memory>
#include <unordered_map>
#include <queue>
#include <mutex>

// ZoneCode 별, Set

class LmData {
public:
    explicit LmData();
    ~LmData();

    bool IsUpdated(const std::vector<stZoneCellNotiRecord> & _vecZCN);
    bool IsUpdated(std::string _zcode, std::string _uTime);
    void SetReadyFromApplied(std::string _zoneCode);
    void Set(const stZoneCellNotiRecord & _st, const std::set<std::string> & _set);
    bool Update(std::string _zcode, std::string _uTime);

    size_t GetQSize();
    std::shared_ptr<stDeltaLocationIds> PopQ();
    size_t GetLocationIds(std::string _zoneCode, stLocationIds & _locationIds);
    void GetZoneCodes(std::vector<std::string> & _vec);

private:
    // ZoneCode 별, Set
    std::unordered_map<std::string, stLocationIds>      mapLocationIdsByZoneCode_;

    // 변경 데이터 queue
    std::queue<std::shared_ptr<stDeltaLocationIds>>     qDeltaLocationIds_;

    std::mutex  mutex_;
};


#endif // LM_DATA_HPP
