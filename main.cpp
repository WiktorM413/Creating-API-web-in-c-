#include "crow.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "curl/curl.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace rapidjson;
using namespace crow;

size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ofstream* outFile = static_cast<ofstream*>(userp);
    size_t totalSize = size * nmemb;
    outFile->write(reinterpret_cast<char*>(contents), totalSize);
    return totalSize;
}
size_t WriteStringCallback(void* contents, size_t size, size_t nmemb, string* out)
{
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}


string GetJSONFromApiLink(const char* APILink)
{
    CURL* curl;
    CURLcode res;
    string response;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, APILink);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStringCallback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            cerr << "Error while accessing API" << endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return response;
}

char* GetDataFromJSON(const string JSON, const char* name)
{
    Document document;
    document.Parse(JSON.c_str());
    if (document.HasParseError())
    {
        cerr << "Error while Parsing JSON" << endl;
        return 0;
    }
    return (char*)(document[name].GetString());
}

void DownloadImageToFolder(const char* path, const char* url)
{
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    ofstream File(path, ios::binary);
    if (curl)
    {
        if (!File.is_open())
        {
            cerr << "Could not open file\n";
            return;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &File);


        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            cerr << "Curl error: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    File.close();
}
string GetIMGExtensionFromURL(const char* URL)
{
    string s = URL;
    int index = -1;
    for (int i = s.length() - 1; i >= 0; --i)
    {
        if (s[i] == '.')
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        return "";
    }
    string extension = "";
    extension.append(s, index, s.length() - 1);
    return extension;
}
void foo(const string folderPath, const int ammount)
{
    const char* url = "https://api.waifu.pics/sfw/waifu";

    for (int i = 0; i < ammount; i++)
    {
        string JSON = GetJSONFromApiLink(url);
        char* ImgURL = GetDataFromJSON(JSON, "url");
        string filename = "img" + to_string(i + 1);
        string extension = GetIMGExtensionFromURL(ImgURL);
        cout << extension << endl;
        string filepath = folderPath + filename + extension;
        cout << JSON << "\n\n\n";

        DownloadImageToFolder(filepath.c_str(), ImgURL);
    }
}

string get_paginated_data()
{
    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();
    Value data_array(kArrayType);

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

map<string, string> parseFormData(const string& body)
{
    map<string, string> form_data;
    size_t start = 0;

    while (start < body.size())
    {
        size_t end = body.find('&', start);
        if (end == string::npos)
        {
            end = body.size();
        }

        string pair = body.substr(start, end - start);
        size_t equal_sign = pair.find('=');

        if (equal_sign != string::npos)
        {
            string key = pair.substr(0, equal_sign);
            string value = pair.substr(equal_sign + 1);

            auto decode = [](const string& str) {
                string decoded;
                for (size_t i = 0; i < str.size(); ++i)
                {
                    if (str[i] == '%' && i + 2 < str.size())
                    {
                        int hex = stoi(str.substr(i + 1, 2), nullptr, 16);
                        decoded += static_cast<char>(hex);
                        i += 2;
                    }
                    else if (str[i] == '+')
                    {
                        decoded += ' ';
                    }
                    else
                    {
                        decoded += str[i];
                    }
                }
                return decoded;
            };

            form_data[decode(key)] = decode(value);
        }

        start = end + 1;
    }

    return form_data;
}

int main()
{
    SimpleApp app;
    const int port = 8080;
    string url = "http://0.0.0.0:" + to_string(port) + "/";
    string html = R"(<!DOCTYPE html>
            <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <title>Button Example</title>
                <style>
                    button {
                        padding: 10px 20px;
                        margin: 10px;
                        font-size: 16px;
                        cursor: pointer;
                    }
                </style>
            </head>
            <body>
                <form action="/" method="POST">
                    <input type="text" id="path" name="path" placeholder="Enter path" required><br>
                    <input type="text" id="ammount" name="ammount" placeholder="Enter ammount" required><br>
                    <input type="hidden" name="button_id" value="btn">
                    <button type="submit">Download</button>
                </form>
            </body>
            </html>)";

    logger::setLogLevel(LogLevel::CRITICAL);

    CROW_ROUTE(app, "/")
    ([&](const request& req) {
        return html;
    });

    CROW_ROUTE(app, "/").methods("POST"_method)
        ([&](const request& req)
        {
            try
            {
                map<string, string> form_data = parseFormData(req.body);

                string path = form_data.count("path") ? form_data["path"] : "";
                int ammount = form_data.count("ammount") ? stoi(form_data["ammount"]) : 0;
                string button_id = form_data.count("button_id") ? form_data["button_id"] : "";

                if (button_id == "btn")
                {
                    cout << "Button clicked: " << button_id << endl;
                    cout << "Path entered: " << path << endl;
                    foo(path, ammount);
                    return response{html};
                }

                return response(400, "Invalid button ID");
            }
            catch (const std::exception& e)
            {
                cerr << "Error: " << e.what() << endl;
                return response(500, "Internal Server Error");
            }
        });

    cout << "The website's URL: " << url << endl;
    app.port(port).multithreaded().run();
}

