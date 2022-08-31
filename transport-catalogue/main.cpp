#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"

#include "input_reader.h"
#include "stat_reader.h"

#include <string>
#include <iostream>
#include <utility>

using namespace std;

int main() {
    tc::TransportCatalogue tc;

    tc::reader::ReadJSON(tc, std::cin);

    return 0;
}

/*
  * Примерная структура программы:
  *
  * Считать JSON из stdin
  * Построить на его основе JSON базу данных транспортного справочника
  * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
  * с ответами.
  * Вывести в stdout ответы в виде JSON
  */
