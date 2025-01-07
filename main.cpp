#include "crow.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace rapidjson;
using namespace crow;


string get_paginated_data(int page, int page_size)
{
    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();
    Value data_array;
    data_array.SetArray();

    vector<int> data;
    for (int i = 0; i < 100; i++)
    {
        data.push_back(i);
    }
    for (int i = 0; i < data.size(); i++)
    {
        data_array.PushBack(data[i], allocator);
    }
    document.AddMember("data", data_array, allocator);

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

int main()
{
    SimpleApp app;
    vector<int> data;
    const int port = 8080;
    string url = "http://0.0.0.0:" + to_string(port) + "/";

    logger::setLogLevel(LogLevel::CRITICAL);

    CROW_ROUTE(app, "/")
    ([&](const request& req)
    {
        int page = 1;
        int page_size = INT_MAX;

        query_string query_params = query_string(req.url_params);
        if (query_params.get("page"))
        {
            page = stoi(query_params.get("page"));
        }
        if (query_params.get("page_size"))
        {
            page_size = stoi(query_params.get("page_size"));
        }

        string result = get_paginated_data(page, page_size);

        response res(result);
        res.add_header("Content-Type", "application/json");
        return res;
    });
    cout << "The websites URL: " << url << endl;
    app.port(port).multithreaded().run();
}

