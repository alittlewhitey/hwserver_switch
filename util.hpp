/*
 * @Author: LittleWhite
 * @Date: 2026-04-13 14:12:33
 * @Copyright: Copyright (c) 2026 by LittleWhite, All Rights Reserved.
 */
#ifndef HWSERVER_UTIL_HPP
#define HWSERVER_UTIL_HPP
#include <iostream>
#include <string>
#include <fstream>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "json.hpp"
struct HuaweiConfig {
    std::string domain_name;
    std::string user_name;  
    std::string password;   
    std::string region;     
    std::string project_id; 
    std::string server_id;  
};

const std::string CRYPTO_KEY = "9391061eeb898e51d0d5cfc268255dc83d3063261dada3e442eb4a245388a015";

void save_config_encrypted(const HuaweiConfig& config, const std::string& path) {
    nlohmann::json j;
    j["domain_name"] = config.domain_name;
    j["user_name"] = config.user_name;
    j["password"] = config.password;
    j["region"] = config.region;
    j["project_id"] = config.project_id;
    j["server_id"] = config.server_id;

    std::string data = j.dump();
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= CRYPTO_KEY[i % CRYPTO_KEY.size()];
    }

    std::ofstream file(path, std::ios::binary);
    if (file.is_open()) {
        file.write(data.data(), data.size());
    } else {
        std::cerr << "Failed to open config file for writing: " << path << std::endl;
    }
}

HuaweiConfig load_config_decrypted(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << path << std::endl;
        return HuaweiConfig();
    }

    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= CRYPTO_KEY[i % CRYPTO_KEY.size()];
    }

    HuaweiConfig config;
    try {
        nlohmann::json j = nlohmann::json::parse(data);
        config.domain_name = j.value("domain_name", "");
        config.user_name = j.value("user_name", "");
        config.password = j.value("password", "");
        config.region = j.value("region", "");
        config.project_id = j.value("project_id", "");
        config.server_id = j.value("server_id", "");
    } catch(std::exception& e) {
        std::cerr << "Failed to parse decrypted config: " << e.what() << std::endl;
    }
    return config;
}
std::string get_token(const HuaweiConfig& config) {
    httplib::SSLClient cli("iam.myhuaweicloud.com");
    
    nlohmann::json req_body = {
        {"auth", {
            {"identity", {
                {"methods", {"password"}},
                {"password", {
                    {"user", {
                        {"domain", {{"name", config.domain_name}}},
                        {"name", config.user_name},
                        {"password", config.password}
                    }}
                }}
            }},
            {"scope", {
                {"project", {{"name", config.region}}}
            }}
        }}
    };
    std::string body = req_body.dump();

    auto res = cli.Post("/v3/auth/tokens", body, "application/json");
    
    if (res) {
        if (res->status == 201) {
            return res->get_header_value("X-Subject-Token");
        } else {
            std::cerr << "get_token request failed. Status: " << res->status << "\nBody: " << res->body << std::endl;
        }
    } else {
        std::cerr << "get_token request encountered an error (connection failed)." << std::endl;
    }
    return "";
}
bool is_token_valid(const std::string& token) {
    if (token.empty()) return false;
    httplib::SSLClient cli("iam.myhuaweicloud.com");
    httplib::Headers headers = {
        { "X-Auth-Token", token },
        { "X-Subject-Token", token }
    };
    auto res = cli.Get("/v3/auth/tokens", headers);
    return (res && res->status == 200);
}

void save_token_encrypted(const std::string& token, const std::string& path) {
    std::string data = token;
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= CRYPTO_KEY[i % CRYPTO_KEY.size()];
    }
    std::ofstream file(path, std::ios::binary);
    if (file.is_open()) {
        file.write(data.data(), data.size());
    } else {
        std::cerr << "Failed to open token file for writing: " << path << std::endl;
    }
}

std::string load_token_decrypted(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= CRYPTO_KEY[i % CRYPTO_KEY.size()];
    }
    return data;
}

std::string get_valid_token_auto(const HuaweiConfig& config, const std::string& token_file = ".token") {
    std::string token = load_token_decrypted(token_file);
    if (is_token_valid(token)) {
        std::cout << "Using cached valid token." << std::endl;
        return token;
    }
    std::cout << "Cached token is invalid or missing. Requesting new token..." << std::endl;
    token = get_token(config);
    if (!token.empty()) {
        save_token_encrypted(token, token_file);
    }
    return token;
}

bool execute_ecs_action(const HuaweiConfig& config, const std::string& token, bool is_start) {
    std::string host = "ecs." + config.region + ".myhuaweicloud.com";
    httplib::SSLClient cli(host.c_str());

    httplib::Headers headers = {
        { "Content-Type", "application/json" },
        { "X-Auth-Token", token }
    };
    std::string body = is_start ? 
                       R"({"os-start": {}})" : 
                       R"({"os-stop": {"type": "SOFT"}})";
    std::string path = "/v2.1/" + config.project_id + "/servers/" + config.server_id + "/action";
    auto res = cli.Post(path.c_str(), headers, body, "application/json");
    if (res) {
        if (res->status == 202) {
            return true;
        } else if (res->status == 409) {
            std::cout << "Action conflict: The server might already be in the target state (e.g., already started or stopped).\nDetails: " << res->body << std::endl;
            return true;
        } else {
            std::cerr << "execute_ecs_action failed. Status: " << res->status << "\nBody: " << res->body << std::endl;
        }
    } else {
        std::cerr << "execute_ecs_action request encountered an error (connection failed)." << std::endl;
    }
    return false;
}
#endif 