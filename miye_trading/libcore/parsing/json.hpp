/*
 * json.hpp
 *
 * Purpose: JavaScript Object Notation (JSON)
 * Author: 
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <array>
#include <list>
#include <set>
#include <deque>
#include <limits>
#include <cstring>
#include <cmath>
#include <fstream>

#include "../math/math_utils.hpp"
#include "libcore/utils/dequecontig.hpp"

#include "libcore/types/types.hpp"
#include "libcore/utils/stringutils.hpp"
#include "libcore/essential/assert.hpp"
#include "libcore/utils/conversions.hpp"

// todo change vector to miye::fundamentals::vector
using std::vector;


namespace miye { namespace parsing {



template<typename T>
struct maintain {
    maintain(T& t)
    : t_(t)
    , t_v_(t)
    {}

    ~maintain()
    {
        t_ = t_v_;
    }

    T& t_;
    T t_v_;
};



static const vector<std::string> INDENT{
    "",
    "  ",
    "    ",
    "      ",
    "        ",
    "          ",
    "            ",
    "              "
};

#define pPre  (pre_pretty() ? INDENT[std::min(level_,INDENT.size())] : "")
#define pPost (post_pretty() ? "\n" : "")
template<typename OStream>
class json_writer
{
public:
    json_writer(OStream& ostream, bool strict=false, bool pretty=false, size_t max_pretty_levels=4)
    : os_(ostream)
    , strict_(strict)
    , emit_separator_(false)
    , pretty_print_(pretty)
    , max_pretty_levels_(max_pretty_levels)
    , level_(0)
    {}

    OStream& os_;
    bool strict_;
    bool emit_separator_;
    bool pretty_print_;
    size_t max_pretty_levels_;
    size_t level_;

    bool pre_pretty() const
    {
        return pretty_print_ && (level_ < max_pretty_levels_);
    }

    bool post_pretty() const
    {
        return pretty_print_ && (level_ < max_pretty_levels_);
    }

    template<typename T>
    void emit(T const& r)
    {
        json_emit(this, r);
    }

    template<typename T>
    void emit(char const* name, T const& t)
    {
        emit_name(name);
        emit(t);
    }

    // common container emits
    template<typename K, typename T, typename C, typename A>
    void emit(std::map<K, T, C, A> const& r)
    {
        emit_map(r);
    }

    template<typename K, typename T, typename C, typename A>
    void emit(std::unordered_map<K, T, C, A> const& r)
    {
        emit_map(r);
    }

    template<typename T>
    void emit(vector<T> const& r)
    {
        emit_sequence(r);
    }

    template<typename T, size_t L>
    void emit(std::array<T, L> const& r)
    {
        emit_sequence(r);
    }

    template<typename T>
    void emit(std::list<T> const& r)
    {
        emit_sequence(r);
    }

    template<typename T>
    void emit(std::set<T> const& r)
    {
        emit_sequence(r);
    }

    template<typename T>
    void emit(std::unordered_set<T> const& r)
    {
        emit_sequence(r);
    }

    template<typename T>
    void emit(std::deque<T> const& r)
    {
        emit_sequence(r);
    }

    template<typename T>
    void emit(fundamentals::dequecontig<T> const& r)
    {
        emit_sequence(r);
    }

    template<typename T1, typename T2>
    void emit(std::pair<T1,T2> const& p)
    {
        emit_pair(p);
    }

    template<class... Types>
    void emit(std::tuple<Types...> const& t)
    {
        emit_tuple(t);
    }

    template<size_t LENGTH>
    void emit(types::fixed_string<LENGTH> const& r)
    {
        emit_quoted(utils::escaped_string(r.c_str(), r.capacity()));
    }

    void emit(const char* s, size_t len)
    {
        emit_quoted(utils::escaped_string(s, len));
    }

    template <typename T>
    void visit(char const* name, T const& value)
    {
        if (emit_separator_) {
            os_ << ',' << pPost;
        }
        emit_name(name);
        emit(value);
        emit_separator_ = true;
    }

    void visit(char const* name, void const* value, size_t len)
    {
        if (emit_separator_) {
            os_ << ',' << pPost;
        }
        emit_name(name);
        vector<uint8_t> v(len);
        memcpy(v.data(), value, len);
        emit(v);
        emit_separator_ = true;
    }


    template<typename T>
    void emit_quoted(T const& t)
    {
        os_ << '"' << t << '"';
    }

    template<typename T>
    void emit_name(T const n)
    {
        os_ << pPre;
        emit_quoted(n);
        os_ << ": ";
    }
    void emit_name(std::string const& n)
    {
        emit_name(n.c_str());
    }


    void emit(bool b) { os_ << (b ? "true" : "false"); }
    void emit(char c) { emit_quoted(utils::escaped_string(&c, 1)); }
    void emit(int8_t i) { os_ << static_cast<int16_t>(i); }
    void emit(uint8_t i) { os_ << static_cast<uint16_t>(i); }
    void emit(uint16_t i) { os_ << i; }
    void emit(int16_t i) { os_ << i; }
    void emit(int32_t i) { os_ << i; }
    void emit(uint32_t i) { os_ << i; }
    void emit(int64_t i)
    {
        if (strict_) {
            os_ << '"' << i << '"';
        } else {
            os_ << i;
        }
    }
    void emit(uint64_t i)
    {
        if (strict_) {
            os_ << '"' << i << '"';
        } else {
            os_ << i;
        }
    }

    void emit(float f)
    {
        if(f != f || std::isnan(f)) {
            os_ << "NaN";
        } else if (std::isinf(f)) {
            // isinf() is broken, does not return -1 for -inf, returns +1 instead
            if(f == std::numeric_limits<float>::infinity()) {
                os_ << "Infinity";
            } else if(f == -std::numeric_limits<float>::infinity()) {
                os_ << "-Infinity";
            } else {
                INVARIANT_FAIL("JSON emit error isinf(" << f
                        << ") returned non zero but not equal to positive or negative std::numeric_limits<float>::inifinity");
            }
        } else {
            auto ff = os_.flags();
            os_.unsetf(std::ios::scientific);
            os_ << f;
            os_.setf(ff);
        }
    }
    void emit(double d)
    {

        if(d != d || std::isnan(d)) {
            os_ << "NaN";
        } else if (std::isinf(d)) {
            if(d == std::numeric_limits<double>::infinity()) {
                os_ << "Infinity";
            } else if(d == -std::numeric_limits<double>::infinity()) {
                os_ << "-Infinity";
            } else {
                INVARIANT_FAIL("JSON emit error isinf(" << d
                        << ") returned non zero but not equal to positive or negative std::numeric_limits<double>::inifinity");
            }
        } else {
            auto ff = os_.flags();
            os_.unsetf(std::ios::scientific);
            os_ << d;
            os_.setf(ff);
        }
    }
    void emit(char const * s) { emit_quoted(utils::escaped_string(s)); }
    void emit(char* s) { emit_quoted(utils::escaped_string(s)); }
    void emit(std::string const & s) { emit_quoted(utils::escaped_string(s)); }

    template <typename T1, typename T2>
    void emit_pair(std::pair<T1, T2> const& p)
    {
        os_ << '{';
        emit_name("first");
        emit(p.first);
        os_ << ',';
        emit_name("second");
        emit(p.second);
        os_ << '}';
    }

    template <int element, typename T>
    struct emit_tuple_element_t
    {
        emit_tuple_element_t(json_writer* w, T const& t)
        {
            w->emit_tuple_element<element,T>(t);
            emit_tuple_element_t<element-1, T>(w, t);
        }
    };

    template <typename T>
    struct emit_tuple_element_t<1, T>
    {
        emit_tuple_element_t(json_writer* w, T const& t)
        {
            w->emit_tuple_element<1,T>(t);
        }
    };

    template <int element, typename T>
    void emit_tuple_element(T const& t)
    {
        std::ostringstream oss;
        oss << (std::tuple_size<T>::value - element);
        emit_name(oss.str());
        emit(std::get<std::tuple_size<T>::value - element>(t));
        if (element != 1) {
            os_ << ',';
        }
    }

    template <typename T>
    void emit_tuple(T const& t)
    {
        os_ << '{';
        emit_tuple_element_t<std::tuple_size<T>::value, T>(this, t);
        os_ << '}';
    }

    template <typename Map>
    void emit_map(Map const& m)
    {
        os_ << "{" << pPost;
        if (pretty_print_) { ++level_; }
        bool emit_separator_cpy = emit_separator_;
        emit_separator_ = false;
        for (auto kv = m.begin(); kv != m.end(); ++kv) {
            if (emit_separator_) {
                os_ << ',' << pPost;
            }
            os_ << pPre;
            emit_name(kv->first);
            emit(kv->second);
            emit_separator_ = true;
        }
        if (emit_separator_) {
            os_ << pPost;
        }
        if (pretty_print_) { --level_; }
        os_ << pPre << "}" << pPost;
        emit_separator_ = emit_separator_cpy;
    }

    template <typename T>
    void emit_sequence(T const& array)
    {
        if (array.size() == 0) {
            os_ << "[]";
        } else {
            os_ << "[";
            if (pretty_print_) ++level_;
            os_ << pPost;
            bool emit_separator_cpy = emit_separator_;
            emit_separator_ = false;
            for (auto i = array.begin(); i != array.end(); ++i) {
                if (emit_separator_) {
                    os_ << ',' << pPost;
                }
                os_ << pPre;
                emit(*i);
                emit_separator_ = true;
            }
            if (emit_separator_) {
                os_ << pPost;
            }
            if (pretty_print_) { --level_; }
            os_ << pPre << "]";
            emit_separator_ = emit_separator_cpy;
        }
    }
};


template <bool IsEnum>
struct indirect_emit
{
    template <typename Emitter, typename T>
    void operator()(Emitter* lhs, T const& rhs) const
    {
        bool emit_separator_cpy = lhs->emit_separator_;
        lhs->emit_separator_ = false;
        lhs->os_ << "{";
        ++lhs->level_;
        lhs->os_ << (lhs->pre_pretty() ? "\n" : "");
        T::visit(&rhs, lhs);
        lhs->os_ << (lhs->pre_pretty() && lhs->emit_separator_ ? "\n" : "");
        --lhs->level_;
        lhs->os_ << (lhs->pre_pretty() ? INDENT[std::min(lhs->level_,INDENT.size())] : "") << "}";
        lhs->emit_separator_ = emit_separator_cpy;
    }
};

template <>
struct indirect_emit<true>
{
    template <typename Emitter, typename T>
    void operator()(Emitter* lhs, T const& rhs) const
    {
        typedef typename std::underlying_type<T>::type U;
        lhs->emit(static_cast<U>(rhs));
    }
    template <typename Emitter>
    void operator()(Emitter* lhs, std::string const& rhs) const
    {
        lhs->emit(rhs);
    }
};


template <typename Emitter, typename T>
inline void json_emit(Emitter* lhs, T const& rhs)
{
    indirect_emit<std::is_enum<T>::value> functor;
    functor(lhs, rhs);
}

template <typename T>
struct json_t
{
    json_t(const T& obj, bool pretty=false, size_t max_pretty_levels=4) : obj(obj), pretty(pretty), max_pretty_levels(max_pretty_levels) {}
    T const& obj;
    bool pretty;
    size_t max_pretty_levels;
};

template<typename T>
inline json_t<T> json(T const& t, bool pretty=false, size_t max_pretty_levels=4) { return json_t<T>(t,pretty,max_pretty_levels); }


template <typename OStream, typename T>
OStream& operator<<(OStream& os, json_t<T> const& rhs)
{
    json_writer<OStream> e(os, false, rhs.pretty, rhs.max_pretty_levels);
    e.emit(rhs.obj);
    return os;
}

enum class json_type_t {object, array, name, string, integer, real, keyword, undefined};

template<typename OStream>
OStream& operator<<(OStream& os, json_type_t type)
{
    switch (type)
    {
    case json_type_t::object:
        os << "object";
        break;
    case json_type_t::array:
        os << "array";
        break;
    case json_type_t::name:
        os << "name";
        break;
    case json_type_t::string:
        os << "string";
        break;
    case json_type_t::integer:
        os << "integer";
        break;
    case json_type_t::real:
        os << "real";
        break;
    case json_type_t::keyword:
        os << "keyword";
        break;
    case json_type_t::undefined:
        os << "undefined";
        break;
    default:
        os << "WTF is json_type_t == " << static_cast<int>(type);
        break;
    }
    return os;
}

template<typename IStream>
class json_reader
{
    public:
    explicit json_reader(IStream& istream)
    : is_(istream)
    , strict_(false)
    , root_node_(-1)
    , current_node_(-1)
    {}

    template<typename T>
    void restore(T& object)
    {
        current_node_ = root_node_;
        parse_json();
        restore(root_node_, object);
    }

    template<typename T>
    void restore(int node, T& r)
    {
        json_restore(node, r, this);
    }

    template <typename T>
    void visit(char const* name, T& value)
    {

        for (auto i = nodes_[current_node_].children.begin(); i != nodes_[current_node_].children.end(); ++i)
        {
            if (!strcmp(name, nodes_[*i].value.data())) {
                restore(nodes_[*i].children[0], value);
                break;
            } else if (strict_) {
                INVARIANT_FAIL("JSON restoring error. Element: " << name << " not present.");
            }
        }
    }

    void visit(char const* name, void* value, size_t length)
    {
        for (auto i = nodes_[current_node_].children.begin(); i != nodes_[current_node_].children.end(); ++i)
        {
            if (!strcmp(name, nodes_[*i].value.data())) {
                vector<uint8_t> vec(length);
                restore(nodes_[*i].children[0], vec);
                std::cerr << "memcop " << value << " " << vec.data() << "\n";
                memcpy(value, vec.data(), length);
                break;
            } else if (strict_) {
                INVARIANT_FAIL("JSON restoring error. Element: " << name << " not present.");
            }
        }
    }

    template<typename K, typename T>
    void restore(int node, std::map<K, T>& r) { r.clear(); restore_map(node, r); }
    template<typename K, typename T>
    void restore(int node, std::unordered_map<K, T>& r)  { r.clear(); restore_map(node, r); }
    template<typename T>
    void restore(int node, vector<T>& r) { r.clear(); restore_sequence(node, r); }
    template<typename T, size_t L>
    void restore(int node, std::array<T, L>& r) { restore_array(node, r); }
    template<typename T>
    void restore(int node, std::list<T>& r) { r.clear(); restore_sequence(node, r); }
    template<typename T>
    void restore(int node, std::set<T>& r) { r.clear(); restore_set(node, r); }
    template<typename T>
    void restore(int node, std::unordered_set<T>& r) { r.clear(); restore_set(node, r); }
    template<typename T>
    void restore(int node, std::deque<T>& r) { r.clear(); restore_sequence(node, r); }
    template<typename T>
    void restore(int node, fundamentals::dequecontig<T>& r) { r.clear(); restore_sequence(node, r); }

    template<typename T1, typename T2>
    void restore(int node, std::pair<T1,T2>& p) { restore_pair(node, p); }

    template<typename... Types>
    void restore(int node, std::tuple<Types...>& p) { restore_tuple(node, p); }


    void restore(int node, bool& b)
    {
        if (nodes_[node].type == json_type_t::keyword) {
            if (!strcmp(nodes_[node].value.data(), "true")) {
                b = true;
            } else if (!strcmp(nodes_[node].value.data(), "false")) {
                b = false;
            } else {
                INVARIANT_FAIL("JSON restoring error. Boolean element must have value [true|false].");
            }
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected a boolean value.");
        }
    }

    void restore(int node, char& c)
    {
        if (nodes_[node].type == json_type_t::string) {
            types::fixed_string<1> h;
            types::fixedstring_ostream<1> pss(h);
            pss << utils::unescaped_string(nodes_[node].value.data(), nodes_[node].value.size()-1);
            c = h[0];
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected a string element of length 1.");
        }
    }

    template<typename T>
    void restore_number(int node, T& n)
    {
        if (nodes_[node].type == json_type_t::integer || nodes_[node].type == json_type_t::real) {
            std::istringstream iss(nodes_[node].value.data());
            if(iss.str().find("NaN")!= std::string::npos || iss.str().find("nan") != std::string::npos) {
                if(nodes_[node].value.data()[0] == '-') {
                    INVARIANT_FAIL("JSON restoring error. Can't have negative NaN, file: "
         << __FILE__ << " on line: " << __LINE__ << " " << nodes_[node].type << " " <<nodes_[node].value.data());
                } else {
                    n = std::numeric_limits<T>::quiet_NaN();
                }
            } else if(iss.str().find("Infinity")!= std::string::npos || iss.str().find("inf") != std::string::npos) {
                if(nodes_[node].value.data()[0] == '-') {
                    n = -std::numeric_limits<T>::infinity();
                } else {
                    n = std::numeric_limits<T>::infinity();
                }
            } else {
                n = 0;
                iss >> n >> std::ws;
            }
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected a number element. file: "
         << __FILE__ << " on line: " << __LINE__ << " "
         << nodes_[node].type << " " <<nodes_[node].value.data());
        }

    }

    void restore_number(int node, uint8_t& n)
    {
        if (nodes_[node].type == json_type_t::integer || nodes_[node].type == json_type_t::real) {
            std::istringstream iss(nodes_[node].value.data());
            int i = 0;
            iss >> i;
            n = 0;
            n = static_cast<uint8_t>(i);
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected a number element." << __LINE__);
        }

    }

    void restore_number(int node, int8_t& n)
    {
        if (nodes_[node].type == json_type_t::integer || nodes_[node].type == json_type_t::real) {
            std::istringstream iss(nodes_[node].value.data());
            int i = 0;
            iss >> i;
            n = 0;
            n = static_cast<int8_t>(i);
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected a number element." << __LINE__);
        }

    }

    void restore(int node, int8_t& i) { restore_number(node, i); }
    void restore(int node, uint8_t& i) { restore_number(node, i); }
    void restore(int node, uint16_t& i) { restore_number(node, i); }
    void restore(int node, int16_t& i) { restore_number(node, i); }
    void restore(int node, int32_t& i) { restore_number(node, i); }
    void restore(int node, uint32_t& i) { restore_number(node, i); ; }
    void restore(int node, int64_t& i) { restore_number(node, i); }
    void restore(int node, uint64_t& i) { restore_number(node, i); }
    void restore(int node, float& f) { restore_number(node, f); }
    void restore(int node, double& d) { restore_number(node, d); }

    void restore(int node, std::string& s)
    {
        if (nodes_[node].type == json_type_t::string) {
            std::ostringstream oss;
            oss << utils::unescaped_string(nodes_[node].value.data());
            s = oss.str();
            return;
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected a string element [" << nodes_[node].value.data() << "]");
        }
    }

    template<size_t LENGTH>
    void restore(int node, types::fixed_string<LENGTH>& s)
    {
        if (nodes_[node].type == json_type_t::string) {
            s.clear();
            types::fixedstring_ostream<LENGTH> pss(s);
            pss << utils::unescaped_string(nodes_[node].value.data(), nodes_[node].value.size()-1);
            return;
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected a string element.");
        }
    }

    template<typename T1, typename T2>
    void restore_pair(int node, std::pair<T1,T2>& rhs)
    {
        if (nodes_[node].type == json_type_t::object) {
            ASSERT(nodes_[node].children.size() == 2);
            for (auto i = nodes_[node].children.begin(); i != nodes_[node].children.end(); ++i) {
                std::string field;
                std::istringstream iss(nodes_[*i].value.data());
                iss >> field;
                if (field == "first" || field == "0") {
                    restore(nodes_[*i].children[0], rhs.first);
                } else if (field == "second" || field == "1") {
                    restore(nodes_[*i].children[0], rhs.second);
                }
            }
        } else {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << "JSON restoring error. Expected an object element.");
        }
    }


    template <int element, typename T>
    struct restore_tuple_element_t
    {
        restore_tuple_element_t(json_reader<IStream>* r, vector<int>::iterator& n, T& t)
        {
            r->restore_tuple_element<element,T>(n, t);
            ++n;
            restore_tuple_element_t<element-1, T>(r, n, t);
        }
    };

    template <typename T>
    struct restore_tuple_element_t<1, T>
    {
        restore_tuple_element_t(json_reader<IStream>* r, vector<int>::iterator& n, T& t)
        {
            r->restore_tuple_element<1,T>(n, t);
        }
    };

    template <int element, typename T>
    void restore_tuple_element(vector<int>::iterator& n, T& t)
    {
        std::string field;
        std::istringstream iss(nodes_[*n].value.data());
        iss >> field;
        restore(nodes_[*n].children[0], std::get<std::tuple_size<T>::value - element>(t));
    }

    template<typename T>
    void restore_tuple(int node, T& rhs)
    {
        if (nodes_[node].type == json_type_t::object) {
            ASSERT(nodes_[node].children.size() == std::tuple_size<T>::value);
            auto iter = nodes_[node].children.begin();
            restore_tuple_element_t<std::tuple_size<T>::value, T>(this, iter, rhs);
        } else {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << "JSON restoring error. Expected an object element.");
        }
    }


    template <typename KeyType>
    void restore_key(char const* str, KeyType& key)
    {
        std::istringstream iss(str);
        iss >> key;
    }

    void restore_key(char const* str, std::string& key)
    {
        key = str;
    }

    template <typename Map>
    void restore_map(int node, Map& rhs)
    {
        if (nodes_[node].type == json_type_t::object) {
            for (auto i = nodes_[node].children.begin(); i != nodes_[node].children.end(); ++i) {
                typename Map::key_type key;
                restore_key(nodes_[*i].value.data(), key);
                restore(nodes_[*i].children[0], rhs[key]);
            }
        } else {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << "JSON restoring error. Expected an object element.");
        }
    }

    template <typename T>
    void restore_sequence(int node, T& array)
    {
        if (nodes_[node].type == json_type_t::array) {
            ASSERT(array.size() == 0);
            for (auto i = nodes_[node].children.begin(); i != nodes_[node].children.end(); ++i) {
                typename T::value_type value;
                restore(*i, value);
                array.push_back(value);
            }
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected an array element.");
        }
    }

    template <typename T>
    void restore_array(int node, T& array)
    {
        if (nodes_[node].type == json_type_t::array) {
            int index = 0;
            for (auto i = nodes_[node].children.begin(); i != nodes_[node].children.end(); ++i, ++index) {
                restore(*i, array[index]);
            }
        } else {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << "JSON restoring error. Expected an array element.");
        }
    }


    template <typename T>
    void restore_set(int node, T& array)
    {
        if (nodes_[node].type == json_type_t::array) {
            for (auto i = nodes_[node].children.begin(); i != nodes_[node].children.end(); ++i) {
                typename T::value_type value;
                restore(*i, value);
                array.insert(value);
            }
        } else {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << "JSON restoring error. Expected an array element.");
        }
    }


    struct node
    {
        node() : type(json_type_t::undefined)
        {
            value.reserve(256);
        }
        json_type_t type;
        vector<char> value;
        vector<int> children;
    };
    IStream& is_;
    bool strict_;

    vector<node> nodes_;
    int root_node_;
    int current_node_;

    void parse_json()
    {
        nodes_.reserve(1024);
        current_node_ = -1;
        root_node_ = parse_node(false);
        ASSERT(root_node_ != -1);
    }

    int parse_node(bool expect_string)
    {
        if (is_.peek() != std::char_traits<char>::eof()) {
            is_ >> std::ws;
        }
        if (is_.peek() == std::char_traits<char>::eof()) {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Unexpected EOF");
        }
        char c = is_.peek();
        int node_index = -1;
        switch(c)
        {
        case '{':
            node_index = parse_object();
            break;
        case '[':
            node_index = parse_sequence();
            break;
        case '"':
            node_index = parse_string_or_name(expect_string?json_type_t::string:json_type_t::name);
            break;
        case 't':
            node_index = parse_keyword("true");
            break;
        case 'f':
            node_index = parse_keyword("false");
            break;
        case 'n':
        {
            c = is_.get();
            if(c == 'u') {
                node_index = parse_keyword("null");
                break;
            } else {
                is_.putback(c);
            }
        }

        case 'N':
        case 'I':
        case 'i':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            node_index = parse_number();
            break;
        default:
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Unexpected character: " << c);
            break;
        }
        return node_index;
    }

    int parse_object()
    {
        char c = is_.get();
        ASSERT(c == '{');
        nodes_.push_back(node());
        int node_index = nodes_.size() - 1;
        current_node_ = node_index;
        nodes_[node_index].type = json_type_t::object;
        nodes_[node_index].children.reserve(100);
        bool expect_name_value = false;
        if (is_.peek() != std::char_traits<char>::eof()) {
            is_ >> std::ws;
        }
        while (is_.peek() != std::char_traits<char>::eof() && (c = is_.peek()) != '}') {
            switch(c)
            {
            case ',':
            {
                if (expect_name_value) {
                    INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Consecutive commas.");
                }
                is_.get();
                expect_name_value = true;
                break;
            }
            case '"':
            {
                int result = parse_name_value();
                nodes_[node_index].children.push_back(result);
                expect_name_value = false;
                break;
            }
            default:
            {
                INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Unexpected character: " << c);
                break;
            }
            }
            if (is_.peek() != std::char_traits<char>::eof()) {
                is_ >> std::ws;
            }
        }
        if (is_.peek() == std::char_traits<char>::eof()) {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Unexpected EOF.");
        }
        if (expect_name_value) {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Trailing comma in object.");
        }
        c = is_.get();
        ASSERT(c == '}');
        return node_index;
    }

    int parse_sequence()
    {
        char c = is_.get();
        ASSERT(c == '[');
        nodes_.push_back(node());
        int node_index = nodes_.size() - 1;
        current_node_ = node_index;
        nodes_[node_index].type = json_type_t::array;
        nodes_[node_index].children.reserve(100);
        bool expect_value = false;
        if (is_.peek() != std::char_traits<char>::eof()) {
            is_ >> std::ws;
        }
        while (is_.peek() != std::char_traits<char>::eof() && (c = is_.peek()) != ']') {
            switch(c) {
            case ',':
            {
                if (expect_value) {
                    INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Consecutive commas.");
                }
                is_.get();
                expect_value = true;
                break;
            }
            default:
            {
                int result = parse_node(true);
                nodes_[node_index].children.push_back(result);
                expect_value = false;
                break;
            }
            }
            if (is_.peek() != std::char_traits<char>::eof()) {
                is_ >> std::ws;
            }
        }
        if (is_.peek() == std::char_traits<char>::eof()) {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Unexpected EOF.");
        }
        if (expect_value) {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Trailing comma in sequence.");
        }
        c = is_.get();
        ASSERT(c == ']');
        return node_index;
    }


    int parse_string_or_name(json_type_t type)
    {
        char c = is_.get();
        ASSERT(c == '"');
        nodes_.push_back(node());
        int node_index = nodes_.size() - 1;
        current_node_ = node_index;
        nodes_[node_index].type = type;
        bool is_escaped = false;
        while (is_.peek() != std::char_traits<char>::eof()) {
            c = is_.get();
            if (c == '"' && !is_escaped) {
                break;
            }
            is_escaped = c == '\\';
            nodes_[node_index].value.push_back(c);
        }
        if (c != '"') {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Terminating '\"' not found.");
        }
        nodes_[node_index].value.push_back('\0');
        return node_index;
    }

    int parse_number()
    {
        nodes_.push_back(node());
        int node_index = nodes_.size() - 1;
        current_node_ = node_index;
        nodes_[node_index].type = json_type_t::integer;
        char c = is_.get();
        ASSERT(c == '-' || c == 'N' || c == 'I' || c == 'n' || c == 'i' || isdigit(c));
        if(c == 'N' || c == 'I' || c == 'n' || c == 'i') {
            is_.unget();
        } else {
            nodes_[node_index].value.push_back(c);
        }
        bool seen_decimal = false;
        bool seen_exponent = false;
        bool end_of_number = false;
        while (!end_of_number && is_.peek() != std::char_traits<char>::eof()) {
            c = is_.peek();
            switch(c) {
            case 'N':
            case 'n':
            {
                parse_nan();
                end_of_number = true;
                break;
            }
            case 'I':
            case 'i':
            {
                parse_inf();
                end_of_number = true;
                break;
            }
            case '.':
            {
                if (seen_decimal) {
                    INVARIANT_FAIL( __func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Only 1 '.' allowed in number.");
                }
                seen_decimal = true;
                nodes_[node_index].type = json_type_t::real;
                break;
            }
            case 'e':
            case 'E':
            {
                if (seen_exponent) {
                    INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Only 1 exponent allowed in number.");
                }
                seen_exponent = true;
                nodes_[node_index].type = json_type_t::real;
                break;
            }
            case '-':
            case '+':
            {
                if (!(nodes_[node_index].value.back() == 'e' || nodes_[node_index].value.back() == 'E')) {
                    INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Sign only allowed immediately after the exponent.");
                }

                break;
            }
            default:
            {
                if (!isdigit(c)) {
                    end_of_number = true;
                }
                break;
            }
            }
            if (!end_of_number) {
                nodes_[node_index].value.push_back(c);
                is_.get();
            }
        }
        nodes_[node_index].value.push_back('\0');
        return node_index;
    }

    void parse_nan()
    {
        nodes_[current_node_].type = json_type_t::real;
        if(is_.peek() == 'N') {
            static const char expected[] = "NaN";

            for(size_t i = 0;i < (sizeof(expected) - 1) && is_.peek() != std::char_traits<char>::eof(); ++i) {
                int c = is_.get();
                if(c != expected[i]) {
                    INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : expected nan");
                }
                nodes_[current_node_].value.push_back(c);
            }
        } else if (is_.peek() == 'n') {
             static const char expected[] = "nan";
            for(size_t i = 0;i < (sizeof(expected) - 1) && is_.peek() != std::char_traits<char>::eof(); ++i) {
                int c = is_.get();
                if(c != expected[i]) {
                    INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : expected nan");
                }
                nodes_[current_node_].value.push_back(c);
            }
        }
    }
    void parse_inf()
    {
        nodes_[current_node_].type = json_type_t::real;
        if(is_.peek() == 'I') {
            static const char expected[] = "Infinity";
            for(size_t i = 0;i < (sizeof(expected) - 1) && is_.peek() != std::char_traits<char>::eof(); ++i) {
                int c = is_.get();
                if(c != expected[i]) {
                    INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : expected inf");
                }
                nodes_[current_node_].value.push_back(c);
            }
        }
        else if (is_.peek() == 'i') {
            static const char expected[] = "inf";
            for(size_t i = 0;i < (sizeof(expected) - 1) && is_.peek() != std::char_traits<char>::eof(); ++i) {
                int c = is_.get();
                if(c != expected[i]) {
                    INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : expected inf");
                }
                nodes_[current_node_].value.push_back(c);
            }
        }
    }


    int parse_keyword(char const* expect)
    {
        nodes_.push_back(node());
        int node_index = nodes_.size() - 1;
        current_node_ = node_index;
        nodes_[node_index].type = json_type_t::keyword;
        char c = is_.get();
        ASSERT(c == 't' || c == 'f' || c == 'n');
        int i = 0;
        nodes_[node_index].value.push_back(c);
        ++i;
        while (expect[i] != '\0') {
            c = is_.peek();
            if (c != expect[i]) {
                INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Unexpected character in keyword (true|false|null): " << c);
            }
            nodes_[node_index].value.push_back(is_.get());
            ++i;
        }
        nodes_[node_index].value.push_back('\0');
        return node_index;
    }

    int parse_name_value()
    {
        int name_index = parse_string_or_name(json_type_t::name);
        if (is_.peek() != std::char_traits<char>::eof()) {
           is_ >> std::ws;
        }
        char c = is_.peek();
        if (c != ':') {
            INVARIANT_FAIL(__func__ << ":" << __LINE__ << " JSON parsing error at: " << is_.tellg() << " : Expected ':' separating name: value got: " << c);
        }
        is_.get();
        if (is_.peek() != std::char_traits<char>::eof()) {
           is_ >> std::ws;
        }
        int value_index = parse_node(true);
        nodes_[name_index].children.push_back(value_index);
        return name_index;
    }

    int set_node(int n)
    {
        int old_node =current_node_;
        current_node_ = n;
        return old_node;
    }

    json_reader<IStream>::node const& get_node(int n) const { return nodes_[n]; }

    std::string to_string(json_type_t t)
    {
        if (t == json_type_t::array) return "array";
        else if (t == json_type_t::name) return "name";
        else if (t == json_type_t::string) return "string";
        else if (t == json_type_t::integer) return "integer";
        else if (t == json_type_t::real) return "real";
        else if (t == json_type_t::keyword) return "keyword";
        else if (t == json_type_t::object) return "object";
        else if (t == json_type_t::undefined) return "undefined";
        return "unknown node_type_t";
    }
    void dump_nodes()
    {
        std::cout << "Nodes {\n";
        for (size_t i = 0; i < nodes_.size(); ++i) {
            node n = nodes_[i];
            std::cout << i << " : " << to_string(n.type) << " [";
            for (size_t j = 0; j < n.value.size(); ++j) {
                std::cout << n.value[j];
            }
            std::cout << "]";
            if (n.children.size() != 0) {
                std::cout << " children [";
                for (size_t k = 0; k < n.children.size(); ++k) {
                    std::cout << " " << n.children[k] << " ";
                }
                std::cout << "]";
            }
            std::cout << "\n";
        }
        std::cout << "}\n";
    }

    template<bool T> friend struct indirect_restore;
    template<typename T> friend struct node_scope;
};

template<typename IStream>
struct node_scope
{
    node_scope(json_reader<IStream>* jr, int new_node)
    : jr_(jr)
    {
        old_node = jr_->set_node(new_node);
    }
    ~node_scope()
    {
        jr_->set_node(old_node);
    }
    int old_node;
    json_reader<IStream>* jr_;
};

template <bool IsEnum>
struct indirect_restore
{
    template <typename T, typename IStream>
    void operator()(int node, T& o, json_reader<IStream>* reader) const
    {
        if (reader->get_node(node).type == json_type_t::object) {
            node_scope<IStream> ns(reader, node);
            T::visit(&o, reader);
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected an object element.");
        }
    }
};

template <>
struct indirect_restore<true>
{
    template <typename T, typename IStream>
    void operator()(int node, T& n, json_reader<IStream>* reader) const
    {
        if (reader->get_node(node).type == json_type_t::integer) {
            int i;
            reader->restore(node, i);
            n = T(i);
        } else if(reader->get_node(node).type == json_type_t::string) {
            char c;
            reader->restore(node, c);
            n = (T)c;
        } else {
            INVARIANT_FAIL("JSON restoring error. Expected a number element.");
        }
    }
};


template <typename T, typename IStream>
inline void json_restore(int node, T& o, json_reader<IStream>* reader)
{
    indirect_restore<std::is_enum<T>::value> functor;
    functor(node, o, reader);
}

template<typename IStream, typename T>
void parse_json(T& o, IStream& is)
{
    json_reader<IStream> reader(is);
    reader.restore(o);
}

template<typename T>
void parse_json(T& o, char const* str)
{
    std::istringstream iss(str);
    parse_json(o, iss);
}

template<typename T>
void parse_json(T& o, std::string const& str)
{
    std::istringstream iss(str);
    parse_json(o, iss);
}

inline std::string get_json_value(std::string const& json, std::string const& key, char start_delim, char end_delim, bool strip_delims = false)
{
    size_t delim_adjust = strip_delims ? 1 : 0;
    size_t pos = json.find(key);
    if(pos != std::string::npos) {
        size_t i = key.size();
        for (; i < json.size()  && json[i] != start_delim; ++i) {
            ;
        }
        size_t contents_start = i + delim_adjust;
        int openers = 1;
        ++i;
        for (; i < json.size() && (openers > 0); ++i) {
            if(json[i] == end_delim) {
                --openers;
            } else if(json[i] == start_delim) {
                ++openers;
            }
        }
        return json.substr(contents_start, i - contents_start - delim_adjust);
    }
    return "";
}

}}
