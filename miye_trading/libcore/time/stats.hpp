/*
 * stats.hpp
 *
 * Purpose: streaming mean, sd and simple histogram output
 *
 * Author: 
 */
#pragma once

#include <limits>
#include <cmath>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>


namespace miye { namespace time {


struct stats_t
{
    stats_t()
    : mean(0.0)
    , m2(0.0)
    , N(0)
    {}

    void add(double value)
    {
        ++N;
        high = std::max(value, high);
        low = std::min(value, low);
        float delta = value - mean;
        mean += delta/N;
        m2 += delta * (value - mean);
    }

    double get_stddev() const {
        if (N < 2)
            return 0;
        return sqrt(m2/(N - 1));
    }

    double get_mean() const {
        return mean;
    }

    double get_high() const {
        return high;
    }

    double get_low() const {
        return low;
    }

    uint32_t get_N() const {
        return N;
    }

    void reset()
    {
        mean = 0.0;
        m2 = 0.0;
        high = -std::numeric_limits<double>::max();
        low = std::numeric_limits<double>::max();
        N = 0;
    }

    std::string to_str() const
    {
        std::ostringstream oss;
        oss << "      N: " << N << std::endl
            << "   High: " << high << std::endl
            << "    Low: " << low << std::endl
            << "   Mean: " << mean << std::endl
            << " StdDev: " << get_stddev() << std::endl;
        return oss.str();
    }

    static std::string csv_header()
    {
        std::ostringstream oss;
        oss << "N,low,high,mean,stddev";
        return oss.str();
    }

    std::string to_csv() const
    {
        std::ostringstream oss;
        oss << N << "," << low << "," << high << "," << mean << "," << get_stddev();
        return oss.str();
    }

private:
    double       mean;
    double       m2;
    double       high {-std::numeric_limits<double>::max()};
    double       low {std::numeric_limits<double>::max()};
    double       std {std::numeric_limits<double>::max()};
    uint32_t     N;

};


struct hist_stats_t
{

    struct bucket {
        bucket()
        : count(0)
        , upper(0)
        , lower(0)
        {}

        bucket(size_t count, double lower, double upper)
        : count(count)
        , upper(upper)
        , lower(lower)
        {}

        size_t  count;
        int64_t  upper;
        int64_t  lower;
    };

    hist_stats_t()
    : bucket_width(0)
    , histogram_height(0)
    , stddev_collapse(0)
    {}

    hist_stats_t(const size_t bucket_width)
    : bucket_width(bucket_width)
    , histogram_height(100)
    , stddev_collapse(0)
    {}

    hist_stats_t(const size_t bucket_width, const size_t histogram_height, const double stddev_collapse)
    : bucket_width(bucket_width)
    , histogram_height(histogram_height)
    , stddev_collapse(stddev_collapse)
    {}

    std::map<int64_t, size_t> map;

    void set_name(std::string tag)
    {
        name = tag;
    }

    void add(int64_t value)
    {
        stats.add(value);
        if(bucket_width > 0)
        {
            int64_t u = (value / (int64_t)bucket_width) * (int64_t)bucket_width;
            std::pair<std::map<int64_t, size_t>::iterator, bool> ret = map.insert(std::map<int64_t, uint32_t>::value_type(u, 1));
            if(!ret.second) {
                ++ret.first->second;
            }
        }
    }

    size_t get_bucket_width() const
    {
        return bucket_width;
    }

    void reset()
    {
        stats.reset();
        map.clear();
    }

    std::string to_str()
    {
        std::ostringstream oss;

        if (!name.empty())
            oss << name << "\n";

        oss << stats.to_str();

        if( bucket_width > 0)
        {
            double min = -std::numeric_limits<double>::max();
            double max = std::numeric_limits<double>::max();

            if(stddev_collapse > 0)
            {
                min = stats.get_mean() - (stddev_collapse * stats.get_stddev());
                max = stats.get_mean() + (stddev_collapse * stats.get_stddev());
            }

            int64_t start_bucket = map.begin()->first;
            int64_t end_bucket = (--map.end())->first;

            oss << "Histogram bucket size: " << bucket_width << std::endl
                << "Range: " << start_bucket << " to " << end_bucket << std::endl;
            if (stddev_collapse > 0)
                oss << "Values +/- " << stddev_collapse << " standard deviations are in head/tail bucket" << std::endl;
            oss << std::endl;
            bool in_max = false;

            std::vector<std::pair<size_t, std::string>> out;

            std::string first_line;
            std::string last_line;

            for(int64_t i = start_bucket; i <= end_bucket; i += bucket_width)
            {
                std::string label;
                size_t N = 0;
                bool in_min = false;

                auto it = map.find(i);
                if(it != map.end())
                    N += it->second;

                label = std::to_string(i);

                while(i < min)
                {
                    if (!in_min)
                    {
                        in_min = true;
                        label = std::to_string(i);
                    }

                    i += bucket_width;
                    auto it = map.find(i);
                    if(it != map.end())
                        N += it->second;
                }

                while(i > max)
                {
                    if (!in_max)
                    {
                        in_max = true;
                    }

                    i += bucket_width;
                    auto it = map.find(i);
                    if(it != map.end())
                        N += it->second;
                    if(i > end_bucket)
                        break;
                }

                if (in_min)
                {
                    first_line = label + "\n-\n";
                    label = std::to_string(i);
                }
                out.emplace_back(N, label);

            }
            if (in_max)
                last_line = "-\n" + std::to_string(end_bucket);


            //render
            size_t m = 0;
            for (auto& b : out)
                m = std::max(m, b.first);
            size_t step = std::max(size_t(1), size_t(m / histogram_height));

            size_t label_size = std::max(std::to_string(start_bucket).size(), std::to_string(end_bucket).size());
            if(first_line.size() > 0)
                oss << first_line;

            for(auto& o : out)
            {
                size_t pcount = 0;
                do{
                    if((pcount + o.second.size()) >= label_size)
                        break;
                    oss << " ";
                    ++pcount;
                }while(true);
                oss << o.second << ": ";

                int64_t remainder = o.first;
                for(size_t c = 0; c < histogram_height; ++c)
                {
                    if(remainder > 0)
                    {
                        oss << "#";
                        remainder -= step;
                    }
                    else
                        oss << " ";

                }
                oss << ": "  << o.first;
                oss << std::endl;
            }

            if(last_line.size() > 0)
                oss << last_line;
            oss << std::endl;
        }
        return oss.str();
    }


    size_t bucket_width;
    size_t histogram_height;
    double stddev_collapse;
    std::string name;
    stats_t stats;
};

}}
