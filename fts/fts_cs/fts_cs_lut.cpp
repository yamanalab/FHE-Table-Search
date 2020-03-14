#include <sstream>
#include <fstream>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_cs/fts_cs_lut.hpp>

#define ENABLE_LOCAL_DEBUG

namespace fts_cs
{
    void fts_cs_lut_read_header(std::ifstream& ifs, LUTFunc_t& func, size_t& size)
    {
        func = kLUTFuncNil;
        size = -1;
        
        std::string line;
        getline(ifs, line);
        {
            std::string str;
            std::stringstream ss(line);
            
            getline(ss, str, ',');
            str = fts_share::utility::trim_string(str);
            if (!fts_share::utility::isdigit(str)) {
                std::ostringstream oss;
                oss << "Invalid format. (function type:" << str << ")";
                STDSC_THROW_FILE(oss.str().c_str());
            }
            func = static_cast<LUTFunc_t>(std::stoi(str));

            getline(ss, str, ',');
            str = fts_share::utility::trim_string(str);
            if (!fts_share::utility::isdigit(str)) {
                std::ostringstream oss;
                oss << "Invalid format. (table size:" << str << ")";
                STDSC_THROW_FILE(oss.str().c_str());
            }
            size = std::stoi(str);
        }
    }

    LUTFunc_t fts_cs_lut_get_funcnumber(const std::string& filepath)
    {
        if (!fts_share::utility::file_exist(filepath)) {
            std::ostringstream oss;
            oss << "File not found. (" << filepath << ")";
            STDSC_THROW_FILE(oss.str());
        }

        std::ifstream ifs(filepath, std::ios::in);

        LUTFunc_t func = kLUTFuncNil;
        size_t    size = -1;

        fts_cs_lut_read_header(ifs, func, size);
        return func;
    }
    
    
    /// LUTLFunc
    
    LUTLFunc::LUTLFunc(const std::string& filepath)
    {
        load_from_file(filepath);
    }

    void LUTLFunc::load_from_file(const std::string& filepath)
    {
        STDSC_LOG_INFO("Read LUT file. (filepath:%s)", filepath.c_str());

        if (!fts_share::utility::file_exist(filepath)) {
            std::ostringstream oss;
            oss << "File not found. (" << filepath << ")";
            STDSC_THROW_FILE(oss.str());
        }

        std::ifstream ifs(filepath, std::ios::in);
        
        LUTFunc_t func = kLUTFuncNil;
        size_t    size = -1;
        fts_cs_lut_read_header(ifs, func, size);

        if ((func != kLUTFuncLinear && func != kLUTFuncQuadratic) || size <= 0) {
            std::ostringstream oss;
            oss << "Invalid format. (filepath:" << filepath;
            oss << ", function type:" << func ;
            oss << ", table size:" << size << ")";
            STDSC_THROW_FILE(oss.str().c_str());
        }

        std::string line;
        while (getline(ifs, line)) {
            std::string str;
            std::stringstream ss(line);
            getline(ss, str, ',');
            int64_t x = std::stol(str);
            getline(ss, str, ',');
            int64_t y = std::stol(str);
            set(x, y);
        }

        if (!(super::map_.size() <= size)) {
            std::ostringstream oss;
            oss << "Invalid format. (filepath:" << filepath;
            oss << ", function type:" << func ;
            oss << ", table size:" << size;
            oss << ", actual table size:" << super::map_.size() << ")";
            STDSC_THROW_FILE(oss.str().c_str());
        }
    }

    void LUTLFunc::set(const int64_t x, const int64_t y)
    {
        emplace(generate_key(x), y);
    }
    
    int64_t LUTLFunc::get(const int64_t x)
    {
        const auto key = generate_key(x);
        if (!is_exist_key(key)) {
            std::ostringstream oss;
            oss << "Invalid key. Value is not exist in LUT. ";
            oss << "(x: " << x << ")";
            STDSC_THROW_INVPARAM(oss.str().c_str());
        }
        return super::map_[key];
    }

