#ifndef _DINOBOT_CONFIGS_H
#define _DINOBOT_CONFIGS_H
#include <sstream>
#include <fstream>
#include <iterator>

#include <string>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <typeinfo> 
#include <regex>

#include <unordered_map>
#include <vector>
#include <sstream>

#include "../../utils/string_time.h"
#include "../json/slow/json/json.h"
#include "../inifile/inicpp.h"

namespace dinobot { namespace lib { 

class configs
{
public: 
    configs(std::string fn)
        : ini_file_(fn)
        , configs_(ini_file_)
        , daily_trade_data_()
    {
        interpose_date();

        // read in the other daily symbol file 
        read_daily_trade_config();

        // set exchange variable here..
        exchange_ = configs_["exchange"]["name"].as<std::string>();

    }

    ~configs()
    {
    }

    template <typename T>
    T get_config(std::string sect, std::string key)
    {
        return configs_[sect][key].as<T>();
    }

    template <typename T, typename ... Args>
    T get_startup_trade_data(Args ... args)
    {
        std::vector<std::string> arguments = {args ...}; 
        
        // This is a rubbish way to do things 
        // but it seems to work so let it be 
        switch (arguments.size())
        {
            case 0:
                throw std::invalid_argument( "get_startup_trade_config: called with 0 parameters, exiting" );
            case 1:
                return get_startup_trade_data_return_type<T>(daily_trade_data_[arguments[0]]);
            case 2:
                return get_startup_trade_data_return_type<T>(daily_trade_data_[arguments[0]][arguments[1]]);
            case 3:
                return get_startup_trade_data_return_type<T>(daily_trade_data_[arguments[0]][arguments[1]][arguments[2]]);
            case 4:
                return get_startup_trade_data_return_type<T>(daily_trade_data_[arguments[0]][arguments[1]][arguments[2]][arguments[3]]);
            case 5:
                return get_startup_trade_data_return_type<T>(daily_trade_data_[arguments[0]][arguments[1]][arguments[2]][arguments[3]][arguments[4]]);
            default:
                throw std::invalid_argument( "get_startup_trade_config: called with too many parameters (need to add more), exiting" );        
        }
    }

private:
    template <typename T>
    T get_startup_trade_data_return_type(Json::Value val)
    {
        if constexpr (std::is_same_v<T, std::string>)
            return val.asString();
        if constexpr (std::is_same_v<T, int>)
            return val.asInt();
        if constexpr (std::is_same_v<T, bool>)
            return val.asBool();
        if constexpr (std::is_same_v<T, double>)
            return val.asDouble();
        if constexpr (std::is_same_v<T, std::vector<std::string>>)
        {
            std::vector<std::string> res;
            for (auto itr : val) {
                std::string name = itr.asString();
                res.push_back(std::move(name));
            }
            return res;
        }
        // TODO add other types as needed 
    }

    // function to interpose latest startup date files
    // config = /data/dinobot.startup.binance.20180813.json
    // config = /data/dinobot.startup.binance.$DATE.json
    // Basically i want to convert $date to todays date... 
    void interpose_date()
    {
        std::string date_conf = configs_["general"]["config"].as<std::string>();
        std::string logs_path = configs_["logs"]["path_ws"].as<std::string>();
        std::string logs_rest_path = configs_["logs"]["path_rest"].as<std::string>();
        
        std::string date_str = generate_date_string(120);
        std::string res0 = std::regex_replace(date_conf, std::regex("\\$DATE"), date_str );
        std::string res1 = std::regex_replace(logs_path, std::regex("\\$DATE"), date_str );
        std::string res2 = std::regex_replace(logs_rest_path, std::regex("\\$DATE"), date_str );

        configs_["general"]["config"] = res0;
        configs_["logs"]["path_ws"] = res1;
        configs_["logs"]["path_rest"] = res2;
    }   
    
    bool read_daily_trade_config()
    {
        std::cout << "trade config file : " << configs_["general"]["config"].as<std::string>() << std::endl;

        std::ifstream config_doc(configs_["general"]["config"].as<std::string>(), std::ifstream::binary);
        if (!config_doc)
            throw std::invalid_argument( "unable to open the general trade file, exiting" );

        config_doc >> daily_trade_data_;

        return true; 
    }


private:
    std::string ini_file_;
    ini::IniFile configs_;
    Json::Value daily_trade_data_;

    std::string exchange_;
};

}}  // dinobot :: lib :: 

#endif
