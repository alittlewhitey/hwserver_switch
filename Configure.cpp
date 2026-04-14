/*
 * @Author: LittleWhite
 * @Date: 2026-04-13 16:19:07
 * @Copyright: Copyright (c) 2026 by LittleWhite, All Rights Reserved.
 */
#include "util.hpp"
#include <iostream>
#include <string>

int main(){
    HuaweiConfig config;
    
    std::cout << "--- Huawei Cloud Configuration ---" << std::endl;
    
    std::cout << "Enter Domain Name: ";
    std::getline(std::cin, config.domain_name);
    
    std::cout << "Enter User Name: ";
    std::getline(std::cin, config.user_name);
    
    std::cout << "Enter Password: ";
    std::getline(std::cin, config.password);
    
    std::cout << "Enter Region: ";
    std::getline(std::cin, config.region);
    
    std::cout << "Enter Project ID: ";
    std::getline(std::cin, config.project_id);
    
    std::cout << "Enter Server ID: ";
    std::getline(std::cin, config.server_id);
    
    std::string config_file = ".config";
    save_config_encrypted(config, config_file);
    
    std::cout << "Configuration saved securely to " << config_file << "!" << std::endl;
    
    return 0;
}