    std::string LUTLFunc::generate_key(const int64_t x) const
    {
        return std::to_string(x);
    }

    
    /// LUTQFunc

    LUTQFunc::LUTQFunc(const std::string& filepath)
    {
        load_from_file(filepath);
    }
        
    void LUTQFunc::load_from_file(const std::string& filepath)
    {
        STDSC_LOG_INFO("Read LUT file. (filepath:%s)", filepath.c_str());

        if (!fts_share::utility::file_exist(filepath)) {
            std::ostringstream oss;
            oss << "File not found. (" << filepath << ")";
            STDSC_THROW_FILE(oss.str());
        }

        std::ifstream ifs(filepath, std::ios::in);
        
        LUTFunc_t func = kLUTFuncNil;
        size_t    size = -1;
        fts_cs_lut_read_header(ifs, func, size);

        if ((func != kLUTFuncLinear && func != kLUTFuncQuadratic) || size <= 0) {
            std::ostringstream oss;
            oss << "Invalid format. (filepath:" << filepath;
            oss << ", function type:" << func ;
            oss << ", table size:" << size << ")";
            STDSC_THROW_FILE(oss.str().c_str());
        }

        std::string line;
        while (getline(ifs, line)) {
            std::string str;
            std::stringstream ss(line);
            getline(ss, str, ',');
            int64_t x0 = std::stol(str);
            getline(ss, str, ',');
            int64_t x1 = std::stol(str);
            getline(ss, str, ',');
            int64_t y = std::stol(str);
            set(x0, x1, y);
        }

        if (!(super::map_.size() <= size)) {
            std::ostringstream oss;
            oss << "Invalid format. (filepath:" << filepath;
            oss << ", function type:" << func ;
            oss << ", table size:" << size;
            oss << ", actual table size:" << super::map_.size() << ")";
            STDSC_THROW_FILE(oss.str().c_str());
        }
    }

    void LUTQFunc::set(const int64_t x0, const int64_t x1, const int64_t y)
    {
        emplace(generate_key(x0, x1), y);
    }
    
    int64_t LUTQFunc::get(const int64_t x0, const int64_t x1)
    {
        const auto key = generate_key(x0, x1);
        if (!is_exist_key(key)) {
            std::ostringstream oss;
            oss << "Invalid key. Value is not exist in LUT. ";
            oss << "(x0: " << x0 << ", x1: " << x1 << ")";
            STDSC_THROW_INVPARAM(oss.str().c_str());
        }
        return super::map_[key];
    }

    const std::pair<int64_t, int64_t> LUTQFunc::decode_key(const std::string& key) const
    {
        std::string str;
        std::stringstream ss(key);
        getline(ss, str, ',');
        int64_t x0 = std::stol(str);
        getline(ss, str, ',');
        int64_t x1 = std::stol(str);
        return std::make_pair(x0, x1);
    }

    std::string LUTQFunc::generate_key(const int64_t x0, const int64_t x1) const
    {
        std::ostringstream oss;
        oss << x0 << "," << x1;
        return oss.str();
    }

} /* namespace fts_cs */


#if defined ENABLE_LOCAL_DEBUG

void test_qfunc(const std::string& filepath)
{
    fts_cs::LUTQFunc lutqf(filepath);
    //lutqf.load_from_file(filepath);
    lutqf.dump();
    std::cout << "x0:1, x1:19, y:" << lutqf.get(1, 19) << std::endl;
}    

void test_lfunc(const std::string& filepath)
{
    fts_cs::LUTLFunc lutlf(filepath);
    lutlf.dump();
    std::cout << "x:15, y:" << lutlf.get(15) << std::endl;
}    

int main()
{
    test_qfunc("../../test/sample_LUT/sample_lut_two_input.csv");
    test_lfunc("../../test/sample_LUT/sample_lut_one_input.csv");
    return 1;
}
#endif
