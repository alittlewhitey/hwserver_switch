/*
 * @Author: LittleWhite
 * @Date: 2026-04-13 13:58:03
 * @Copyright: Copyright (c) 2026 by LittleWhite, All Rights Reserved.
 */

#include "util.hpp"
#include <iostream>

int main(){
    std::cout << "Loading configuration..." << std::endl;
    auto config = load_config_decrypted(".config");
    
    if (config.domain_name.empty()) {
        std::cerr << "Failed to load configuration or invalid domain name." << std::endl;
        return 1;
    }
    
    std::cout << "Getting valid authentication token..." << std::endl;
    auto token = get_valid_token_auto(config);
    if (token.empty()) {
        std::cerr << "Cannot proceed without a valid token." << std::endl;
        return 1;
    }
    
    std::cout << "Authentication successful. Executing ECS Start action..." << std::endl;
    bool success = execute_ecs_action(config, token, true);
    if (success) {
        std::cout << "Successfully sent ECS Start action!" << std::endl;
    } else {
        std::cerr << "Failed to send ECS Start action." << std::endl;
        return 1;
    }
    
    return 0;
}