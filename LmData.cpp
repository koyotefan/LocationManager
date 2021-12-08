
#include <algorithm>
// #include <iterator>

#include "NDFServiceLog.hpp"
#include "LmData.hpp"

LmData::LmData() {

}

LmData::~LmData() {

}

bool LmData::IsUpdated(const std::vector<stZoneCellNotiRecord> & _vecZCN) {

    for(auto & citer : _vecZCN) {
        if(IsUpdated(citer.zoneCode, citer.uTime) == true)
            return true;
    }

    return false;
}

bool LmData::IsUpdated(std::string _zcode, std::string _uTime) {

    auto iter = mapLocationIdsByZoneCode_.find(_zcode);

    if(iter == mapLocationIdsByZoneCode_.end())
        return true;

    return iter->second.record.uTime.compare(_uTime) != 0;
}

bool LmData::Update(std::string _zcode, std::string _uTime) {

    auto iter = mapLocationIdsByZoneCode_.find(_zcode);

    if(iter == mapLocationIdsByZoneCode_.end())
        return false;

    iter->second.record.uTime = _uTime;
    return true;
}

void LmData::Set(const stZoneCellNotiRecord & _record, const std::set<std::string> & _set) {

    std::lock_guard<std::mutex>  guard(mutex_);

    auto spDeltaLocationIds = std::make_shared<stDeltaLocationIds>();
    spDeltaLocationIds->zoneCode = _record.zoneCode;

    auto iter = mapLocationIdsByZoneCode_.find(_record.zoneCode);

    if(iter == mapLocationIdsByZoneCode_.end()) {
        stLocationIds       st;

        st.record  = _record;

        mapLocationIdsByZoneCode_.emplace(st.record.zoneCode, st);

        iter = mapLocationIdsByZoneCode_.find(st.record.zoneCode);
        iter->second.setLocationIds = _set;

        std::copy(_set.begin(),
                  _set.end(),
                  std::back_inserter(spDeltaLocationIds->vecAdditions));
    } else {

        stLocationIds & oldst = iter->second;

        // 기존 - 신규, 남는 것은 Delete 대상 입니다.
        std::set_difference(oldst.setLocationIds.begin(), oldst.setLocationIds.end(),
                            _set.begin(), _set.end(),
                            std::back_inserter(spDeltaLocationIds->vecDeletions));

        // 신규 - 기존, 남는 것은 Add 대상 입니다.
        std::set_difference(_set.begin(), _set.end(),
                            oldst.setLocationIds.begin(), oldst.setLocationIds.end(),
                            std::back_inserter(spDeltaLocationIds->vecAdditions));

        oldst.setLocationIds = _set;
    }

    if(spDeltaLocationIds->vecAdditions.size() +
       spDeltaLocationIds->vecDeletions.size() == 0) {
        // 변경된게 없어요.
        return ;
}

    // Queue 에 vector 를 넣어요!
    qDeltaLocationIds_.push(spDeltaLocationIds);

    stLocationIds & st = iter->second;

    I_LOG("zcode:[%s] tbl:[%s] uTime:[%s] set:[%zu] add:[%zu] del:[%zu]",
        st.record.zoneCode.c_str(),
        st.record.tblName.c_str(),
        st.record.uTime.c_str(),
        st.setLocationIds.size(),
        spDeltaLocationIds->vecAdditions.size(),
        spDeltaLocationIds->vecDeletions.size());

    return ;

}

size_t LmData::GetQSize() {

    std::lock_guard<std::mutex>  guard(mutex_);
    return qDeltaLocationIds_.size();

}

std::shared_ptr<stDeltaLocationIds> LmData::PopQ() {

    std::lock_guard<std::mutex>  guard(mutex_);

    auto ret = qDeltaLocationIds_.front();
    qDeltaLocationIds_.pop();

    return ret;
}

size_t LmData::GetLocationIds(std::string _zoneCode, stLocationIds & _locationIds) {

    std::lock_guard<std::mutex>  guard(mutex_);

    auto iter = mapLocationIdsByZoneCode_.find(_zoneCode);

    if(iter == mapLocationIdsByZoneCode_.end())
        return 0;

    _locationIds = iter->second;

    return _locationIds.setLocationIds.size();
}

void LmData::GetZoneCodes(std::vector<std::string> & _vec) {
    for(auto & citer : mapLocationIdsByZoneCode_)
        _vec.push_back(citer.first);
}
