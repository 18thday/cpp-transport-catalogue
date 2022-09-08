Transport Catalogue
---------------------------------------------------
It's final educational project at Yandex Practicum.

Transport Catalogue (TC) allows to store a map of public transport routes 
and to obtain information about certain route or any bus stop contained in the TC.

Input data can be assumed in JSON format or from Console in specific format.
TC handles requests of two types: "base requests" and "stat requests".
The former fiils the database fills with information about stops location
and existing public transport routes. The letter includes database queries,
which allow to get information about some stop, bus route or current database 
in the SVG image format.

Output data can be obtained in JSON format or SVG image.


System Requirements
---------------------------------------------------
* C++17 (STL)
* GCC (MinGW-w64) 11.2.0   


Installation
---------------------------------------------------
You can download the current development version as a number of cpp-files. 

Then compile files and run application through your preferred IDE.

Usage
---------------------------------------------------
Example of TC usage is contained in 
[main.cpp](https://github.com/18thday/cpp-transport-catalogue/blob/main/transport-catalogue/main.cpp)

Example of input JSON file can be found in 
[input_test_B3.json](https://github.com/18thday/cpp-transport-catalogue/blob/main/input_test_B3.json)


Upcoming Updates
---------------------------------------------------
1. Building an optimal route when moving from one point to another.
2. Adding CSV format support for creating bus stop map.
