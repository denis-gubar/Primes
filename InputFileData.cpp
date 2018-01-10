#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <type_traits>
#include <vector>

using boost::property_tree::ptree;
using boost::property_tree::read_xml;
using std::enable_if;
using std::is_integral;
using std::pair;
using std::string;
using std::vector;

namespace PrimeNumbers
{
    template<typename Int,
        //Enforcing usage of integral types only
        typename T = typename enable_if<is_integral<Int>::value, Int>::type>
        struct InputFileData
    {
        vector<pair<Int, Int>> intervals;
        static InputFileData<Int, T> loadFromXML( const string& filename )
        {
            InputFileData result;
            // Create empty property tree object
            ptree tree;

            // Parse the XML into the property tree.
            read_xml( filename, tree );

            for (const auto& intervalSubtree : tree.get_child( "root.intervals" ))
                if (intervalSubtree.first == "interval")
                    result.intervals.emplace_back( intervalSubtree.second.get<Int>( "low" ), intervalSubtree.second.get<Int>( "high" ) );

            return result;
        }
    };
}